/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     FF 55 common protocol
 * @file      codey_ff_55.c
 * @author    Leo
 * @version V1.0.0
 * @date      2017/11/15
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
 * This file include some system function.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *   Leo              2017/11/17      1.0.0            build the new.
 * </pre>* </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "py/mpstate.h"
#include "py/runtime.h"
#include "codey_firmware.h"
#include "codey_get_chn.h"
#include "codey_ff_55_comm_protocol.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#undef  TAG
#define TAG                     ("FF_55")
#define FW_FRAME_HEADER_1       (0xFF)
#define FW_FRAME_HEADER_2       (0x55)
#define FW_FRAME_END_1          (0x0D)
#define FW_FRAME_END_2          (0x0A)
#define FW_MAX_DATA_LEN         (0xF0)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/ 
typedef enum
{
  S_FW_HEAD_1 = 0,
  S_FW_HEAD_2,
  S_FW_LEN,
  S_FW_INDEX,
  S_FW_MODE,
  S_FW_CMD,
  S_FW_DATA,
  S_FW_END
} fw_fsm_state_t;

typedef enum
{
  FW_MODE_READ = 0x01,
  FW_MODE_WRITE = 0x02,
} fw_mode_t;

typedef enum
{
  FW_CMD_FIRMWARE_ID = 0x00,
  FW_CMD_GET_CHN_ID = 0xf0,
  FW_CMD_NUM = 3,
} fw_cmd_id_t;

typedef void (*fw_pf_t)(channel_data_tag_t chn_tag, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);

typedef struct
{
  fw_mode_t         cmd_mode;
  fw_cmd_id_t       cmd_id;
  uint8_t           exp_len;
  fw_pf_t           cmd_exe;
} fw_cmd_elem_t ;

