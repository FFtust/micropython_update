  /**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     
 * @file      codey_neurons_ftp.c
 * @author    Leo lu
 * @version   V1.0.0
 * @date      2017/06/28
 *
 * \par Copyright
 * This software is Copyright (C), 2012-2016, MakeBlock. Use is subject to license \n
 * conditions. The main licensing options available are GPL V2 or Commercial: \n
 *
 * \par Open Source Licensing GPL V2
 * This is the appropriate option if you want to share the source code of your \n
 * application with everyone you distribute it to, and you also want to give them \n
 * the right to share who uses it. If you wish to use this software under Open \n
 * Source Licensing, you must contribute all your source code to the open source \n
 * community in accordance with the GPL Version 2 when your application is \n
 * distributed. See http://www.gnu.org/copyleft/gpl.html
 *
 * \par Description
 * This file setup ble to device and start ble device with a makeblock profile.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * Leo lu             2017/06/28      1.0.0              Initial version
 * </pre>
 *
 */
  
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "codey_utils.h"
#include "lib/oofatfs/ff.h"
#include "extmod/vfs_fat.h"
#include "codey_neurons_ftp.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "codey_sys.h"
#include "codey_esp32_resouce_manager.h"
#include "codey_neurons_deal.h"
#include "codey_neurons_ftp.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#undef    TAG
#define   TAG                     ("CODEY_NEURONS_FTP")

#define FLASH_WRITE_TRY           (3)
#define FLASH_FILE_WRITE_BUFFER   (4096)
#define FRAME_RSP_LEN             (9)

#define HEADER                    (0xF3)
#define END                       (0xF4)
#define SRV_ID                    (0x5E)
#define CMD_ID_HEAD               (0x01)
#define CMD_ID_BLOCK              (0x02)
#define CMD_ID_DEL                (0x03)
#define CMD_ID_STATUS             (0xf0)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/ 
enum
{
  HEAD_S = 0,
  DEV_ID_S,
  SRV_ID_S,
  CMD_ID_S,
  CMD_LEN_1_S,
  CMD_LEN_2_S,
  DATA_S,
  CHECK_S,
  END_S,
};

enum
{
  CODEY_STATUS_OK = 0x00,
  CODEY_STATUS_ERR = 0x01,
  CODEY_STATUS_FILE_CHECKSUM_ERR = 0xf0,
};

typedef struct
{
  uint8_t     file_type;
  uint32_t    file_size;
  uint32_t    file_check_sum;
  uint8_t     *file_name;
  uint8_t     file_name_len;
  uint32_t    file_calc_check_sum;
  FIL         fp;
  bool        is_file_open;
  uint8_t     *flash_write_buf;
  uint32_t    flash_write_buf_cnt;
  uint32_t    file_write_idx;
}codey_neurons_ftp_file_info_t;

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
extern fs_user_mount_t *codey_sflash_vfs_fat;
static SemaphoreHandle_t s_recv_file_name_sem = NULL;
static bool s_module_init = false;
static bool s_get_rx_file_name = false;
static uint8_t s_rx_file_name[MAX_NEU_FTP_FILE_NAME_LEN];

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
static uint8_t  codey_neurons_ftp_parse(uint8_t cmd_id, uint8_t *cmd_data, uint16_t cmd_len);
static bool     is_valid_cmd(uint8_t cmd_id);
static bool     check_file_header_len(uint16_t len);
static bool     check_file_block_len(uint16_t len);
static uint8_t  *get_path_from_full_file_name(uint8_t *full_file_name);
static uint8_t  *get_name_from_full_file_name(uint8_t *full_file_name);
static void     init_file_head_info(codey_neurons_ftp_file_info_t *file_info);
static bool     read_file_head_info(codey_neurons_ftp_file_info_t *file_info, uint8_t *cmd_data, uint16_t cmd_len);
static void     print_file_header(codey_neurons_ftp_file_info_t *file_info);
static uint32_t calc_check_sum(uint8_t *buf, uint32_t len);
static void     build_rsp(uint8_t dev_id, uint8_t *data, uint16_t len, uint8_t *inoutput_buf, uint32_t *output_len);
static void     build_rsp_data(uint8_t dev_id, uint8_t *data, uint16_t len, uint8_t *inoutput_buf, uint32_t *output_len);
static bool     open_file(codey_neurons_ftp_file_info_t *file_info);
static bool     prepare_flash_write_buffer(codey_neurons_ftp_file_info_t *file_info);
static void     free_file_info(codey_neurons_ftp_file_info_t *file_info);
static bool     write_buffer_to_flash(codey_neurons_ftp_file_info_t *file_info, uint32_t total_request_write_wb);
static void     codey_neurons_record_recv_file_name(const char *recv_file_name);
static void     codey_neurons_ready_for_read_recv_file_name(void);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_neurons_ftp_init(void)
{
  if(!s_module_init)
  {
    s_recv_file_name_sem = xSemaphoreCreateMutex();
    s_module_init = true;
  }
}

