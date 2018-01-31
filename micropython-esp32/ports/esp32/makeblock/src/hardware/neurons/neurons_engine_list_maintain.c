/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock neurons engine module
 * @file    neurons_engine_list_maintain.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/09/06
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
 * This file is a drive for neurons engine module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/09/06      1.0.0              build the new.
 * Leo lu             2017/10/30      1.0.1              neuron command FIFO lock to just one 
 *                                                       Push command must wait for FIFO
 * </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

#include "codey_sys.h"
#include "codey_esp32_resouce_manager.h"
#include "codey_config.h"

#include "neurons_engine_lib_struct.h"
#include "neurons_engine_list_maintain.h"
#include "neurons_engine_port.h"
#include "codey_neurons_universal_protocol.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG                           ("NEU_ENG_MAINTAIN")
#define NEURONS_HEART_INTERVAL        (500) // ms
#define NEURONS_ROCKY_REPORT_INTERVAL (1) // 10ms
#define NEURONS_READ_WAIT_MAX         (100) // ms

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
static uint8_t s_rocky_sensor_report_status[ROCKY_SENSOR_REPORT_MAX + 1] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static bool s_neurons_lib_found_flag = false;
static EventGroupHandle_t s_neurons_read_event_group = NULL;

/* this array store the on_line block infomation for the last heart package */
/* update the main list depend on this array */
struct neurons_engine_online_inf_t
{
  uint8_t now_group_index; /* 0 or 1*/
  uint8_t blocks_num[2];
  neurons_engine_online_block_info_t blocks_info[2][NEURONS_NODE_MAX];
}neurons_engine_online_info;

static neurons_engine_flash_lib_file_head_t s_neurons_engine_info;

typedef struct
{
  void *next;
  neurons_engine_online_block_lib_t *lib_addr;
}neurons_engine_online_block_lib_list_t;

neurons_engine_online_block_lib_list_t *s_neurons_engine_online_block_lib_list_head = NULL;

struct neurons_command_buffer_t
{
  uint8_t cmd_cash_in_index;
  uint8_t cmd_cash_out_index;
  neurons_comnad_type_t neurons_comnad_type[CMD_BUFFER_MAX];
}neurons_command_buffer;

/******************************************************************************
 DECLARE PRIVATE FUNCTION
 ******************************************************************************/
STATIC void neurons_engine_read_init_t(void);
STATIC bool neurons_engine_online_block_list_is_empty_t(void);
STATIC void neurons_engine_online_block_list_add_t(neurons_engine_online_block_lib_list_t *new_node);
STATIC void neurons_engine_online_block_list_delete_t(neurons_engine_online_block_lib_list_t *node);
STATIC neurons_engine_online_block_lib_t *neurons_engine_online_block_list_check_t(uint8_t type, uint8_t sub_type);
STATIC void neurons_engine_online_block_list_clear_all_block_num(void);
STATIC void neurons_engine_online_block_list_not_used_free(void);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void neurons_engine_read_set_bits_t(uint32_t bits)
{
  xEventGroupSetBits(s_neurons_read_event_group, bits);
}

void neurons_engine_read_clear_bits_t(uint32_t bits)
{
  xEventGroupClearBits(s_neurons_read_event_group, bits);
}

void neurons_engine_wait_bits_t(uint32_t bits)
{
  xEventGroupWaitBits(s_neurons_read_event_group, bits, pdTRUE, pdFALSE, NEURONS_READ_WAIT_MAX / portTICK_PERIOD_MS);
}
bool neurons_engine_get_lib_found_flag_t(void)
{
  return s_neurons_lib_found_flag;
}

/* make this function a static function to avoid unlawful usage */
STATIC void neurons_engine_set_lib_found_flag_t(bool sta)
{
  s_neurons_lib_found_flag = (bool)sta;
}

uint8_t neurons_engine_get_main_list_length_t(void)
{
  return neurons_engine_online_info.blocks_num[(neurons_engine_online_info.now_group_index + 1) % 2];
}

uint8_t neurons_engine_get_main_list_now_group_t(void)
{
  return (neurons_engine_online_info.now_group_index + 1) % 2;
}

neurons_engine_online_block_info_t *neurons_engine_get_main_list_block_info_by_id_t(uint8_t now_group, uint8_t id)
{
  if(now_group > 1 || id >= NEURONS_NODE_MAX)
  {
    return NULL;
  }
  else
  {
    if(neurons_engine_online_info.blocks_info[now_group][id].block_ram_addr == NULL)
    {
      ESP_LOGE(TAG, "RAM addr is NULL");
      return NULL;
    }
    return &neurons_engine_online_info.blocks_info[now_group][id];
  }
}

neurons_engine_online_block_info_t *neurons_engine_get_main_list_block_info_by_index_t(uint8_t index,
                                                                                                                uint8_t type, uint8_t sub_type)
{
  if(index == 0)
  {
    return NULL;
  }
  
  uint8_t main_list_len = neurons_engine_get_main_list_length_t();
  uint8_t now_group = neurons_engine_get_main_list_now_group_t();
  neurons_engine_online_block_info_t *info = NULL;
  uint8_t now_index = 0;
  for(uint8_t i = 1; i <= main_list_len; i++)
  {
    info = neurons_engine_get_main_list_block_info_by_id_t(now_group, i);
    if(info != NULL)
    { 
      if(info->block_ram_addr == NULL)
      {
        ESP_LOGE(TAG, "get list by index ram addr is NULL");
        return NULL;
      }
      if(info->block_ram_addr->block_type == type && info->block_ram_addr->block_sub_type == sub_type)
      {
        if(++now_index == index)
        {
          return info;
        }
      }
    }
  }
  return NULL;
}


