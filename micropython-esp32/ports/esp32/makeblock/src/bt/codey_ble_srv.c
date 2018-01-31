/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     
 * @file      codey_ble_srv.c
 * @author    Leo lu
 * @version   V1.0.0
 * @date      2017/06/13
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
 * Leo lu             2017/06/13      1.0.0              Initial version
 * </pre>
 *
 */
  
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "py/nlr.h"
#include "py/obj.h"
#include "py/binary.h"
#include "py/mpstate.h"
#include "py/runtime.h"
#include "esp_system.h"
#include "esp_log.h"
#include "bt.h"
#include "bta_api.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_heap_caps.h"
#include "codey_utils.h"
#include "codey_ble.h"
#include "codey_ble_prof.h"
#include "codey_ble_srv.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#undef    TAG
#define   TAG           ("SRV")

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/ 

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
bool codey_ble_srv_get_uuid_c(codey_ble_srv_obj_t *srv, esp_bt_uuid_t *uuid_out)
{
  esp_attr_desc_t *attr_desc;

  attr_desc = &(srv->attr_tab[0].att_desc);
  if(!attr_desc->length)
  {
    return false;
  }

  uuid_out->len = attr_desc->length;
  switch(uuid_out->len)
  {
  case ESP_UUID_LEN_16:
    memcpy(&(uuid_out->uuid.uuid16), attr_desc->value, ESP_UUID_LEN_16);
    return true;
  break;

  case ESP_UUID_LEN_32:
    memcpy(&(uuid_out->uuid.uuid32), attr_desc->value, ESP_UUID_LEN_16);
    return true;
  break;

  case ESP_UUID_LEN_128:
    memcpy(&(uuid_out->uuid.uuid128), attr_desc->value, ESP_UUID_LEN_128);
    return true;
  break;

  default:
  return false;
  break;
  }
}

bool codey_ble_srv_start_c(codey_ble_srv_obj_t *srv)
{
  if(srv->is_start)
  {
    return true;
  }

  if(ESP_OK == esp_ble_gatts_start_service(srv->hd_tab[0]))
  {
    srv->is_start = true;
    return true;
  }
  else
  {
    return false;
  }
}

bool codey_ble_srv_stop_c(codey_ble_srv_obj_t *srv)
{
  if(!srv->is_start)
  {
    return true;
  }

  if(ESP_OK == esp_ble_gatts_stop_service(srv->hd_tab[0]))
  {
    srv->is_start = false;
    return true;
  }
  else
  {
    return false;
  }
}

