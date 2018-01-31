/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     The basis of the function for makeblock.
 * @file      codey_message.c
 * @author    Leo
 * @version   V1.0.0
 * @date      2017/10/13
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
 * This file include message up function.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  Leo               2017/10/13      1.0.0               build the new.
 * </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>	

#include "py/mphal.h"
#include "py/mpstate.h"
#include "py/runtime.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_task.h"
#include "esp_log.h"

#include "driver/uart.h"
#include "driver/adc.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "codey_ble_sys_dev_func.h"
#include "codey_utils.h"
#include "codey_message.h"
#include "codey_event_mechanism.h"
#include "codey_comm_protocol.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#undef    TAG
#define   TAG                           ("CODEY_MSG")
#define   BDCAST_MSG_TX_BUF_SIZE        (256)
#define   UART                          (UART_NUM_0)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
typedef struct
{
  mp_obj_base_t base;
} codey_message_obj_t;

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
const  mp_obj_type_t codey_message_type;
static uint8_t s_bdcast_buf[BDCAST_MSG_TX_BUF_SIZE];
static codey_message_obj_t s_codey_message_obj = {.base = {&codey_message_type}};

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_message_recv(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  if(0 == data[len-1])
  {
    // ESP_LOGI(TAG, "Trigger message: %s", (char *)data);
    codey_eve_trigger_t(EVE_MESSAGE, (void *)data);
  }
  else
  {
    ESP_LOGI(TAG, "data[len] is NOT 0\r\n");
  }
  *output_len = 0;
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC void codey_message_send(const uint8_t *msg)
{
  uint32_t data_len;

  data_len = strlen((const char *)msg) + 1;
  strcpy((char *)s_bdcast_buf, (char *)msg);
  codey_comm_build_frame(MSG_BCAST_ID, s_bdcast_buf, &data_len);

  // ESP_LOGI(TAG, "Send to UART");
  // codey_print_hex(s_bdcast_buf, data_len);
  // ESP_LOGI(TAG, "");
  uart_write_bytes(UART, (const char *)s_bdcast_buf, data_len);
  codey_ble_dev_put_data(s_bdcast_buf, data_len);
}

STATIC mp_obj_t codey_message_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  codey_message_obj_t *self = &s_codey_message_obj;
  return self;
}

STATIC mp_obj_t codey_message_broadcast(mp_obj_t self_in, mp_obj_t arg1)
{
  size_t len;
  const char *data = NULL;

  // ESP_LOGI(TAG, "codey_message_broadcast");
  if(MP_OBJ_IS_STR(arg1))
  {
    data = mp_obj_str_get_data(arg1, &len);
    if(data)
    {
      codey_eve_trigger_t(EVE_MESSAGE, (void *)data);
      codey_message_send((const uint8_t *)data);
      return mp_const_true;
    }
    else
    {
      return mp_const_false;
    }
  }
  else
  {
    return mp_const_false; 
  }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_message_broadcast_obj, codey_message_broadcast);

STATIC const mp_map_elem_t codey_message_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_message_advance),          (mp_obj_t)&codey_message_broadcast_obj },
};

STATIC MP_DEFINE_CONST_DICT(codey_message_locals_dict, codey_message_locals_dict_table);
const mp_obj_type_t codey_message_type =
{
  { &mp_type_type },
  .name = MP_QSTR_message_board,
  .make_new = codey_message_make_new,
  .locals_dict = (mp_obj_t)&codey_message_locals_dict,
};