uint8_t codey_neurons_ftp_pump_char(uint8_t c, uint8_t **out_buf, uint32_t *out_len)
{
  uint8_t ret;
  uint8_t ret_value;

  static uint8_t  state = HEAD_S;
  static uint8_t  dev_id = 0;
  static uint8_t  cmd_id = 0;
  static uint16_t cmd_len = 0;
  static uint8_t  recv_cnt = 0;
  static uint8_t  check_sum = 0;
  static uint8_t  *recv_data;
  static uint8_t  rsp_buf[FRAME_RSP_LEN];

  codey_neurons_ftp_init();

  *out_buf = NULL;
  *out_len = 0;
  switch(state)
  {
    case HEAD_S:
      if(HEADER == c)
      {
        state = DEV_ID_S;
        ret = 1;
      }
      else
      {
        ret = 0;
      }
    break;

    case DEV_ID_S:
      check_sum = c;
      dev_id = c;
      state = SRV_ID_S;
      ret = 1;
    break;

    case SRV_ID_S:
      if(SRV_ID == c)
      {
        check_sum += c;
        state = CMD_ID_S;
        ret = 1;
      }
      else
      {
        state = HEAD_S;
        ret = 0;
      }
    break;

    case CMD_ID_S:
      check_sum += c;
      if(is_valid_cmd(c))
      {
        cmd_id = c;
        state = CMD_LEN_1_S;
        ret = 1;
      }
      else
      {
        state = HEAD_S;
        ret = 0;
      }
    break;

    case CMD_LEN_1_S:
      check_sum += c;
      cmd_len = c;
      state = CMD_LEN_2_S;
      ret = 1;
    break;

    case CMD_LEN_2_S:
      check_sum += c;
      cmd_len += c*0xFF;
      recv_cnt = 0;
      recv_data = (uint8_t *)malloc(cmd_len);
      if(recv_data)
      {
        state = DATA_S;
        ret = 1;
      }
      else
      {
        state = HEAD_S;
        ret = 0;
      }
    break;

    case DATA_S:
      check_sum += c;
      recv_data[recv_cnt++] = c;
      if(recv_cnt == cmd_len)
      {
        state = CHECK_S;
      }
      ret = 1;
    break;

    case CHECK_S:
      if(check_sum == c)
      {
        ret = 1;
        state = END_S;
      }
      else
      {
        free(recv_data);
        state = HEAD_S;
        ret = 0;
      }
    break;

    case END_S:
      if(END == c)
      {
        ret_value = codey_neurons_ftp_parse(cmd_id, recv_data, recv_cnt);
        build_rsp(dev_id, &ret_value, 1, rsp_buf, out_len);
        *out_buf = rsp_buf;
        *out_len = FRAME_RSP_LEN;
      }
      else
      {
        ret = 0;
      }
      free(recv_data);
      state = HEAD_S;
    break;

    default:
      state = HEAD_S;
      ret = 0;    
    break;
  }

  if(CHECK_S == state)
  {
    // ESP_LOGI(TAG, "expected check_sum: %02x\r\n", check_sum);
  }

  return ret;
}


void codey_neurons_ftp(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint8_t ret;
  uint8_t cmd_id;
  uint8_t dev_id;
  
  if(!data || !len)
  {
    return;
  }

  codey_neurons_ftp_init();

  // dev_id + srv_id + cmd_id + len_1 + len_2 + data
  dev_id = data[0];
  cmd_id = data[2];

  ret = codey_neurons_ftp_parse(cmd_id, data + 5, len - 5);
  build_rsp_data(dev_id, &ret, sizeof(ret), output_buf, output_len);
}

