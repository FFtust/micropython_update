/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     Heard for codey_ble_client.c
 * @file      codey_ble_client.h
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
 * Leo lu             2017/06/18         1.0.0              Initial version
 * </pre>
 *
 */

#ifndef _CODEY_BLE_CLIENT_H_
#define _CODEY_BLE_CLIENT_H_

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

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES
******************************************************************************/
typedef struct codey_ble_find_srv
{
  esp_gatt_srvc_id_t      srv;
  struct codey_ble_find_srv *next_srv;
} codey_ble_find_srv_t;

typedef struct codey_ble_find_char_elem
{
  esp_gatt_id_t                 char_data;
  struct codey_ble_find_char_elem *next_char;
} codey_ble_find_char_elem_t;

typedef struct codey_ble_find_char
{
  esp_gatt_srvc_id_t        srv;
  codey_ble_find_char_elem_t  *char_list;
} codey_ble_find_char_t;

typedef struct codey_ble_client_obj
{
  mp_obj_base_t             base;
  struct codey_ble_obj      *ble;
  char                      *name;
  mp_obj_t                  gattc_cb;
  uint16_t                  app_id;
  bool                      is_conn;
  uint16_t                  conn_id;
  esp_bd_addr_t             remote_bda;
  esp_gatt_if_t             interface;

  bool                      is_search_all_srv;
  codey_ble_find_srv_t      *find_srv_list;
  
  bool                      is_search_char;
  codey_ble_find_char_t     find_char_list;

  struct codey_ble_client_obj *next_client;
} codey_ble_client_obj_t;

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE DATAS
 ******************************************************************************/

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DECLARE PUBLIC FUNCTIONS
 ******************************************************************************/
extern const mp_obj_type_t codey_ble_client_type;

extern void codey_ble_client_print_c(codey_ble_client_obj_t *client_in);
extern void codey_ble_add_find_srv(codey_ble_client_obj_t *client_in, codey_ble_find_srv_t *srv_in);
extern bool codey_ble_find_char_by_uuid(codey_ble_client_obj_t *client_in, esp_gatt_id_t *char_data);
extern void codey_ble_add_find_char(codey_ble_client_obj_t *client_in, esp_gatt_id_t *char_data);

#endif /* _CODEY_BLE_CLIENT_H_ */

