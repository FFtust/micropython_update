/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     
 * @file      codey_ble_client.c
 * @author    Leo lu
 * @version   V1.0.0
 * @date      2017/06/18
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
 * Leo lu             2017/06/18      1.0.0              Initial version
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
#include "esp_gattc_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_heap_caps.h"
#include "codey_utils.h"
#include "codey_ble.h"
#include "codey_ble_client.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#undef    TAG
#define   TAG           ("CLIENT")

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
void codey_ble_srv_id_mp_to_c(mp_obj_t srv_mp_in, esp_gatt_srvc_id_t *srv_c_in)
{
  mp_obj_list_t *srv_mp = srv_mp_in;
  mp_obj_list_t *uuid_mp = (mp_obj_list_t *)(srv_mp->items[2]);
  
  srv_c_in->is_primary = mp_obj_get_int(srv_mp->items[0]);
  srv_c_in->id.inst_id = mp_obj_get_int(srv_mp->items[1]);
  srv_c_in->id.uuid.len = uuid_mp->len;
  codey_get_data_from_int_list_obj(uuid_mp, (uint8_t *)(&(srv_c_in->id.uuid.uuid)));
}

void codey_ble_char_id_mp_to_c(mp_obj_t char_mp_in, esp_gatt_id_t *char_c_in)
{
  mp_obj_list_t *char_mp = char_mp_in;
  mp_obj_list_t *uuid_mp = (mp_obj_list_t *)(char_mp->items[1]);
  
  char_c_in->inst_id = mp_obj_get_int(char_mp->items[0]);
  char_c_in->uuid.len = uuid_mp->len;
  codey_get_data_from_int_list_obj(uuid_mp, (uint8_t *)(&(char_c_in->uuid.uuid)));
}
 
void codey_ble_client_print_c(codey_ble_client_obj_t *client_in)
{
  codey_ble_client_obj_t *client = client_in;
  // ESP_LOGI(TAG,  "client name: %s\n", client->name);
}

void codey_ble_free_find_srv_list(codey_ble_find_srv_t *srv_list)
{
  codey_ble_find_srv_t *srv_node;
  codey_ble_find_srv_t *next_srv;

  srv_node = srv_list;
  while(srv_node)
  {
    next_srv = srv_node->next_srv;
    vPortFree(srv_node);
    srv_node = next_srv;
  }
}

void codey_ble_add_find_srv(codey_ble_client_obj_t *client_in, codey_ble_find_srv_t *srv_in)
{
  codey_ble_client_obj_t *client = client_in;
  codey_ble_find_srv_t *srv_node;

  srv_in->next_srv = NULL;
  if(!client->find_srv_list)
  {
    client->find_srv_list = srv_in;
  }
  else
  {
    srv_node = client->find_srv_list;
    while(srv_node->next_srv) srv_node = srv_node->next_srv;
    srv_node->next_srv = srv_in;
  }
}

void codey_ble_free_char_elem_list(codey_ble_find_char_elem_t *char_list)
{
  codey_ble_find_char_elem_t *char_data, *next_char;

  char_data = char_list;
  while(char_data)
  {
    next_char = char_data->next_char;
    vPortFree(char_data);
    char_data = next_char;
  }
}

bool codey_ble_find_char_by_uuid(codey_ble_client_obj_t *client_in, esp_gatt_id_t *char_data)
{
  codey_ble_client_obj_t *client = client_in;
  codey_ble_find_char_elem_t *char_elem;

  if(!client->is_search_char)
  {
    return false;
  }

  char_elem = client->find_char_list.char_list;
  while(char_elem)
  {
    if(0 == memcmp(&(char_elem->char_data.uuid), &(char_data->uuid), sizeof(esp_bt_uuid_t)))
    {
      return true;
    }
    else
    {
      char_elem = char_elem->next_char;
    }
  }

  return false;
}