void neurons_engine_get_command_info_t(neurons_engine_online_block_info_t *info, uint8_t cmd_id, 
                                                        uint8_t *data, uint8_t *len)
{
  uint8_t cmd_data_len = 0;
  /* the first byte is the lenth of command length */
  uint8_t cmd_lenth = info->block_ram_addr->command_data_type[0];
  uint16_t offset = 0;
  ESP_LOGD(TAG, "cmd length is %d", cmd_lenth);
  ESP_LOGD(TAG, "cmd id is %d", cmd_id);
  for(uint8_t i = 1; i <= cmd_lenth; i++)
  {
    if(info->block_ram_addr->command_data_type[1 + (i - 1) * MODULE_ONLINE_INFO_SINGLE_COMMAND_LEN] == cmd_id)
    {
      offset = 1 + (i - 1) * MODULE_ONLINE_INFO_SINGLE_COMMAND_LEN;
    }
  }
  if(offset == 0)
  {
    (*len) = 0;
    return;
  }
  ESP_LOGD(TAG, "cmd offset is %d", offset);
  while(info->block_ram_addr->command_data_type[offset + cmd_data_len] != 0x00)
  {
    if(cmd_data_len == COMMAND_ITEM_MAX)
    {
      cmd_data_len++;
      break;
    }
    cmd_data_len++;
  }
  *len = cmd_data_len;
  ESP_LOGD(TAG, "cmd data len is%d\n", *len);
  for(uint8_t i = 0; i < cmd_data_len; i++)
  {
    *(data + i) = info->block_ram_addr->command_data_type[offset + i];
    ESP_LOGD(TAG, "cmd data %d is %d\n", i, (*(data + i)));
  }
}

void neurons_engine_get_respond_info_t(neurons_engine_online_block_info_t *info, uint8_t respond_id, 
                                                        uint8_t *respond_data_offset, uint8_t *data, uint8_t *len)
{
  uint8_t respond_data_len = 0;
  /* the first byte is the lenth of command length */
  uint8_t respond_lenth = info->block_ram_addr->respond_data_type[0];
  uint16_t offset = 0;
  uint8_t special_type_num = 0;
  
  ESP_LOGD(TAG, "respond length is %d", respond_lenth);
  ESP_LOGD(TAG, "respond id is %d", respond_id);
  for(uint8_t i = 1; i <= respond_lenth; i++)
  {
    if(info->block_ram_addr->respond_data_type[1 + (i - 1) * MODULE_ONLINE_INFO_SINGLE_RESPOND_LEN] == respond_id)
    {
      offset = 1 + (i - 1) * MODULE_ONLINE_INFO_SINGLE_RESPOND_LEN;
    }
  }
  if(offset == 0)
  {
    (*len) = 0;
    return;
  }
  ESP_LOGD(TAG, "respond offset is %d", offset);
  
  while(info->block_ram_addr->respond_data_type[offset + respond_data_len] != 0x00)
  {
    if(respond_data_len == RESPOND_ITEM_MAX)
    {
      respond_data_len++;
      break;
    }
    /*the data after 0xbx is not a type */
    if((info->block_ram_addr->respond_data_type[offset + respond_data_len] & 0xf0)== ALTERABLE_NUM_BYTE)
    {
      special_type_num++;
    }
    respond_data_len++;
  }
  *len = ((respond_data_len  - special_type_num) > 5 ? 5 : (respond_data_len - special_type_num));
  ESP_LOGD(TAG, "respond data type len is%d", *len);
  for(uint8_t i = 0; i < respond_data_len; i++)
  {
    *(data + i) = info->block_ram_addr->respond_data_type[offset + i];
    ESP_LOGD(TAG, "respond data %d is %d", i, *(data + i));
  }
  (*respond_data_offset) = ((neurons_engine_online_block_lib_respond_t *)(&info->block_ram_addr->respond_data_type[offset]))->respond_struct.value_offset;
}

void neurons_engine_node_lib_information_show_t(neurons_engine_online_block_lib_t *lib)
{
  // ESP_LOGD(TAG, "respond data addr is %p\n", lib->respond_data_type);
  // ESP_LOGD(TAG, "block type is %d\n", lib->block_type);
  // ESP_LOGD(TAG, "block sub type is %d\n", lib->block_sub_type);
  // ESP_LOGD(TAG, "block respond data is %d & %d\n", *(uint8_t *)lib->respond_data, *(uint8_t *)(lib->respond_data + 1));
}

void neurons_engine_get_version_t(char *dataout, uint8_t *len)
{
  *len = strlen((char *)s_neurons_engine_info.head_frame.version_info);
  ESP_LOGD(TAG, "version len is %d", (*len));
  strcpy(dataout, (char *)s_neurons_engine_info.head_frame.version_info);
  ESP_LOGD(TAG, "version string is %s", dataout);
}

bool neurons_engine_init_t(void)
{
  uint8_t wait = 0;
  static bool init_flag = false;
  if(!init_flag)
  {
    init_flag = true;
    while(codey_resource_manager_get_file_system_status_t() != true)
    {
      vTaskDelay(10);
      if(wait++ > 100)
      { 
        return false;
      }
    }
    if(neurons_engine_read_head_from_flash_lib_t(&s_neurons_engine_info))
    {
      neurons_engine_set_lib_found_flag_t(true);
      init_flag = true;
      /* the following function will be deleted later */
      neurons_engine_read_init_t();
      neurons_engine_read_control_init_t();
      return true;
    }
    else
    {
      ESP_LOGE(TAG, "neurons read flash failed");
      neurons_engine_set_lib_found_flag_t(false);
      return false;
    }
    /* use event group for rocky reading */
  }
  else
  {
    ESP_LOGD(TAG, "neurons have inited");
    return true;
  }
}

void neurons_engine_add_new_block_t(neurons_engine_online_block_info_t *inf)
{
  
  memcpy(&neurons_engine_online_info.blocks_info[neurons_engine_online_info.now_group_index][inf->dev_id], inf, 3);
  neurons_engine_online_info.blocks_num[neurons_engine_online_info.now_group_index]++;
  
  ESP_LOGD(TAG, "add_a new_block dev:%d type:%d,sub_type:%d\n", inf->dev_id, inf->block_type, inf->block_sub_type);
  ESP_LOGD(TAG, "now blocks index is:%d\n", neurons_engine_online_info.blocks_num[neurons_engine_online_info.now_group_index]);
}