bool codey_ble_srv_find_handle_by_uuid_c(codey_ble_srv_obj_t *srv, esp_bt_uuid_t *uuid, uint16_t *handle_out)
{
  size_t index;
  esp_attr_desc_t *attr_desc;

  for(index = 0; index < srv->attr_num; index++)
  {
    attr_desc = &(srv->attr_tab[index].att_desc);
    if(attr_desc->uuid_length == uuid->len)
    {
      if(0 == memcmp(attr_desc->uuid_p, (uint8_t *)(&(uuid->uuid)), uuid->len))
      {
        *handle_out = srv->hd_tab[index];
        return true;
      }
    }
  }

  return false;
}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
static mp_obj_t codey_ble_srv_start(mp_obj_t srv_in)
{
  codey_ble_srv_obj_t *srv = srv_in;

  if(ESP_OK == esp_ble_gatts_start_service(srv->hd_tab[0]))
  {
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_srv_start_obj, codey_ble_srv_start);

static mp_obj_t codey_ble_srv_stop(mp_obj_t srv_in)
{
  codey_ble_srv_obj_t *srv = srv_in;

  if(ESP_OK == esp_ble_gatts_stop_service(srv->hd_tab[0])) 
  {
    srv->is_start = false;
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_srv_stop_obj, codey_ble_srv_stop);

static mp_obj_t codey_ble_srv_print(mp_obj_t srv_in)
{
  size_t index;
  esp_attr_desc_t *attr_desc;
  codey_ble_srv_obj_t *srv = srv_in;

  if(!srv->attr_num)
  {
    return mp_const_none;
  }

  // ESP_LOGI(TAG,  "\n========== gatts table size %d =========", srv->attr_num);
  for(index = 0; index < srv->attr_num; index++)
  {
    attr_desc = &(srv->attr_tab[index].att_desc);
    codey_ble_print_attr(attr_desc);
  }
 
  return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_srv_print_obj, codey_ble_srv_print);

static mp_obj_t codey_ble_srv_get_uuid(mp_obj_t srv_in)
{
  esp_attr_desc_t *attr_desc;
  codey_ble_srv_obj_t *srv = srv_in;

  if(!srv->attr_num)
  {
    return mp_const_none;
  }

  attr_desc = &(srv->attr_tab[0].att_desc);
  return codey_build_int_list_obj((uint8_t *)(attr_desc->value), attr_desc->length);
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_srv_get_uuid_obj, codey_ble_srv_get_uuid);

static mp_obj_t codey_ble_srv_get_handle_list(mp_obj_t srv_in)
{
  mp_obj_list_t *handle_list;
  codey_ble_srv_obj_t *srv = srv_in;

  if(!srv->is_start)
  {
    return mp_const_none;
  }

  handle_list = mp_obj_new_list(0, NULL);
  if(!handle_list)
  {
    return mp_const_none;
  }
  
  int index;
  for(index = 0; index < srv->attr_num; index++)
  {
    mp_obj_list_append(handle_list, mp_obj_new_int(srv->hd_tab[index]));
  }
 
  return handle_list;
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_srv_get_handle_list_obj, codey_ble_srv_get_handle_list);

static mp_obj_t codey_ble_srv_set_attr_value_by_uuid(mp_obj_t srv_in, mp_obj_t uuid_in, mp_obj_t data_in)
{
  codey_ble_srv_obj_t *srv = srv_in;
  mp_obj_list_t *uuid = uuid_in;
  mp_obj_list_t *data = data_in;
  uint8_t *data_raw; 
  esp_bt_uuid_t uuid_esp;
  uint16_t handle;
  
  if(!uuid->len)
  {
    return mp_const_false;
  }

  uuid_esp.len = uuid->len;
  codey_get_data_from_int_list_obj(uuid, (uint8_t *)(&uuid_esp.uuid));
  if(!codey_ble_srv_find_handle_by_uuid_c(srv, &uuid_esp, &handle))
  {
    ESP_LOGI(TAG, "Can't find handle with the givent uuid");
    return mp_const_false;
  }

  data_raw = pvPortMalloc(data->len);
  if(!data_raw)
  {
    return mp_const_false;
  }
  codey_get_data_from_int_list_obj(data, data_raw);
  
  if(ESP_OK == esp_ble_gatts_set_attr_value(handle, (uint16_t)data->len, data_raw))
  {
    vPortFree(data_raw);
    return mp_const_true;
  }
  else
  {
    vPortFree(data_raw);
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_3(codey_ble_srv_set_attr_value_by_uuid_obj, codey_ble_srv_set_attr_value_by_uuid);

static mp_obj_t codey_ble_srv_set_attr_value(mp_obj_t srv_in, mp_obj_t handle_in, mp_obj_t data_in)
{
  mp_obj_list_t *data = data_in;
  uint8_t *data_raw; 
  uint16_t handle;
  
  handle = mp_obj_get_int(handle_in);
  data_raw = pvPortMalloc(data->len);
  if(!data_raw)
  {
    return mp_const_false;
  }
  codey_get_data_from_int_list_obj(data, data_raw);
  
  if(ESP_OK == esp_ble_gatts_set_attr_value(handle, (uint16_t)data->len, data_raw))
  {
    vPortFree(data_raw);
    return mp_const_true;
  }
  else
  {
    vPortFree(data_raw);
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_3(codey_ble_srv_set_attr_value_obj, codey_ble_srv_set_attr_value);

static mp_obj_t codey_ble_srv_send_notify_by_uuid(mp_obj_t srv_in, mp_obj_t uuid_in, mp_obj_t data_in)
{
  codey_ble_srv_obj_t *srv = srv_in;
  mp_obj_list_t *uuid = uuid_in;
  mp_obj_list_t *data = data_in;
  uint8_t *data_raw; 
  esp_bt_uuid_t uuid_esp;
  uint16_t handle;
  
  if(!uuid->len)
  {
    return mp_const_false;
  }

  uuid_esp.len = uuid->len;
  codey_get_data_from_int_list_obj(uuid, (uint8_t *)(&uuid_esp.uuid));
  if(!codey_ble_srv_find_handle_by_uuid_c(srv, &uuid_esp, &handle))
  {
    ESP_LOGI(TAG, "Can't find handle with the givent uuid");
    return mp_const_false;
  }

  data_raw = pvPortMalloc(data->len);
  if(!data_raw)
  {
    return mp_const_false;
  }
  codey_get_data_from_int_list_obj(data, data_raw);
  
  if(ESP_OK == esp_ble_gatts_send_indicate(srv->prof->interface, srv->prof->conn_id, handle, (uint16_t)data->len, data_raw, false))
  {
    vPortFree(data_raw);
    return mp_const_true;
  }
  else
  {
    vPortFree(data_raw);
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_3(codey_ble_srv_send_notify_by_uuid_obj, codey_ble_srv_send_notify_by_uuid);

static mp_obj_t codey_ble_srv_send_notify(mp_obj_t srv_in, mp_obj_t handle_in, mp_obj_t data_in)
{
  codey_ble_srv_obj_t *srv = srv_in;
  mp_obj_list_t *data = data_in;
  uint8_t *data_raw; 
  uint16_t handle;
  
  handle = mp_obj_get_int(handle_in);
  data_raw = pvPortMalloc(data->len);
  if(!data_raw)
  {
    return mp_const_false;
  }
  codey_get_data_from_int_list_obj(data, data_raw);
  
  if(ESP_OK == esp_ble_gatts_send_indicate(srv->prof->interface, srv->prof->conn_id, handle, (uint16_t)data->len, data_raw, false))
  {
    vPortFree(data_raw);
    return mp_const_true;
  }
  else
  {
    vPortFree(data_raw);
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_3(codey_ble_srv_send_notify_obj, codey_ble_srv_send_notify);

static const mp_map_elem_t codey_ble_srv_locals_dist_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_start),                 (mp_obj_t)&codey_ble_srv_start_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_stop),                  (mp_obj_t)&codey_ble_srv_stop_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_print),                 (mp_obj_t)&codey_ble_srv_print_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_uuid),              (mp_obj_t)&codey_ble_srv_get_uuid_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_handle_list),       (mp_obj_t)&codey_ble_srv_get_handle_list_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_attr_value_by_uuid),(mp_obj_t)&codey_ble_srv_set_attr_value_by_uuid_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_attr_value),        (mp_obj_t)&codey_ble_srv_set_attr_value_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_send_notify_by_uuid),   (mp_obj_t)&codey_ble_srv_send_notify_by_uuid_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_send_notify),           (mp_obj_t)&codey_ble_srv_send_notify_obj },
};
static MP_DEFINE_CONST_DICT(codey_ble_srv_locals_dist, codey_ble_srv_locals_dist_table);

const mp_obj_type_t codey_ble_srv_type = 
{
  { &mp_type_type },
  .name = MP_QSTR_srv,
  .locals_dict = (mp_obj_t)&codey_ble_srv_locals_dist,
};

// END OF FILE
