/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     
 * @file      codey_ble_prof.c
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
#include "codey_ble_srv.h"
#include "codey_ble_prof.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/ 

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
static void codey_ble_prof_add_srv(codey_ble_prof_obj_t *prof, codey_ble_srv_obj_t *srv)
{
  codey_ble_srv_obj_t *srv_node;

  srv_node = prof->srv_list;
  if(!srv_node)
  {
    prof->srv_list = srv;
  }
  else
  {
    while(srv_node->next_srv) srv_node = srv_node->next_srv;
    srv_node->next_srv = srv;
  }
  srv->prof = prof;
}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
struct codey_ble_srv_obj *codey_ble_prof_find_srv_by_uuid_c(codey_ble_prof_obj_t *prof, esp_bt_uuid_t *uuid)
{
  codey_ble_srv_obj_t *srv_node;
  esp_bt_uuid_t uuid_buf;

  srv_node = prof->srv_list;
  while(srv_node)
  {
    if(codey_ble_srv_get_uuid_c(srv_node, &uuid_buf))
    {
      if(codey_ble_compare_uuid(&uuid_buf, uuid))
      {
        return srv_node;
      }
    }

    srv_node = srv_node->next_srv;
  }
  
  return NULL;
}

static mp_obj_t codey_ble_prof_get_name(mp_obj_t prof_in)
{
  codey_ble_prof_obj_t *prof = prof_in;
  return mp_obj_new_str(prof->name, strlen(prof->name), false);
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_prof_get_name_obj, codey_ble_prof_get_name);