/* in this function, we may get data from flash */
void neurons_engine_online_blocks_set_new_t(uint8_t index)
{
  uint8_t last_index = (index + 1) % 2;
  bool file_opend = false;
  FIL  fp;
  neurons_block_type_inf_t block = {.block_type = 0, .block_sub_type = 0};
  uint8_t *neurons_block_info;
  ESP_LOGD(TAG, "blocks_num is %d, %d", neurons_engine_online_info.blocks_num[index], neurons_engine_online_info.blocks_num[last_index]);
  neurons_engine_online_block_list_clear_all_block_num();
  // ESP_LOGD(TAG, "block num clear\n");
  neurons_engine_online_block_lib_t *lib_out = NULL;
  for(uint8_t i = 1; i <= neurons_engine_online_info.blocks_num[index]; i++)
  {
    /* see if a same block is already existed */
    lib_out = neurons_engine_online_block_list_check_t(neurons_engine_online_info.blocks_info[index][i].block_type,
    neurons_engine_online_info.blocks_info[index][i].block_sub_type);
    /* a same block existed */
    if(lib_out != NULL)
    {
      // ESP_LOGD(TAG, "find in ram");
      /* make the pointer refer to the ram address */
      neurons_engine_online_info.blocks_info[index][i].block_ram_addr = lib_out;
      /* increase the block num, if block num is zero after this loop, indicate that no this block existed */
      neurons_engine_online_info.blocks_info[index][i].block_ram_addr->block_num++;
      /* this means the block position is not changed */
      if(!memcmp(&neurons_engine_online_info.blocks_info[index][i], &neurons_engine_online_info.blocks_info[last_index][i], 3))
      {
        neurons_engine_online_info.blocks_info[index][i].respond_data_info.respond_data_buffer = 
        neurons_engine_online_info.blocks_info[last_index][i].respond_data_info.respond_data_buffer;
        neurons_engine_online_info.blocks_info[index][i].respond_data_info.respond_data_buffer_size = 
        neurons_engine_online_info.blocks_info[last_index][i].respond_data_info.respond_data_buffer_size;
      }
      /* try to find in the last list */
      else
      {
        uint8_t s_block_id = 1;
        for(; s_block_id <= neurons_engine_online_info.blocks_num[index]; s_block_id++)
        {
          if(!memcmp(&neurons_engine_online_info.blocks_info[index][i], &neurons_engine_online_info.blocks_info[index][s_block_id], 2))
          {
            break;
          }
        }
        /* alloc a static memory to store blcok received data, and free the memory that would not be used */
        neurons_engine_online_info.blocks_info[index][i].respond_data_info.respond_data_buffer_size = 
        neurons_engine_online_info.blocks_info[index][s_block_id].respond_data_info.respond_data_buffer_size;
        neurons_engine_online_info.blocks_info[index][i].respond_data_info.respond_data_buffer = 
        heap_caps_malloc(neurons_engine_online_info.blocks_info[index][s_block_id].respond_data_info.respond_data_buffer_size, MALLOC_CAP_8BIT);
        memset(neurons_engine_online_info.blocks_info[index][i].respond_data_info.respond_data_buffer, 0, neurons_engine_online_info.blocks_info[index][i].respond_data_info.respond_data_buffer_size);
        ESP_LOGD(TAG, "alloc respond data buffer, size is %d, dev_id is %d", neurons_engine_online_info.blocks_info[index][i].respond_data_info.respond_data_buffer_size, i);
        if(neurons_engine_online_info.blocks_info[index][i].respond_data_info.respond_data_buffer == NULL)
        {
          ESP_LOGE(TAG, "alloc respond data buffer failed");
        }
        if(neurons_engine_online_info.blocks_info[last_index][i].respond_data_info.respond_data_buffer != NULL)
        {
          free(neurons_engine_online_info.blocks_info[last_index][i].respond_data_info.respond_data_buffer);
          neurons_engine_online_info.blocks_info[last_index][i].respond_data_info.respond_data_buffer = NULL;
        }
      }
    }
    /* a new block . start to find in flash */
    else
    {
      // ESP_LOGD(TAG, "start find in flash\n");
      if(file_opend == false)
      {
        if(xSemaphoreTake(g_fatfs_sema, 5000 / portTICK_PERIOD_MS) == pdTRUE)
        {
          f_chdir(&codey_sflash_vfs_fat->fatfs, "/lib");
          if(FR_OK == f_stat(&codey_sflash_vfs_fat->fatfs, NEURONS_ENGINE_LIB_FILE_NAME, NULL)) 
          {
            f_chdir(&codey_sflash_vfs_fat->fatfs, "/lib");
            f_open(&codey_sflash_vfs_fat->fatfs, &fp, NEURONS_ENGINE_LIB_FILE_NAME, FA_READ);
            file_opend = true;
          }
        }
      }
            
      neurons_engine_special_block_t special_block = {.offset = 0xffff};
      neurons_engine_special_block_t *special_indicate = NULL;
      uint16_t offset = 0xffff;
      memcpy(&block, &neurons_engine_online_info.blocks_info[index][i].block_type, 2);
      offset = neurons_engine_check_block_from_flash_lib_t(&fp, &block, &s_neurons_engine_info, &special_block);
      if(offset == 0)
      {
        ESP_LOGE(TAG, "not found this block in flash");
        continue;
      }
      /* indicate that is a special block */
      else if(offset == special_block.offset)
      {  
        uint16_t size = (special_block.command_num + special_block.respond_num) * REC_CMD_ITEM_PER_LEN + 2;
        ESP_LOGD(TAG, "special block command bytes size is %d", size);
        neurons_block_info = heap_caps_malloc(size, MALLOC_CAP_8BIT);
        ESP_LOGD(TAG, "a special block offset got");
        if(neurons_block_info == NULL)
        {
          ESP_LOGE(TAG, "heap over");
          break;
        }
        special_indicate = &special_block;
      }
      /* a general block */
      else
      {
        neurons_block_info = heap_caps_malloc(sizeof(neurons_block_type_t), MALLOC_CAP_8BIT);
        if(neurons_block_info == NULL)
        {
          ESP_LOGE(TAG, "heap over");
          break;
        }
        special_indicate = NULL;
      }

      neurons_engine_get_blcok_info_t(&fp, &block, offset, neurons_block_info, &special_block);
      
      neurons_engine_online_block_lib_t *block_lib_info = NULL;
      block_lib_info = (neurons_engine_online_block_lib_t *)heap_caps_malloc(sizeof(neurons_engine_online_block_lib_t), MALLOC_CAP_8BIT);
      if(block_lib_info == NULL)
      {
        ESP_LOGE(TAG, "heap over");
        heap_caps_free(neurons_block_info);
        continue;
      }
      block_lib_info->block_num = 1;
      block_lib_info->block_type = block.block_type;
      block_lib_info->block_sub_type = block.block_sub_type;
      
      neurons_engine_get_respond_info_from_lib_t(neurons_block_info, (void **)&block_lib_info->respond_data_type, 
                                                 &neurons_engine_online_info.blocks_info[index][i].respond_data_info, special_indicate);
      block_lib_info->command_data_type = (uint8_t *)neurons_engine_get_command_info_from_lib_t(neurons_block_info, special_indicate);
      neurons_engine_online_info.blocks_info[index][i].block_ram_addr = block_lib_info;

      neurons_engine_online_block_lib_list_t *new_node = heap_caps_malloc(sizeof(neurons_engine_online_block_lib_list_t), MALLOC_CAP_8BIT);
      new_node->lib_addr = neurons_engine_online_info.blocks_info[index][i].block_ram_addr;
      neurons_engine_online_block_list_add_t(new_node);
      heap_caps_free(neurons_block_info);
      ESP_LOGD(TAG, "find in flash type is%d, sub is %d", block.block_type, block.block_sub_type)
    }
  }
  /* delete the blocks that not existed, and free the related memory */
  if(neurons_engine_online_info.blocks_num[index] < neurons_engine_online_info.blocks_num[last_index])
  {
    for(uint8_t i = neurons_engine_online_info.blocks_num[index] + 1; i <= neurons_engine_online_info.blocks_num[last_index]; i++)
    {
      
      if(neurons_engine_online_info.blocks_info[last_index][i].respond_data_info.respond_data_buffer != NULL)
      {
        // ESP_LOGD(TAG, "block respond data free, dev id is%d", i);
        heap_caps_free(neurons_engine_online_info.blocks_info[last_index][i].respond_data_info.respond_data_buffer);
        neurons_engine_online_info.blocks_info[last_index][i].respond_data_info.respond_data_buffer = NULL;
      }
      else
      {
        ESP_LOGE(TAG, "block respond data NULL, dev id is%d", i);
      }
    }
  }
  neurons_engine_online_block_list_not_used_free();
  if(file_opend == true)
  {
    f_close(&fp);
    xSemaphoreGive(g_fatfs_sema);
  }
}