void codey_ble_add_find_char(codey_ble_client_obj_t *client_in, esp_gatt_id_t *char_data)
{
  codey_ble_client_obj_t *client = client_in;
  codey_ble_find_char_elem_t *char_elem, *new_char_elem;
  
  new_char_elem = pvPortMalloc(sizeof(codey_ble_find_char_elem_t));
  memcpy(&(new_char_elem->char_data), char_data, sizeof(esp_gatt_id_t));
  new_char_elem->next_char = NULL;

  if(!client->find_char_list.char_list)
  {
    client->find_char_list.char_list = new_char_elem;
  }
  else
  {
    char_elem = client->find_char_list.char_list;
    while(char_elem->next_char) char_elem = char_elem->next_char;
    char_elem->next_char = new_char_elem;
  }
}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
static mp_obj_t codey_ble_client_conn(mp_obj_t client_in, mp_obj_t bd_addr_in)
{
  codey_ble_client_obj_t *client = client_in;
  esp_bd_addr_t remote_bda;

  if(!codey_get_data_from_int_list_obj((mp_obj_list_t *)bd_addr_in, (uint8_t *)remote_bda))
  {
    return mp_const_false;
  }
  // codey_print_hex(remote_bda, ESP_BD_ADDR_LEN);
  // ESP_LOGI(TAG,  "\n");
  
  if(ESP_OK == esp_ble_gattc_open(client->interface, remote_bda, true))
  {
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_2(codey_ble_client_conn_obj, codey_ble_client_conn);

static mp_obj_t codey_ble_client_close(mp_obj_t client_in)
{
  codey_ble_client_obj_t *client = client_in;

  if(!client->is_conn)
  {
    return mp_const_false;
  }
  
  if(ESP_OK == esp_ble_gattc_close(client->interface, client->conn_id))
  {
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_client_close_obj, codey_ble_client_close);

static mp_obj_t codey_ble_client_print(mp_obj_t client_in)
{
  codey_ble_client_obj_t *client = client_in;
  ESP_LOGI(TAG,  "client name: %s\n", client->name);
  return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_client_print_obj, codey_ble_client_print);

static mp_obj_t codey_ble_client_reg_cb(mp_obj_t client_in, mp_obj_t app_cb_in)
{ 
  codey_ble_client_obj_t *client = client_in;

  if(mp_obj_is_callable(app_cb_in))
  {
    client->gattc_cb = app_cb_in;
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_2(codey_ble_client_reg_cb_obj, codey_ble_client_reg_cb);

static mp_obj_t codey_ble_client_search_srv(mp_obj_t client_in, mp_obj_t srv_uuid_in)
{ 
  codey_ble_client_obj_t *client = client_in;
  mp_obj_list_t *srv_uuid = srv_uuid_in;
  esp_bt_uuid_t srv_uuid_c;

  codey_get_data_from_int_list_obj(srv_uuid, (uint8_t *)(&(srv_uuid_c.uuid)));
  srv_uuid_c.len = srv_uuid->len;
  esp_ble_gattc_search_service(client->interface, client->conn_id, &srv_uuid_c);
  return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(codey_ble_client_search_srv_obj, codey_ble_client_search_srv);

static mp_obj_t codey_ble_client_search_all_srv(mp_obj_t client_in)
{ 
  codey_ble_client_obj_t *client = client_in;

  if(client->find_srv_list)
  {
    codey_ble_free_find_srv_list(client->find_srv_list);
    client->find_srv_list = NULL;
  }

  client->is_search_all_srv = true;
  if(ESP_OK == esp_ble_gattc_search_service(client->interface, client->conn_id, NULL))
  {
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_client_search_all_srv_obj, codey_ble_client_search_all_srv);

static mp_obj_t codey_ble_client_search_all_char(mp_obj_t client_in, mp_obj_t srv_in)
{ 
  codey_ble_client_obj_t *client = client_in;
  mp_obj_list_t *mp_search_srv = srv_in;

  if(client->find_char_list.char_list)
  {
    ESP_LOGI(TAG, "Free last char list");
    codey_ble_free_char_elem_list(client->find_char_list.char_list);
  }

  client->is_search_char = true;
  client->find_char_list.srv.id.uuid.len = mp_search_srv->len;
  codey_get_data_from_int_list_obj(mp_search_srv, (uint8_t *)(&(client->find_char_list.srv.id.uuid.uuid)));
  client->find_char_list.srv.is_primary = true;
  client->find_char_list.srv.id.inst_id = 0;
  client->find_char_list.char_list = NULL;
  
  if(ESP_OK == esp_ble_gattc_get_characteristic(client->interface, client->conn_id, &(client->find_char_list.srv), NULL))
  {
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_2(codey_ble_client_search_all_char_obj, codey_ble_client_search_all_char);

static mp_obj_t codey_ble_client_get_char_desc(mp_obj_t client_in, mp_obj_t srv_id_in, mp_obj_t char_id_in)
{
  codey_ble_client_obj_t *client = client_in;
  esp_gatt_srvc_id_t srv_id;
  esp_gatt_id_t char_id;
  
  if(!client->is_conn)
  {
    return mp_const_false;
  }

  codey_ble_srv_id_mp_to_c(srv_id_in, &srv_id);
  codey_ble_char_id_mp_to_c(char_id_in, &char_id);

  if(ESP_OK == esp_ble_gattc_get_descriptor(client->interface, client->conn_id, &srv_id, &char_id, NULL))
  {
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }

  return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_3(codey_ble_client_get_char_desc_obj, codey_ble_client_get_char_desc);

static mp_obj_t codey_ble_client_read_char(mp_obj_t client_in, mp_obj_t srv_id_in, mp_obj_t char_id_in)
{
  codey_ble_client_obj_t *client = client_in;
  esp_gatt_srvc_id_t srv_id;
  esp_gatt_id_t char_id;
  
  if(!client->is_conn)
  {
    return mp_const_false;
  }

  codey_ble_srv_id_mp_to_c(srv_id_in, &srv_id);
  codey_ble_char_id_mp_to_c(char_id_in, &char_id);

  if(ESP_OK == esp_ble_gattc_read_char(client->interface, client->conn_id, &srv_id, &char_id, ESP_GATT_AUTH_REQ_NONE))
  {
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }

  return mp_const_true;  
}
static MP_DEFINE_CONST_FUN_OBJ_3(codey_ble_client_read_char_obj, codey_ble_client_read_char);

static mp_obj_t codey_ble_client_rsp_write_char(mp_uint_t n_args, const mp_obj_t *args)
{
  if(4 != n_args)
  {
    return mp_const_false;
  }

  codey_ble_client_obj_t *client = args[0];
  mp_obj_list_t *srv_id_mp = args[1];
  mp_obj_list_t *char_id_mp = args[2];
  mp_obj_list_t *value = args[3];
  esp_gatt_srvc_id_t srv_id;
  esp_gatt_id_t char_id;
  uint8_t *buf;

  if(!client->is_conn)
  {
    return mp_const_false;
  }

  buf = pvPortMalloc(value->len);
  if(!buf)
  {
    return mp_const_false;
  }
  codey_get_data_from_int_list_obj(value, buf);

  codey_ble_srv_id_mp_to_c(srv_id_mp, &srv_id);
  codey_ble_char_id_mp_to_c(char_id_mp, &char_id);
  if(ESP_OK == esp_ble_gattc_write_char( client->interface, 
                                         client->conn_id, 
                                         &srv_id, 
                                         &char_id, 
                                         value->len, 
                                         buf, 
                                         ESP_GATT_WRITE_TYPE_RSP, 
                                         ESP_GATT_AUTH_REQ_NONE  
                                         ) 
   )
  {
    vPortFree(buf);
    return mp_const_true;
  }
  else
  {
    vPortFree(buf);
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(codey_ble_client_rsp_write_char_obj, 4, 4, codey_ble_client_rsp_write_char);

static mp_obj_t codey_ble_client_reg_notify(mp_obj_t client_in, mp_obj_t srv_id_in, mp_obj_t char_id_in)
{
  codey_ble_client_obj_t *client = client_in;
  esp_gatt_srvc_id_t srv_id;
  esp_gatt_id_t char_id;

  if(!client->is_conn)
  {
    return mp_const_false;
  }

  codey_ble_srv_id_mp_to_c(srv_id_in, &srv_id);
  codey_ble_char_id_mp_to_c(char_id_in, &char_id);

  if(ESP_OK == esp_ble_gattc_register_for_notify(client->interface, client->remote_bda, &srv_id, &char_id))
  {
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_3(codey_ble_client_reg_notify_obj, codey_ble_client_reg_notify);

static const mp_map_elem_t codey_ble_client_locals_dist_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_conn),                  (mp_obj_t)&codey_ble_client_conn_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_close),                 (mp_obj_t)&codey_ble_client_close_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_print),                 (mp_obj_t)&codey_ble_client_print_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_reg_gattc_cb),          (mp_obj_t)&codey_ble_client_reg_cb_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_search_srv),            (mp_obj_t)&codey_ble_client_search_srv_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_search_all_srv),        (mp_obj_t)&codey_ble_client_search_all_srv_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_search_all_char),       (mp_obj_t)&codey_ble_client_search_all_char_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_char_desc),         (mp_obj_t)&codey_ble_client_get_char_desc_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_read_char),             (mp_obj_t)&codey_ble_client_read_char_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_rsp_write_char),        (mp_obj_t)&codey_ble_client_rsp_write_char_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_reg_notify),            (mp_obj_t)&codey_ble_client_reg_notify_obj },
};

static MP_DEFINE_CONST_DICT(codey_ble_client_locals_dist, codey_ble_client_locals_dist_table);
const mp_obj_type_t codey_ble_client_type = 
{
  { &mp_type_type },
  .name = MP_QSTR_client,
  .locals_dict = (mp_obj_t)&codey_ble_client_locals_dist,
};


// END OF FILE