static mp_obj_t codey_ble_prof_new_srv(mp_obj_t prof_in, mp_obj_t attr_tab_in)
{
  codey_ble_srv_obj_t *new_srv;
  codey_ble_prof_obj_t *prof = prof_in;
  mp_obj_list_t *attr_tab = attr_tab_in;
  mp_obj_list_t *attr;
  esp_attr_desc_t *attr_desc;
  esp_bt_uuid_t uuid;
  
  if(codey_ble_prof_find_srv_by_uuid_c(prof, &uuid))
  {
    return mp_const_none;
  }
  
  codey_ble_create_srv_uuid_from_mp_attr(attr_tab->items[0], &uuid);
  new_srv = (codey_ble_srv_obj_t *)pvPortMalloc(sizeof(codey_ble_srv_obj_t));
  memset(new_srv, 0, sizeof(codey_ble_srv_obj_t));
  new_srv->base.type = &codey_ble_srv_type;
  new_srv->attr_tab = pvPortMalloc(sizeof(esp_gatts_attr_db_t) * attr_tab->len);
  memset(new_srv->attr_tab, 0, sizeof(esp_gatts_attr_db_t) * attr_tab->len);
  new_srv->attr_num = attr_tab->len;
  new_srv->hd_tab = pvPortMalloc(sizeof(uint16_t)*attr_tab->len);
  memset(new_srv->hd_tab, 0, sizeof(uint16_t) * attr_tab->len);
  new_srv->next_srv = NULL;
  
  /*
  attr_tab is a list, each entry is also a list, entry's list define as follow:
  ((UUID_LEN)int, (UUID_VALUE)list, (PERMIT)int, (MAX_LEN)int, (CHAR_CURRENT_LEN)int, (CHAR_VALUE)list)
  */
  uint32_t index;
  for(index = 0; index < new_srv->attr_num; index++)
  {
    (new_srv->attr_tab)[index].attr_control.auto_rsp = ESP_GATT_AUTO_RSP;
    attr_desc = &((new_srv->attr_tab)[index].att_desc);
    attr = attr_tab->items[index];

    attr_desc->uuid_length = mp_obj_get_int(attr->items[0]);    
    attr_desc->uuid_p = pvPortMalloc(((mp_obj_list_t *)(attr->items[1]))->len);
    codey_get_data_from_int_list_obj((mp_obj_list_t *)(attr->items[1]), (attr_desc->uuid_p));
    attr_desc->perm = mp_obj_get_int(attr->items[2]);
    attr_desc->max_length = mp_obj_get_int(attr->items[3]);
    attr_desc->length = mp_obj_get_int(attr->items[4]);
    if(attr_desc->length)
    {
      attr_desc->value = pvPortMalloc(((mp_obj_list_t *)(attr->items[5]))->len);
      codey_get_data_from_int_list_obj((mp_obj_list_t *)(attr->items[5]), (attr_desc->value));  
    }
    else
    {
      attr_desc->value = NULL;
    }
  }

  codey_ble_prof_add_srv(prof, new_srv);
  if(ESP_OK == esp_ble_gatts_create_attr_tab(new_srv->attr_tab, prof->interface, new_srv->attr_num, 0))
  {
    return new_srv;
  }
  else
  {
    // TODO
    // codey_ble_srv_free(new_srv);
    return mp_const_none;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_2(codey_ble_prof_new_srv_obj, codey_ble_prof_new_srv);

static mp_obj_t codey_ble_prof_find_srv_by_uuid(mp_obj_t prof_in, mp_obj_t uuid_in)
{
  codey_ble_prof_obj_t *prof = prof_in;
  codey_ble_srv_obj_t *find_srv;
  mp_obj_list_t *uuid = uuid_in;
  esp_bt_uuid_t find_uuid;

  find_uuid.len = uuid->len;
  codey_get_data_from_int_list_obj((mp_obj_list_t *)uuid, (uint8_t *)(&(find_uuid.uuid)));

  find_srv = codey_ble_prof_find_srv_by_uuid_c(prof, &find_uuid);
  if(find_srv)
  {
    return find_srv;
  }
  else
  {
    return mp_const_none;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_2(codey_ble_prof_find_srv_by_uuid_obj, codey_ble_prof_find_srv_by_uuid);

static mp_obj_t codey_ble_prof_reg_gatts_cb(mp_obj_t prof_in, mp_obj_t gatts_cb_in)
{
  codey_ble_prof_obj_t *prof = prof_in;

  if(mp_obj_is_callable(gatts_cb_in))
  {
    prof->gatts_cb = gatts_cb_in;
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_2(codey_ble_prof_reg_gatts_cb_obj, codey_ble_prof_reg_gatts_cb);

static mp_obj_t codey_ble_prof_start(mp_obj_t prof_in)
{
  codey_ble_prof_obj_t *prof = prof_in; 
  codey_ble_srv_obj_t *srv_node;

  srv_node = prof->srv_list;
  while(srv_node)
  {
    codey_ble_srv_start_c(srv_node);
    srv_node = srv_node->next_srv;
  }
  
  return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_prof_start_obj, codey_ble_prof_start);

static mp_obj_t codey_ble_prof_stop(mp_obj_t prof_in)
{
  codey_ble_prof_obj_t *prof = prof_in; 
  codey_ble_srv_obj_t *srv_node;

  srv_node = prof->srv_list;
  while(srv_node)
  {
    codey_ble_srv_stop_c(srv_node);
    srv_node = srv_node->next_srv;
  }
  
  return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_prof_stop_obj, codey_ble_prof_stop);

static const mp_map_elem_t codey_ble_prof_locals_dist_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_name),                    (mp_obj_t)&codey_ble_prof_get_name_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_new_srv),                 (mp_obj_t)&codey_ble_prof_new_srv_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_find_srv),                (mp_obj_t)&codey_ble_prof_find_srv_by_uuid_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_reg_gatts_cb),            (mp_obj_t)&codey_ble_prof_reg_gatts_cb_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_start),                   (mp_obj_t)&codey_ble_prof_start_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_stop),                    (mp_obj_t)&codey_ble_prof_stop_obj },
};
static MP_DEFINE_CONST_DICT(codey_ble_prof_locals_dist, codey_ble_prof_locals_dist_table);

const mp_obj_type_t codey_ble_prof_type = 
{
  { &mp_type_type },
  .name = MP_QSTR_prof,
  .locals_dict = (mp_obj_t)&codey_ble_prof_locals_dist,
};

// END OF FILE