void neurons_engine_show_mian_list_info(void)
{ 
  uint8_t now_group_index = neurons_engine_online_info.now_group_index;
  uint8_t last_group_index = (neurons_engine_online_info.now_group_index + 1) % 2;
  ESP_LOGD(TAG, "now group block num is %d, the detail:", neurons_engine_online_info.blocks_num[now_group_index]);
  for(uint8_t i = 1; i <= neurons_engine_online_info.blocks_num[now_group_index]; i++)
  {
     ESP_LOGD(TAG, "block %d type is %d, sub type is %d", i, neurons_engine_online_info.blocks_info[now_group_index][i].block_type,
                  neurons_engine_online_info.blocks_info[now_group_index][i].block_sub_type);
  }
  ESP_LOGD(TAG, "last group block num is %d, the detail:", neurons_engine_online_info.blocks_num[last_group_index]);
  for(uint8_t i = 1; i <= neurons_engine_online_info.blocks_num[last_group_index]; i++)
  {
    ESP_LOGD(TAG, "block %d type is %d, sub type is %d", i, neurons_engine_online_info.blocks_info[last_group_index][i].block_type,
                  neurons_engine_online_info.blocks_info[last_group_index][i].block_sub_type);
  }
}

/* before sending the heart package, update the main list */ 
void neurons_engine_online_blocks_update_t(void)
{
  static uint8_t off_line_time = 0;
  uint8_t now_group_index = neurons_engine_online_info.now_group_index;
  uint8_t last_index = (now_group_index + 1) % 2;
  
  for(uint8_t i = 1; i <= neurons_engine_online_info.blocks_num[now_group_index]; i++)
  {
    /* compare type and sub_type and device_id*/
    if(memcmp(&neurons_engine_online_info.blocks_info[now_group_index][i], &neurons_engine_online_info.blocks_info[last_index][i], 3))
    {
      /* checked package loss */
      if(neurons_engine_online_info.blocks_info[now_group_index][i].dev_id == 0)
      {
        neurons_engine_online_info.blocks_info[now_group_index][i].offline_time++;
      }
      /* checked package losing three times, update new*/
      if(neurons_engine_online_info.blocks_info[now_group_index][i].offline_time >= 3 || neurons_engine_online_info.blocks_info[now_group_index][i].dev_id != 0)
      {
        // ESP_LOGD(TAG, "main list changed or add new block");
        neurons_engine_show_mian_list_info();
        neurons_engine_online_blocks_set_new_t(now_group_index);
        neurons_engine_online_info.now_group_index = last_index;
        off_line_time = 0;
        goto buffer_clear;
      }
    }

  }
  /* some blocks are off_line, we need record the times they are in offline status */
  if(neurons_engine_online_info.blocks_num[last_index] > neurons_engine_online_info.blocks_num[now_group_index])
  {
    // ESP_LOGD(TAG, "find offline block");
    if(off_line_time >= 3)
    {
      // ESP_LOGD(TAG, "off_line three times checked");
      neurons_engine_show_mian_list_info();
      neurons_engine_online_blocks_set_new_t(now_group_index);
      neurons_engine_online_info.now_group_index = last_index;
     
      off_line_time = 0;
      goto buffer_clear;
    }
    else
    {
      // ESP_LOGD(TAG, "off_line times increase, now is%d\n", off_line_time);
      off_line_time++;
      goto buffer_clear;
    }
  }
  else
  {
    /* not changed */
    // ESP_LOGD(TAG, "main list not changed, main list index is %d\n", neurons_engine_online_info.now_group_index);
    off_line_time = 0;
    goto buffer_clear;

  }
buffer_clear:  
  neurons_engine_online_info.blocks_num[neurons_engine_online_info.now_group_index] = 0;
  memset(neurons_engine_online_info.blocks_info[neurons_engine_online_info.now_group_index], 0,
         sizeof(neurons_engine_online_block_info_t) * NEURONS_NODE_MAX);
}

/* neurons engine port */
uint8_t neurons_engine_find_block_in_main_list_by_id_t(uint8_t dev_id, uint8_t type, uint8_t sub_type)
{
  uint8_t now_group = neurons_engine_get_main_list_now_group_t();
  neurons_engine_online_block_info_t *info = NULL;

  info =  neurons_engine_get_main_list_block_info_by_id_t(now_group, dev_id);
  /* invalid block id */
  if(info == NULL)
  {
    return 1;
  }
  /* the information of this id is matched */
  if(type == info->block_type && sub_type == info->block_sub_type)
  {
    return 0;
  }
  /* not matched */
  else
  {
    return 2;
  }
}