typedef struct
{
  fw_fsm_state_t  s;
  uint8_t         recv_len;
  uint8_t         len;
  uint8_t         index;
  fw_mode_t       mode;
  uint8_t         cmd;
  uint8_t         data[FW_MAX_DATA_LEN];
} fw_fsm_t;

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
static fw_cmd_elem_t *codey_cmd_elem_mode_id(fw_mode_t cmd_mode, fw_cmd_id_t cmd_id);
static void codey_ff_55_comm_build_frame(uint8_t cmd_index, uint8_t *in_data, uint8_t in_data_len, uint8_t *out_buf, uint8_t *out_len);
static void codey_fw_read_firmware_version(channel_data_tag_t chn_tag, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
static void codey_fw_read_commu_chn(channel_data_tag_t chn_tag, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
static fw_fsm_t s_fw_fsm_tab[COMM_CHN_NUM] = { 0, };
static fw_cmd_elem_t s_fw_cmd_tab[FW_CMD_NUM] = 
{
  { FW_MODE_READ, FW_CMD_FIRMWARE_ID,     0, codey_fw_read_firmware_version },
  { FW_MODE_READ, FW_CMD_GET_CHN_ID,      0, codey_fw_read_commu_chn },
};

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_ff_55_protocol_pump_char(channel_data_tag_t chn_tag, uint8_t c, uint8_t *output_buf, uint32_t *output_len)
{
  fw_fsm_t *p_fsm;
  fw_cmd_elem_t *p_cmd;
  
  if(!output_buf || !output_len)
  {
    return;
  }

  if(chn_tag > COMM_CHN_NUM)
  {
    return;
  }

  *output_len = 0;
  p_fsm = &s_fw_fsm_tab[chn_tag];

  // ESP_LOGD(TAG, "channel tag: %d", chn_tag);
  // ESP_LOGD(TAG, "recv: %c", c);
  // ESP_LOGD(TAG, "fsm: %d", p_fsm->s);

  switch(p_fsm->s)
  {
    case S_FW_HEAD_1:
      if(FW_FRAME_HEADER_1 == c)
      {
        p_fsm->s = S_FW_HEAD_2;
      }
    break;

    case S_FW_HEAD_2:
      if(FW_FRAME_HEADER_2 == c)
      {
        p_fsm->s = S_FW_LEN;
      }
    break;

    case S_FW_LEN:
      p_fsm->len = c;
      p_fsm->recv_len = 0;
      p_fsm->s = S_FW_INDEX;
    break;

    case S_FW_INDEX:
      p_fsm->index = c;
      p_fsm->data[p_fsm->recv_len++] = c;
      p_fsm->s = S_FW_MODE;
    break;

    case S_FW_MODE:
      p_fsm->mode = c;
      p_fsm->data[p_fsm->recv_len++] = c;
      p_fsm->s = S_FW_CMD;
    break;

    case S_FW_CMD:
      p_fsm->cmd = c;
      p_fsm->data[p_fsm->recv_len++] = c;
      p_cmd = codey_cmd_elem_mode_id(p_fsm->mode, p_fsm->cmd);
      if(p_cmd)
      {
        if(p_cmd->exp_len)
        {
          p_fsm->s = S_FW_DATA;
        }
        else
        {
          if(p_cmd->cmd_exe)
          {
            p_cmd->cmd_exe(chn_tag, p_fsm->data, p_fsm->recv_len, output_buf, output_len);
          }
          p_fsm->s = S_FW_HEAD_1; 
        }
      }
      else
      {
        p_fsm->s = S_FW_HEAD_1; 
      }
    break;

    case S_FW_DATA:
      p_fsm->data[p_fsm->recv_len++] = c;
      if(p_fsm->recv_len == p_fsm->len)
      {
        p_cmd = codey_cmd_elem_mode_id(p_fsm->mode, p_fsm->cmd);
        if(p_cmd)
        {
          if(p_cmd->cmd_exe)
          {
            p_cmd->cmd_exe(chn_tag, p_fsm->data, p_fsm->recv_len, output_buf, output_len);
          }
        }
        p_fsm->s = S_FW_HEAD_1;     
      }
    break;

    case S_FW_END:
    break;
  }
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
static fw_cmd_elem_t *codey_cmd_elem_mode_id(fw_mode_t cmd_mode, fw_cmd_id_t cmd_id)
{
  uint32_t i;

  for(i = 0; i < FW_CMD_NUM; i++)
  {
    if(cmd_id == s_fw_cmd_tab[i].cmd_id && cmd_mode == s_fw_cmd_tab[i].cmd_mode)
    {
      return &(s_fw_cmd_tab[i]);
    }
  }

  return NULL;
}

static void codey_ff_55_comm_build_frame(uint8_t cmd_index, uint8_t *in_data, uint8_t in_data_len, uint8_t *out_buf, uint8_t *out_len)
{
  uint8_t pos;
  
  /*header1 +header2 + index + data + 0x0d + 0x0a */
  pos = 0;
  out_buf[pos++] = FW_FRAME_HEADER_1;
  out_buf[pos++] = FW_FRAME_HEADER_2;
  out_buf[pos++] = cmd_index;
  memcpy(out_buf + pos, in_data, in_data_len);
  pos += in_data_len;
  out_buf[pos++] = FW_FRAME_END_1;
  out_buf[pos++] = FW_FRAME_END_2;

  *out_len = pos;
}

static void codey_fw_read_firmware_version(channel_data_tag_t chn_tag, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  size_t firmware_str_len;
  const char *firmware;
  uint8_t *buf;

  // ESP_LOGD(TAG, "%s", __FUNCTION__);

  firmware = codey_get_firmware();
  firmware_str_len = strlen(firmware);

  if(firmware_str_len < FW_MAX_DATA_LEN)
  {
    buf = malloc(firmware_str_len + 2);
    if(buf)
    {
      buf[0] = FW_DATA_TYPE_STR;
      buf[1] = firmware_str_len;
      memcpy(buf + 2, firmware, firmware_str_len);
      codey_ff_55_comm_build_frame(data[0], buf, firmware_str_len + 2, output_buf, (uint8_t *)output_len);
      free(buf);
    }
    else
    {
      *output_len = 0;  
    }
  }
  else
  {
    *output_len = 0;
  }
}

static void codey_fw_read_commu_chn(channel_data_tag_t chn_tag, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  // ESP_LOGD(TAG, "%s", __FUNCTION__);

  size_t chanel_str_len;
  const char *channel_str;
  uint8_t *buf;
  
  
  if(data[0] < COMM_CHN_NUM)
  {
    channel_str = codey_get_chn_tag(chn_tag);
    chanel_str_len = strlen(channel_str);
    if(chanel_str_len < FW_MAX_DATA_LEN)
    {
      buf = malloc(chanel_str_len + 2);
      if(buf)
      {
        buf[0] = FW_DATA_TYPE_STR;
        buf[1] = chanel_str_len;
        memcpy(buf + 2, channel_str, chanel_str_len);
        codey_ff_55_comm_build_frame(data[0], buf, chanel_str_len + 2, output_buf, (uint8_t *)output_len);
        free(buf);
      }
      else
      {
        *output_len = 0;  
      } 
    }
  }
  else
  {
    *output_len = 0;
  }
}