void codey_neurons_get_recv_file_name(char *output_buf)
{
  if(!output_buf)
  {
    return;
  }

  codey_neurons_ftp_init();

  if(s_get_rx_file_name)
  {
    output_buf[0] = 0;
    return ;
  }

  xSemaphoreTake(s_recv_file_name_sem, portMAX_DELAY);

  strcpy(output_buf, (char *)s_rx_file_name);
  s_get_rx_file_name = true;
  printf("************abc:\n %s", s_rx_file_name);
  xSemaphoreGive(s_recv_file_name_sem);
  
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
static uint8_t codey_neurons_ftp_parse(uint8_t cmd_id, uint8_t *cmd_data, uint16_t cmd_len)
{
  uint8_t ret;
  bool file_finish;

  uint32_t block_len;
  static codey_neurons_ftp_file_info_t file_info;

  file_finish = false;
  switch(cmd_id)
  {
    case CMD_ID_HEAD:
      {
        free_file_info(&file_info);
        init_file_head_info(&file_info);
        if(read_file_head_info(&file_info, cmd_data, cmd_len))
        {
          // print_file_header(&file_info);
          if(prepare_flash_write_buffer(&file_info))
          {
            if(open_file(&file_info))
            {
              codey_neurons_record_recv_file_name((char *)"NEU_FTY_READY");
              codey_neurons_ready_for_read_recv_file_name();
              ret = CODEY_STATUS_OK;
            }
            else
            {
              ret = CODEY_STATUS_ERR;
            }
          }
          else
          {
            ret = CODEY_STATUS_ERR;
          }
        }
        else
        {
          ret = CODEY_STATUS_ERR;
        }
      }
    break;

    case CMD_ID_BLOCK:
      {
        if(file_info.is_file_open && check_file_block_len(cmd_len))
        {
          block_len = cmd_len - 4;
          if(file_info.flash_write_buf_cnt + block_len <= FLASH_FILE_WRITE_BUFFER)
          {
            // Just move file block data to flash_write_buffer
            // ESP_LOGI(TAG, "Just move to write buffer\r\n");
            memcpy(file_info.flash_write_buf + file_info.flash_write_buf_cnt, cmd_data + 4, block_len);
            file_info.flash_write_buf_cnt += block_len;
            ret = CODEY_STATUS_OK;
          }
          else
          {
            // First flush the flash_write_buffer to flash, and then move block data to flash_write_buffer
            // ESP_LOGI(TAG, " First flush the flash_write_buffer to flash, and then move block data to flash_write_buffer\r\n");
            if(write_buffer_to_flash(&file_info, file_info.flash_write_buf_cnt & 0xFFFFFFFC))
            {
              // TODO, nead to deal with the block_len > flash file write buffer
              if(file_info.flash_write_buf_cnt + block_len <= FLASH_FILE_WRITE_BUFFER)
              {

                memcpy(file_info.flash_write_buf + file_info.flash_write_buf_cnt, cmd_data + 4, block_len);
                file_info.flash_write_buf_cnt += block_len;
                ret = CODEY_STATUS_OK;
              }
              else
              {
                // ESP_LOGI(TAG, "flash write buffer can NOT hole one Block data, need to write to flash directly");
                ret = CODEY_STATUS_ERR;
              }

            }
            else
            {
              ret = CODEY_STATUS_ERR;
            }
          }
          // ESP_LOGI(TAG, "Write buffer cnt %d\r\n", file_info.flash_write_buf_cnt);

          // EOF
          if(CODEY_STATUS_OK == ret && (file_info.file_write_idx + file_info.flash_write_buf_cnt == file_info.file_size))
          {
            file_finish = true;
            // ESP_LOGI(TAG, "Recv all the data\r\n");
            if(file_info.flash_write_buf_cnt)
            {
              if(write_buffer_to_flash(&file_info, file_info.flash_write_buf_cnt))
              {
                if(file_info.file_check_sum == file_info.file_calc_check_sum)
                {
                  // ESP_LOGI(TAG, "EOF\r\n");
                  ret = CODEY_STATUS_OK;
                }
                else
                {
                  // ESP_LOGI(TAG, "File check sum error\r\n");
                  // DELETE THIS FILE
                  ret = CODEY_STATUS_ERR;
                }
                // ESP_LOGI(TAG, "Expected: 0x%08x, get: 0x%08x\r\n", file_info.file_check_sum, file_info.file_calc_check_sum);
              }
            }
          }
        }
        else
        {
          ret = CODEY_STATUS_ERR;
        }
      }
    break;

    case CMD_ID_DEL:
      ret = CODEY_STATUS_ERR;
    break;


    case CMD_ID_STATUS:
      ret = CODEY_STATUS_ERR;
    break;

    default:
      ret = CODEY_STATUS_ERR;
    break;
  }

  if(ret == CODEY_STATUS_ERR || file_finish == true)
  {
    if(file_finish)
    {
      codey_neurons_record_recv_file_name((char *)file_info.file_name);
    }
    free_file_info(&file_info);
    init_file_head_info(&file_info);
    codey_neurons_ready_for_read_recv_file_name();
  }

  return ret;
}

static bool is_valid_cmd(uint8_t cmd_id)
{
  if(CMD_ID_HEAD <= cmd_id && cmd_id <= CMD_ID_STATUS)
  {
    return true;
  }
  else
  {
    return false;
  }
}

static bool check_file_header_len(uint16_t len)
{
  if(len < 10)
  {
    return false;
  }
  else
  {
    return true;
  }
}

static bool check_file_block_len(uint16_t len)
{
  if(len >= 4)
  {
    return true;
  }
  else
  {
    return false;
  }
}

static uint8_t *get_path_from_full_file_name(uint8_t *full_file_name)
{
  size_t check_idx;
  uint8_t *path;

  /*
  A full file name:  "/flash/music/cat.wav"
  It's path is: "/flash/music"
  */

  path = NULL;
  check_idx = strlen((char *)full_file_name) - 1;
  while(check_idx >= 0)
  {
    if(full_file_name[check_idx] == '/')
    {
      // string len = idx + 1; so nead buffer is string len + ternimal = idx + 1 + 1 
      path = (uint8_t *)malloc(check_idx + 1 + 1);
      if(path)
      {
        memcpy(path, full_file_name, check_idx);
        path[check_idx] = 0;
      }
      break;
    }
    else
    {
      check_idx--;
    }
  }

  return path;
}

static uint8_t *get_name_from_full_file_name(uint8_t *full_file_name)
{
  size_t check_idx;
  size_t len;
  uint8_t *name;

  /*
  A full file name:  "/flash/music/cat.wav"
  It's name is: "cat.wav"
  */

  name = NULL;
  len = strlen((char *)full_file_name);
  check_idx = len - 1;
  while(check_idx >= 0)
  {
    if(full_file_name[check_idx] == '/')
    {
      // string len = len - idx - 1;  so nead buffer is string len + ternimal =  len - idx - 1 + 1
      name = (uint8_t *)malloc(len - check_idx);
      if(name)
      {
        memcpy(name, full_file_name + check_idx + 1, len - check_idx - 1);
        name[len - check_idx - 1] = 0;
      }
      break;
    }
    else
    {
      check_idx--;
    }
  }

  return name;
}

static void init_file_head_info(codey_neurons_ftp_file_info_t *file_info)
{
  memset(file_info, 0, sizeof(codey_neurons_ftp_file_info_t));
}

static bool read_file_head_info(codey_neurons_ftp_file_info_t *file_info, uint8_t *cmd_data, uint16_t cmd_len)
{
  bool ret;

  if(check_file_header_len(cmd_len))
  {
    /*
    File header format:
    1 bytes       4 bytes       4bytes            varible bytes
    file_type     file_size     file_check_sum    full_file_name
    */
    file_info->file_type = cmd_data[0];
    memcpy(&(file_info->file_size), cmd_data + 1, 4);
    memcpy(&(file_info->file_check_sum), cmd_data + 5, 4);
    file_info->file_name_len = cmd_len - 9;
    file_info->file_name = malloc(file_info->file_name_len + 1);
    if(file_info->file_name)
    {
      memcpy(file_info->file_name, cmd_data + 9, file_info->file_name_len);
      file_info->file_name[ file_info->file_name_len ] = 0;
      file_info->file_write_idx = 0;

      ret = true;
    }
    else
    {
      ret = false;
    }
  }
  else
  {
    ret = false;
  }
  return ret;
}

static void print_file_header(codey_neurons_ftp_file_info_t *file_info)
{
  // ESP_LOGI(TAG,  "file type %d\n",               file_info->file_type);
  // ESP_LOGI(TAG, "file size %d\n",               file_info->file_size);
  // ESP_LOGI(TAG, "file name %s\n",               file_info->file_name);
  // ESP_LOGI(TAG, "file write idx %d\n",          file_info->file_write_idx);
  // ESP_LOGI(TAG, "file check sum 0x%08x\n",      file_info->file_check_sum);
  // ESP_LOGI(TAG, "file cal check sum 0x%08x\n",  file_info->file_calc_check_sum);
}

static uint32_t calc_check_sum(uint8_t *buf, uint32_t len)
{
  uint32_t oxr_check;
  uint32_t tmp;
  uint32_t idx;
  
  idx = 0;
  oxr_check = 0;
  while(idx < len)
  {
    if(idx + 4 <= len)
    {
      memcpy(&tmp, buf + idx, 4);
      oxr_check ^= tmp;
      idx += 4;
    }
    else
    {
      tmp = 0;
      memcpy(&tmp, buf + idx, len - idx);
      oxr_check ^= tmp;
      break;
    }
  }

	return oxr_check;

}

static void build_rsp(uint8_t dev_id, uint8_t *data, uint16_t len, uint8_t *inoutput_buf, uint32_t *output_len)
{
  uint8_t i;
  uint32_t idx;
  uint8_t check_sum;

  idx = 0;
  check_sum = 0;
  inoutput_buf[idx++] = HEADER;
  inoutput_buf[idx++] = dev_id;
  inoutput_buf[idx++] = SRV_ID;
  inoutput_buf[idx++] = CMD_ID_STATUS;
  inoutput_buf[idx++] = len & 0x0ff;
  inoutput_buf[idx++] = (len>>8) & 0x0ff;
  memcpy(inoutput_buf + idx, data, len); 
  for (i = 1; i < idx + len; i++)
  {
    check_sum += inoutput_buf[i];
  }
  idx = i;
  inoutput_buf[idx++] = check_sum;
  inoutput_buf[idx++] = END;
  *output_len = idx;
}

// NO frame header, NO checksum, and NO frame end
static void build_rsp_data(uint8_t dev_id, uint8_t *data, uint16_t len, uint8_t *inoutput_buf, uint32_t *output_len)
{
  uint32_t idx = 0;

  inoutput_buf[idx++] = dev_id;
  inoutput_buf[idx++] = SRV_ID;
  inoutput_buf[idx++] = CMD_ID_STATUS;
  inoutput_buf[idx++] = len & 0x0ff;
  inoutput_buf[idx++] = (len>>8) & 0x0ff;
  memcpy(inoutput_buf + idx, data, len);
  idx += len;
  *output_len = idx;

  // ESP_LOGI(TAG, "NEU_FTP_RSP: ");
  // codey_print_hex(inoutput_buf, *output_len);
  // ESP_LOGI(TAG, "\r\n");
}

static bool open_file(codey_neurons_ftp_file_info_t *file_info)
{
  bool ret;
  uint8_t *path;
  uint8_t *name;

  if(xSemaphoreTake(g_fatfs_sema, 1000/portTICK_PERIOD_MS) == pdTRUE)
  {
    path = get_path_from_full_file_name(file_info->file_name);
    name = get_name_from_full_file_name(file_info->file_name);
    if(path && name)
    {

      // ESP_LOGI(TAG, "path: %s, name: %s\r\n", (char *)path, (char *)name);
      if(FR_OK == f_chdir(&codey_sflash_vfs_fat->fatfs, (char *)path)) 
      {
        if(FR_OK == f_open(&codey_sflash_vfs_fat->fatfs, &(file_info->fp), (const char *)name, FA_WRITE|FA_CREATE_ALWAYS))
        {
          // ESP_LOGI(TAG, "Open file %s\r\n", file_info->file_name);
          file_info->is_file_open = true;
          ret = true;
        }
        else
        {
          // ESP_LOGI(TAG, "Cat not open file %s\r\n", (char *)name);
          ret = false;
        }

      }
      else
      {
        // ESP_LOGI(TAG, "Cat not change to dir %s\r\n", (char *)path);
        ret = false;

      }
      
    }
    else
    {
      ret = false;

    }
    xSemaphoreGive(g_fatfs_sema);
  }
  else
  {
    return false;
  }

  if(path)
  {
    free(path);
  }
  
  if(name)
  {
    free(name);
  }

  return ret;
}

static bool prepare_flash_write_buffer(codey_neurons_ftp_file_info_t *file_info)
{
  file_info->flash_write_buf = (uint8_t *)malloc(FLASH_FILE_WRITE_BUFFER);

  if(file_info->flash_write_buf)
  {
    return true;
  }
  else
  {
    return false;
  }
}

static void free_file_info(codey_neurons_ftp_file_info_t *file_info)
{
  if(file_info->flash_write_buf)
  {
    free(file_info->flash_write_buf);
  }

  if(file_info->file_name)
  {
    free(file_info->file_name);
  }

  if(file_info->is_file_open)
  {
    f_close(&(file_info->fp));
    file_info->is_file_open = false;
  }
}

static bool write_buffer_to_flash(codey_neurons_ftp_file_info_t *file_info, uint32_t total_request_write_wb)
{
  bool ret;
  uint32_t have_been_wb;
  uint32_t wb;
  uint8_t write_try_cnt;

  if(file_info->is_file_open)
  { 
    if(xSemaphoreTake(g_fatfs_sema, 1000/portTICK_PERIOD_MS) == pdTRUE)
    {

      // ESP_LOGI(TAG, "Write index %d, total write wb %d\r\n", file_info->file_write_idx, total_request_write_wb);
      have_been_wb = 0;
      write_try_cnt  = 0;
      f_lseek(&(file_info->fp), file_info->file_write_idx);
      while((have_been_wb < total_request_write_wb) && (write_try_cnt < FLASH_WRITE_TRY))

      {
        f_write(&(file_info->fp), file_info->flash_write_buf + have_been_wb, total_request_write_wb - have_been_wb, &wb);
        have_been_wb += wb;
        file_info->file_write_idx += wb;
        // ESP_LOGI(TAG, "Write %d bytes to flash OK, write index %d\r\n", wb, file_info->file_write_idx);

        if(!wb)
        {
          // ESP_LOGI(TAG, "Can NOT write to flash, try %d time\r\n", write_try_cnt+1);
          write_try_cnt++;
        }
      }

      if(have_been_wb == total_request_write_wb)
      {
        // update the file check sum
        file_info->file_calc_check_sum ^= calc_check_sum(file_info->flash_write_buf, have_been_wb);
        
        // move the left bytes to the beginning of the write buffer
        if(file_info->flash_write_buf_cnt - have_been_wb)
        {
          memcpy(file_info->flash_write_buf, file_info->flash_write_buf + have_been_wb, file_info->flash_write_buf_cnt - have_been_wb);
          // ESP_LOGI(TAG, "Move the left %d bytes to the beginning of the write buffer", file_info->flash_write_buf_cnt - have_been_wb);
        }
        file_info->flash_write_buf_cnt -= have_been_wb;
        ret = true;
      }
      else
      {
        // ESP_LOGI(TAG, "Need to write %d bytes, but just write %d\r\n", total_request_write_wb, have_been_wb);
        ret = false;
      }
      
      xSemaphoreGive(g_fatfs_sema);    
    }
    else
    {
      ret = false;
    }
  }
  else
  {
    // ESP_LOGI(TAG, "File is NOT open\r\n");
    ret = false;
  }
  
  return ret;
}

static void codey_neurons_record_recv_file_name(const char *recv_file_name)
{
  xSemaphoreTake(s_recv_file_name_sem, portMAX_DELAY);
  printf("************abcff:\n %s", recv_file_name);
  if(strlen(recv_file_name) + 1 < MAX_NEU_FTP_FILE_NAME_LEN)
  {
    strcpy((char *)s_rx_file_name, recv_file_name);
  }
  else
  {
    s_rx_file_name[0] = 0;
  }
  
  xSemaphoreGive(s_recv_file_name_sem);

  codey_give_data_recv_sem();
}

static void codey_neurons_ready_for_read_recv_file_name(void)
{
  xSemaphoreTake(s_recv_file_name_sem, portMAX_DELAY);
  
  s_get_rx_file_name = false;
  
  xSemaphoreGive(s_recv_file_name_sem);
}

// END OF FILE