uint8_t neurons_engine_find_block_in_main_llist_by_index_t(uint8_t index, uint8_t type, uint8_t sub_type)
{
  /* invalid index */
  if(index == 0)
  {
    return 1;
  }
  
  uint8_t main_list_len = neurons_engine_get_main_list_length_t();
  uint8_t now_group = neurons_engine_get_main_list_now_group_t();
  neurons_engine_online_block_info_t *info = NULL;
  uint8_t now_index = 0;
  for(uint8_t i = 1; i <= main_list_len; i++)
  {
    info = neurons_engine_get_main_list_block_info_by_id_t(now_group, i);
    if(info != NULL)
    { 
      if(info->block_ram_addr == NULL)
      {
        ESP_LOGE(TAG, "get list by index ram addr is NULL");
        return 1;
      }
      if(info->block_ram_addr->block_type == type && info->block_ram_addr->block_sub_type == sub_type)
      {
        if(++now_index == index)
        {
          ESP_LOGD(TAG, "found block by index");
          return 0;
        }
      }
    }
  }
  /* not found  */
  return 2;
}

void neurons_engine_parse_blocks_respond_t(neurons_commmd_package_t *package)
{
  uint8_t dev_id = package->cmd_package_info.data[1];
  uint8_t type = package->cmd_package_info.data[2];
  uint8_t sub_type = package->cmd_package_info.data[3];
  uint8_t cmd_id = package->cmd_package_info.data[4];
  uint8_t now_group = neurons_engine_get_main_list_now_group_t();
  neurons_engine_online_block_info_t *list_node_info = NULL;
  neurons_engine_online_block_lib_respond_t *respond_info = NULL;
  
  ESP_LOGD(TAG, "received package is: \n");
  neurons_show_package_t(package);
  
  uint8_t respond_num = 0;
  uint8_t data_region_start_index = 0;
  uint8_t data_region_length = 0;
  /* use to get the address of information of this respond command */
  uint8_t respond_data_in_ram[MODULE_ONLINE_INFO_SINGLE_RESPOND_LEN] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t respond_type_len = 0;
  respond_data_in_ram[0] = cmd_id;
  if(neurons_engine_find_block_in_main_list_by_id_t(dev_id, type, sub_type) == 0)
  { 
    list_node_info = neurons_engine_get_main_list_block_info_by_id_t(now_group, dev_id);
    ESP_LOGD(TAG, "list_node_info: type: %d, sub: %d, dev_id: %d, data buffer: %p ", list_node_info->block_type, list_node_info->block_sub_type, list_node_info->dev_id, list_node_info->respond_data_info.respond_data_buffer);
    neurons_engine_get_respond_info_t(list_node_info, cmd_id, &respond_data_in_ram[6], &respond_data_in_ram[1], &respond_type_len);
    respond_num = *(list_node_info->block_ram_addr->respond_data_type);
    ESP_LOGD(TAG, "respond num is %d\n", respond_num);
    if(respond_type_len != 0)
    {
      respond_info = (neurons_engine_online_block_lib_respond_t *)respond_data_in_ram;
    }
    else
    {
      ESP_LOGE(TAG, "package receive: command received is invalid");
      return;
    }
    neurons_get_data_region_from_package_t(package, &data_region_start_index, &data_region_length);
    // ESP_LOGD(TAG, "value_offset is %d\n", respond_info->respond_struct.value_offset);
    // ESP_LOGD(TAG, "respond data addr is %p\n", list_node_info->respond_data_info.respond_data_buffer);
    if(list_node_info->respond_data_info.respond_data_buffer == NULL)
    { 
      ESP_LOGE(TAG, "package receive: respond data buffer is NULL");
      return;
    }
    memcpy(list_node_info->respond_data_info.respond_data_buffer + respond_info->respond_struct.value_offset, 
           &package->cmd_package_info.data[data_region_start_index], data_region_length);

#if 0
    for(uint8_t i = 0; i < data_region_length; i++)
    { 
      ESP_LOGD(TAG, "the %d data received is %d\n", i, 
               *(list_node_info->respond_data_info.respond_data_buffer + respond_info->respond_struct.value_offset + i));
    }
#endif
  }
  else
  {
    ESP_LOGE(TAG, "package receive: block not found in main list");
  }
  /* for neurons reading */
  extern void neurons_engine_read_check_t(uint8_t, uint8_t, uint8_t, uint8_t);
  neurons_engine_read_check_t(dev_id, type, sub_type, cmd_id);

}

/* ex:allocate id */
void neurons_engine_parse_system_respond_t(neurons_commmd_package_t *package)
{
  neurons_engine_online_block_info_t block_info;
  switch(package->cmd_package_info.cmd_info.service_id)
  {
    case CTL_ASSIGN_DEV_ID:
      block_info.dev_id = package->cmd_package_info.data[1];
      block_info.block_type = package->cmd_package_info.data[3];
      block_info.block_sub_type = package->cmd_package_info.data[4];
      // ESP_LOGD(TAG, "dev id is %d, type is %d, sub type is %d", block_info.dev_id, block_info.block_type, block_info.block_sub_type);
      neurons_engine_add_new_block_t(&block_info);
    break;
    case CTL_SYSTEM_RESET:
    break;
    case CTL_QUERY_FIRMWARE_VERSION:
    break;
    case CTL_SET_BAUDRATE:
    break;
    default:
    break;
      
  }
}
void neurons_engine_parse_general_respond_t(neurons_commmd_package_t *package)
{
  
}

/* common command respond || block command respond */
void neurons_engine_parse_package_t(neurons_commmd_package_t *package)
{
  uint8_t command_type = neurons_engine_parse_respond_type_t(package);
  // ESP_LOGD(TAG, "neurons_engine get command type is %d\n", command_type);
  
  /* system respond */
  if(command_type == 1)
  {
    neurons_engine_parse_system_respond_t(package);
  }
  /*general respond */
  else if(command_type == 2)
  {
    neurons_engine_parse_general_respond_t(package);
  }
  /* blocks respond */
  else
  {
    neurons_engine_parse_blocks_respond_t(package);
  }
  
}


/* send heart package per 500ms */
void rocky_sensor_set_report_status_t(uint8_t index, uint8_t sta)
{
  if(index > ROCKY_SENSOR_REPORT_MAX || index < ROCKY_SENSOR_REPORT_MIN)
  {
    return;
  }
  s_rocky_sensor_report_status[index] = sta;
  
}

