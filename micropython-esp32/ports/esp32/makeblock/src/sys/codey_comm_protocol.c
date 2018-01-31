/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief       The basis of the function for makeblock.
 * @file        codey_comm_protocol.c
 * @author      Leo lu
 * @version V1.0.0
 * @date        2017/09/29
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
 * Leo lu             2017/09/29      1.0.0              Build the new.
 * </pre>
 *
 */
 
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "esp_log.h"
#include "codey_neurons_ftp.h"
#include "codey_super_var.h"
#include "codey_comm_protocol.h"
#include "codey_sensor_report.h"
#include "codey_message.h"
#include "codey_firmware.h"
#include "codey_get_mac.h"
#include "codey_utils.h"
#include "codey_dtr_req.h"
#include "codey_ff_55_comm_protocol.h"
#include "codey_ready_notify.h"
#include "codey_get_chn.h"
#include "codey_product_test.h"
#include "codey_baudrate_setting.h"
#include "codey_get_filesys.h"


/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/ 
#undef  TAG
#define TAG                     ("COMM_PROTOCOL")
#define FRAME_HEADER            (0xF3)
#define FRAME_END               (0xF4)
#define FRAME_DATA_MAX_LEN      (256 + 128)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/  
typedef enum
{
  S_HEAD = 0,
  S_HEAD_CHECK,
  S_LEN1,
  S_LEN2,
  S_DATA,
  S_CHECK,
  S_END,
} fsm_state_t;

typedef struct
{
  fsm_state_t   s;
  uint8_t       recv_head_check;
  uint8_t       calc_head_check;
  uint16_t      len;
  uint16_t      recv_len;
  uint8_t       check;
  uint8_t       data[FRAME_DATA_MAX_LEN];
} fsm_t;

typedef void (*pf_protocol_t)(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);

/******************************************************************************
 DEFINE PRIVATE DATAS 
 ******************************************************************************/  
static fsm_t s_fsm_tab[COMM_CHN_NUM] = { 0, };
static pf_protocol_t s_protocol_tab[PROTOCOL_NUM] = 
{ 
  [0] = NULL,
  [1] = codey_neurons_ftp, 
  [2] = codey_super_var, 
  [3] = codey_message_recv, 
  [4] = codey_sensor_report_request,
  [5] = codey_get_mac,
  [6] = codey_comm_get_firmware,
  [7] = codey_dtr_req,
  [8] = codey_ready,
  [9] = codey_get_chn,
  [10] = codey_product_test_request,
  [11] = codey_comm_baudrate_setting,
  [12] = codey_get_filesys,
};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
static pf_protocol_t codey_get_protocol_by_id(protocol_id_t id);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_comm_protocol_pump_char(channel_data_tag_t chn_tag, uint8_t c, uint8_t *output_buf, uint32_t *output_len)
{
  fsm_t *p_fsm;
  pf_protocol_t pf;
  
  if(!output_buf || !output_len)
  {
    return;
  }

  if(chn_tag > COMM_CHN_NUM)
  {
    return;
  }  

  p_fsm = &s_fsm_tab[chn_tag];

  *output_len = 0;
  switch(p_fsm->s)
  {
    case S_HEAD:
      if(FRAME_HEADER == c)
      {
        p_fsm->len = 0;
        p_fsm->recv_head_check = 0;
        p_fsm->calc_head_check = FRAME_HEADER;
        p_fsm->recv_len = 0;
        p_fsm->check = 0;
        p_fsm->s = S_HEAD_CHECK;
      }
    break;

    case S_HEAD_CHECK:
      p_fsm->recv_head_check = c;
      p_fsm->s = S_LEN1;
    break;

    case S_LEN1:
      p_fsm->calc_head_check += c;
      p_fsm->len += c;
      p_fsm->s = S_LEN2;
    break;

    case S_LEN2:
      p_fsm->calc_head_check += c;
      p_fsm->len += c * 256;
      if(p_fsm->recv_head_check == p_fsm->calc_head_check)
      {
        p_fsm->s = S_DATA;
      }
      else
      {
        p_fsm->s = S_HEAD;
      }
    break;

    case S_DATA:
      p_fsm->check += c;
      p_fsm->data[p_fsm->recv_len++] = c;
      if(p_fsm->recv_len == p_fsm->len)
      {
        ESP_LOGI(TAG, "Excpeted check sum: %02x", p_fsm->check);
        p_fsm->s = S_CHECK;
      }
    break;

    case S_CHECK:
      if(p_fsm->check == c)
      {
        p_fsm->s = S_END;
      }
      else
      {
        p_fsm->s = S_HEAD;
      }
    break;

    case S_END:
      if(FRAME_END == c)
      {
        pf = codey_get_protocol_by_id(p_fsm->data[0]);
        if(pf)
        {
          pf(chn_tag, p_fsm->data + 1, p_fsm->len - 1, output_buf, output_len);
          if(*output_len)
          {
            codey_comm_build_frame(p_fsm->data[0], output_buf, output_len);
          }
        }
      }
      p_fsm->s = S_HEAD;
    break;
  }

  if(!(*output_len))
  {
    codey_ff_55_protocol_pump_char(chn_tag, c, output_buf, output_len);
  }
}

void codey_comm_build_frame(uint8_t protocol_id, uint8_t *inoutput_data, uint32_t *inoutput_len)
{
  uint16_t pos;
  uint8_t *tmp_buf;
  
  /*header + head_check + len1 + len 2 + protocol_id + data + checksum + end */
  tmp_buf = pvPortMalloc((*inoutput_len) + 7);
  if(!tmp_buf)
  {
    *inoutput_len = 0;
    return;
  }

  pos = 0;
  tmp_buf[pos++] = FRAME_HEADER;
  tmp_buf[pos++] = FRAME_HEADER + ((*inoutput_len + 1)&(0xFF)) + (((*inoutput_len + 1)>>8)&0xFF);
  tmp_buf[pos++] = (*inoutput_len + 1)&(0xFF);
  tmp_buf[pos++] = ((*inoutput_len + 1)>>8)&0xFF;
  tmp_buf[pos++] = protocol_id;

  memcpy(tmp_buf+pos, inoutput_data, *inoutput_len);
  pos += (*inoutput_len);

  tmp_buf[pos] = codey_calc_add_check_sum(inoutput_data, *inoutput_len);
  tmp_buf[pos++] += protocol_id;

  tmp_buf[pos++] = FRAME_END;

  *inoutput_len = pos;
  memcpy(inoutput_data, tmp_buf, *inoutput_len);

  vPortFree(tmp_buf);
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/ 
static pf_protocol_t codey_get_protocol_by_id(protocol_id_t id)
{
 if(id > PROTOCOL_NUM)
 {
   return NULL;
 }

 return s_protocol_tab[id];
}