int8_t rocky_sensor_get_report_status_t(uint8_t index)
{
  if(index > ROCKY_SENSOR_REPORT_MAX || index < ROCKY_SENSOR_REPORT_MIN)
  {
    return -1;
  }
  return s_rocky_sensor_report_status[index];
}

void neurons_engine_reset_rocky_report_mode_t(void)
{
  for(uint8_t i = ROCKY_SENSOR_REPORT_MIN; i <= ROCKY_SENSOR_REPORT_MAX; i++)
  {
    rocky_sensor_set_report_status_t(i, 0);
  }
}

void neurons_engine_rocky_stop()
{
  neurons_comnad_type_t cmd_type = 
  {
    .type_id = 0x63,
    .sub_type_id = 0x10,
    .index = 1,
    .cmd_id = 0x0c,
  };
  neurons_engine_send_single_command_t(&cmd_type);
}

void neurons_engine_send_heart_package_t(void)
{
  static uint32_t last_millis = 0;
  static uint32_t heard_package_count = 0;
  if(!neurons_engine_get_lib_found_flag_t())
  {
     return;
  }
  if(millis() - last_millis >= NEURONS_HEART_INTERVAL)
  { 
    neurons_engine_online_blocks_update_t();
    if(heard_package_count == 1)
    { 
      /* after update the list firstly, enable the mp python */
      neurons_engine_set_status_t(1);
      neurons_engine_rocky_stop();
    }
    neurons_engine_alloc_id_t();
    last_millis = millis();
    heard_package_count++;
  }

}

/* system common command */
void neurons_engine_alloc_id_t(void)
{  
  neurons_command_frame_t frame = {.head = START_SYSEX, .device_id = ALL_DEVICE, .service_id = CTL_ASSIGN_DEV_ID, .end = END_SYSEX};
                                  
  neurons_commmd_package_t package = {.data_in_index = 0};
  neurons_buffer_add_head_frame_t(&frame, &package, false);
  uint8_t temp = 0x00; // data value
  neurons_buffer_add_data_t(&package, BYTE_8_T, &temp);
  neurons_buffer_add_end_frame_t(&frame, &package);
  neurons_command_packege_send_all(&package, CODEY_UART1_ID);
}

void neurons_engine_reset_block_t(uint8_t block_id)
{
  neurons_command_frame_t frame = {.head = START_SYSEX, .device_id = block_id, .service_id = CTL_SYSTEM_RESET, .end = END_SYSEX};
                                  
  neurons_commmd_package_t package = {.data_in_index = 0};
  neurons_buffer_add_head_frame_t(&frame, &package, false);
  neurons_buffer_add_end_frame_t(&frame, &package);
  neurons_command_packege_send_all(&package, CODEY_UART1_ID);

}

void neurons_engine_check_block_hardware_version_t(uint8_t block_id)
{
  neurons_command_frame_t frame = {.head = START_SYSEX, .device_id = block_id, .service_id = CTL_QUERY_FIRMWARE_VERSION, .end = END_SYSEX};
                                  
  neurons_commmd_package_t package = {.data_in_index = 0};
  neurons_buffer_add_head_frame_t(&frame, &package, false);
  neurons_buffer_add_end_frame_t(&frame, &package);
  neurons_command_packege_send_all(&package, CODEY_UART1_ID);  
}
/* rate can be 0/1/2(9600/115200/921600) */
void neurons_engine_set_block_baudrate_t(uint8_t block_id, uint8_t rate_index)
{
  neurons_command_frame_t frame = {.head = START_SYSEX, .device_id = block_id, .service_id = CTL_SET_BAUDRATE, .end = END_SYSEX};
                                  
  neurons_commmd_package_t package = {.data_in_index = 0};
  neurons_buffer_add_head_frame_t(&frame, &package, false);
  neurons_buffer_add_data_t(&package, BYTE_8_T, &rate_index);
  neurons_buffer_add_end_frame_t(&frame, &package);
  neurons_command_packege_send_all(&package, CODEY_UART1_ID);  

}

/* send 1 - 5 */
void neurons_engine_test_command_t(uint8_t block_id)
{
  neurons_command_frame_t frame = {.head = START_SYSEX, .device_id = block_id, .service_id = CTL_COMMUNICATION_TEST, .end = END_SYSEX};
                                  
  neurons_commmd_package_t package = {.data_in_index = 0};
  neurons_buffer_add_head_frame_t(&frame, &package, false);
  for(uint8_t i = 1; i < 6; i++)
  {
    neurons_buffer_add_data_t(&package, BYTE_8_T, &i);
  }
  neurons_buffer_add_end_frame_t(&frame, &package);
  neurons_command_packege_send_all(&package, CODEY_UART1_ID);  
}

/* module common command */
void neurons_engine_set_if_respond_to_common_cmd_t(uint8_t block_id, uint8_t sta)
{
  neurons_command_frame_t frame = {.head = START_SYSEX, .device_id = block_id, .service_id = CTL_GENERAL, .sub_service_id = CTL_SET_FEEDBACK,
                                   .end = END_SYSEX};
                                  
  neurons_commmd_package_t package = {.data_in_index = 0};
  neurons_buffer_add_head_frame_t(&frame, &package, true);
  if(sta != 0)
  {
    sta = 1;
  }
  neurons_buffer_add_data_t(&package, BYTE_8_T, &sta);
  neurons_buffer_add_end_frame_t(&frame, &package);
  neurons_command_packege_send_all(&package, CODEY_UART1_ID);  
}

void neurons_engine_set_indicate_led_t(uint8_t block_id, uint16_t r, 
                                                       uint16_t g, uint16_t b)
{
  neurons_command_frame_t frame = {.head = START_SYSEX, .device_id = block_id, .service_id = CTL_GENERAL, .sub_service_id = CTL_SET_RGB_LED,
                                   .end = END_SYSEX};
                                  
  neurons_commmd_package_t package = {.data_in_index = 0};
  neurons_buffer_add_head_frame_t(&frame, &package, true);

  neurons_buffer_add_data_t(&package, SHORT_16_T, &r);
  neurons_buffer_add_data_t(&package, SHORT_16_T, &g);
  neurons_buffer_add_data_t(&package, SHORT_16_T, &b);
  
  neurons_buffer_add_end_frame_t(&frame, &package);
  neurons_command_packege_send_all(&package, CODEY_UART1_ID);  
}

void neurons_engine_find_block_t(uint8_t block_id, uint8_t blink_times)
{
  neurons_command_frame_t frame = {.head = START_SYSEX, .device_id = block_id, .service_id = CTL_GENERAL, .sub_service_id = CTL_FIND_BLOCK,
                                   .end = END_SYSEX};
                                  
  neurons_commmd_package_t package = {.data_in_index = 0};
  neurons_buffer_add_head_frame_t(&frame, &package, true);
  neurons_buffer_add_data_t(&package, BYTE_8_T, &blink_times);
  neurons_buffer_add_end_frame_t(&frame, &package);
  neurons_command_packege_send_all(&package, CODEY_UART1_ID);  
}
/* set report mode */
void neurons_engine_set_report_mode_t(uint8_t block_id, uint8_t block_type, uint8_t sub_type, 
                                                      uint8_t mode, long period)
{
  neurons_command_frame_t frame = {.head = START_SYSEX, .device_id = block_id, .service_id = block_type, .sub_service_id = sub_type,
                                   .end = END_SYSEX};
                                  
  neurons_commmd_package_t package = {.data_in_index = 0};
  if(sub_type != 0)
  {
    neurons_buffer_add_head_frame_t(&frame, &package, true);
  }
  else
  {
    neurons_buffer_add_head_frame_t(&frame, &package, false);
  }
  uint8_t temp = 0x7f;
  neurons_buffer_add_data_t(&package, BYTE_8_T, &temp);
  neurons_buffer_add_data_t(&package, BYTE_8_T, &mode);
  neurons_buffer_add_data_t(&package, LONG_40_T, &period);
  neurons_buffer_add_end_frame_t(&frame, &package);
  
  neurons_command_packege_send_all(&package, CODEY_UART1_ID);    
}


/* special cmd to set rocky report, just as a test */
void neurons_engine_set_rocky_report_mode_t(uint8_t block_id, uint8_t sensor_id, uint8_t mode,
                                                               long period)
{
  neurons_command_frame_t frame = {.head = START_SYSEX, .device_id = block_id, .service_id = CLASS_SENSOR, .sub_service_id = BLOCK_CODEY_CAR,
                                   .end = END_SYSEX};
                                  
  neurons_commmd_package_t package = {.data_in_index = 0};

  neurons_buffer_add_head_frame_t(&frame, &package, true);

  uint8_t temp = 0x7f;
  neurons_buffer_add_data_t(&package, BYTE_8_T, &temp);
  temp = sensor_id;
  neurons_buffer_add_data_t(&package, BYTE_8_T, &temp);
  
  neurons_buffer_add_data_t(&package, BYTE_8_T, &mode);
  neurons_buffer_add_data_t(&package, LONG_40_T, &period);
  // neurons_show_package_t(&package);
  neurons_buffer_add_end_frame_t(&frame, &package);
  
  neurons_command_packege_send_all(&package, CODEY_UART1_ID);    
}

/* neurons command send functions */ 
void neurons_engine_command_buffer_init(void)
{
  memset(&neurons_command_buffer, 0, sizeof(neurons_command_buffer));
}

void neurons_engine_command_buffer_push(neurons_comnad_type_t *cmd_type)
{

  while(neurons_command_buffer.cmd_cash_out_index == ((neurons_command_buffer.cmd_cash_in_index + 1) & (CMD_BUFFER_MAX - 1)))
  {
    // ESP_LOGD( TAG, "command buffer run out, must wait");
    vTaskDelay( CMD_BUFFER_WAIT_MS/portTICK_PERIOD_MS); 
  }
  
  memcpy(&neurons_command_buffer.neurons_comnad_type[neurons_command_buffer.cmd_cash_in_index++], cmd_type, sizeof(neurons_comnad_type_t));
  neurons_command_buffer.cmd_cash_in_index &= (CMD_BUFFER_MAX - 1);
  // ESP_LOGD(TAG, "buffer index in is %d", neurons_command_buffer.cmd_cash_in_index);
}

uint8_t neurons_engine_command_buffer_pop(neurons_comnad_type_t *cmd_type)
{
  if(neurons_command_buffer.cmd_cash_out_index != neurons_command_buffer.cmd_cash_in_index)
  {
    memcpy(cmd_type, &neurons_command_buffer.neurons_comnad_type[neurons_command_buffer.cmd_cash_out_index++],
           sizeof(neurons_comnad_type_t));
    neurons_command_buffer.cmd_cash_out_index &= (CMD_BUFFER_MAX - 1);
    // ESP_LOGD(TAG, "buffer index out is %d", neurons_command_buffer.cmd_cash_in_index);
    return 1;
  }
  else
  {
    //ESP_LOGD(TAG, "buffer index out is lll%d", neurons_command_buffer.cmd_cash_in_index);
    return 0; // no data pop
  }
}

uint8_t neurons_engine_send_single_command_t(neurons_comnad_type_t *cmd_type)
{ 
  /* type, [sub_type], index, [cmd_id, parameter, para2, ...] */
  uint8_t type = cmd_type->type_id;
  uint8_t sub_type = cmd_type->sub_type_id;
  uint8_t index = cmd_type->index;
  uint8_t cmd_id = cmd_type->cmd_id;
  
  uint8_t cmd_data_type[RES_CMD_TYPE_MAX];
  uint8_t cmd_type_len = 0;
  neurons_engine_online_block_info_t *info = NULL;
  info = neurons_engine_get_main_list_block_info_by_index_t(index, type, sub_type);
  if(info == NULL)
  {
    ESP_LOGE(TAG, "not found block: index is %d, type is%d, sub is %d",index, type, sub_type);    
    return 0;
  }
  neurons_engine_get_command_info_t(info, cmd_id, cmd_data_type, &cmd_type_len);
  /* command id invalid */
  if(cmd_type_len == 0)
  {
    ESP_LOGE(TAG, "cmd id invalid");
    return 0;
  }
  neurons_command_frame_t frame = {.head = START_SYSEX, .device_id = info->dev_id, .service_id = info->block_type,
                                   .sub_service_id = info->block_sub_type, .end = END_SYSEX};
  
  neurons_commmd_package_t package = {.data_in_index = 0};
  neurons_buffer_add_head_frame_t(&frame, &package, true);
  neurons_buffer_add_data_t(&package, BYTE_8_T, &cmd_id);

  uint8_t type_data_index = 0;
  ESP_LOGD(TAG, "send command cmd type len is: %d", cmd_type_len);

  uint8_t type_number = 0;
  for(uint8_t i = 1; i < cmd_type_len; i++)
  {
    /* special data type dealing */
    if((cmd_data_type[i] & 0xf0) == ALTERABLE_NUM_BYTE)
    {
      /* the first type can't be  ALTERABLE_NUM_BYTE */
      if(i == 1)
      {
        return 0;
      }
      type_number = (uint8_t)(cmd_type->cmd_data[type_data_index - 1]);
      ESP_LOGD(TAG, "the number of alterable type is %d, type is %d", type_number, (cmd_data_type[i] & 0x0f));
    }
    /* general data type  */
    else
    {
      type_number = ((cmd_data_type[i] & 0xf0) >> 4);
      type_number = (type_number == 0 ? 1 : type_number);  
    }
    
    for(uint8_t j = 0; j < type_number; j++)
    {
      switch(cmd_data_type[i] & 0x0f)
      { 
        case BYTE_8_T:  
          { 
            uint8_t data_s = (uint8_t)(cmd_type->cmd_data[type_data_index]);
            neurons_buffer_add_data_t(&package, BYTE_8_T, &data_s);
          }
        break;
        case BYTE_16_T: 
          {
            int8_t data_s = (int8_t)(cmd_type->cmd_data[type_data_index]);
            neurons_buffer_add_data_t(&package, BYTE_16_T, &data_s);
          }
        break;
        case SHORT_16_T:
          { 
            uint16_t data_s = (uint16_t)(cmd_type->cmd_data[type_data_index]);
            neurons_buffer_add_data_t(&package, SHORT_16_T, &data_s);
          }
        break;
        case SHORT_24_T: 
          {
            int16_t data_s = (int16_t)(cmd_type->cmd_data[type_data_index]);
            neurons_buffer_add_data_t(&package, SHORT_24_T, &data_s);
  
          }
        break;
        case LONG_40_T: 
          {
            long data_s = (long)(cmd_type->cmd_data[type_data_index]);
            neurons_buffer_add_data_t(&package, LONG_40_T, &data_s);
          }
        break;
        case FLOAT_40_T:
          {
            neurons_buffer_add_data_t(&package, FLOAT_40_T, &cmd_type->cmd_data[type_data_index]);
          }
        break;
        default:
            
        break;
      }
      type_data_index++;
    }
    ESP_LOGD(TAG, "data type is %d", cmd_data_type[i]);
  }
  neurons_show_package_t(&package);
  neurons_buffer_add_end_frame_t(&frame, &package);
  neurons_command_packege_send_all(&package, CODEY_UART1_ID);
  return 1;
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC void neurons_engine_read_init_t(void)
{
  s_neurons_read_event_group = xEventGroupCreate();
}

STATIC bool neurons_engine_online_block_list_is_empty_t(void)
{
  if(s_neurons_engine_online_block_lib_list_head == NULL)
  {
    return true;
  }
  else
  {
    return false;
  }
}

STATIC void neurons_engine_online_block_list_add_t(neurons_engine_online_block_lib_list_t *new_node)
{
  neurons_engine_online_block_lib_list_t *p_next = s_neurons_engine_online_block_lib_list_head;
  if(neurons_engine_online_block_list_is_empty_t())
  {
    s_neurons_engine_online_block_lib_list_head = new_node;
    new_node->next = NULL;
    return;
  }
  
  while(p_next->next != NULL)
  {
    p_next = p_next->next;
  }
  p_next->next = new_node;
  new_node->next = NULL;
}

STATIC void neurons_engine_online_block_list_delete_t(neurons_engine_online_block_lib_list_t *node)
{
  neurons_engine_online_block_lib_list_t *p_next = s_neurons_engine_online_block_lib_list_head;
  neurons_engine_online_block_lib_list_t *p_pre = s_neurons_engine_online_block_lib_list_head;

  if(neurons_engine_online_block_list_is_empty_t())
  {
    return;
  }

  if(node == s_neurons_engine_online_block_lib_list_head)
  {
    s_neurons_engine_online_block_lib_list_head = node->next;
    heap_caps_free(node);
    return;
  }

  while(p_next != node && p_next != NULL)
  { 
    p_pre = p_next;
    p_next = p_next->next;
  }
  
  p_pre->next = p_next->next;
  if(p_next != NULL)
  {
    heap_caps_free(p_next);
  }
}

STATIC neurons_engine_online_block_lib_t *neurons_engine_online_block_list_check_t(uint8_t type, uint8_t sub_type)
{
  neurons_engine_online_block_lib_list_t *p_next = s_neurons_engine_online_block_lib_list_head;

  if(neurons_engine_online_block_list_is_empty_t())
  {
    return NULL;
  }
  
  while(p_next != NULL && (p_next->lib_addr->block_type != type || p_next->lib_addr->block_sub_type != sub_type))
  { 
    p_next = p_next->next;
  }
  if(p_next == NULL)
  {
    return NULL;
  }
  else
  {
    return p_next->lib_addr;
  }
}

STATIC void neurons_engine_online_block_list_clear_all_block_num(void)
{
  neurons_engine_online_block_lib_list_t *p_next = s_neurons_engine_online_block_lib_list_head;
  if(s_neurons_engine_online_block_lib_list_head == NULL)
  {
    return;
  }
  
  while(p_next != NULL)
  {
    if(p_next->lib_addr != NULL)
    {
      p_next->lib_addr->block_num = 0;
    }
    p_next = p_next->next;
  }

}

STATIC void neurons_engine_online_block_list_not_used_free(void)
{
  neurons_engine_online_block_lib_list_t *p_next = s_neurons_engine_online_block_lib_list_head;
  neurons_engine_online_block_lib_list_t *free_addr = s_neurons_engine_online_block_lib_list_head;
  
  while(p_next != NULL)
  {
    if(p_next->lib_addr->block_num == 0)
    {
      free_addr = p_next;
      p_next = p_next->next;
      
      // ESP_LOGD(TAG, "type is%d sub is%d", free_addr->lib_addr->block_type, free_addr->lib_addr->block_sub_type);
      heap_caps_free(free_addr->lib_addr->command_data_type);
      heap_caps_free(free_addr->lib_addr->respond_data_type);
      heap_caps_free(free_addr->lib_addr);
      neurons_engine_online_block_list_delete_t(free_addr); 
    }
    else
    {
      p_next = p_next->next;
    }
  }  
  // ESP_LOGD(TAG, "free succeed");
}

