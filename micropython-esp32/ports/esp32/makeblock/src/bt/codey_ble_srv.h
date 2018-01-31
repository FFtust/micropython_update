/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     Heard for codey_ble_srv.c.
 * @file      codey_ble_srv.h
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
 * Leo lu             2017/06/13         1.0.0              Initial version
 * </pre>
 *
 */

#ifndef _CODEY_BLE_SRV_H_
#define _CODEY_BLE_SRV_H_

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
#include "nvs_flash.h"
#include "bt.h"
#include "bta_api.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_heap_caps.h"
#include "codey_utils.h"
#include "objarray.h"
#include "codey_ble_prof.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/ 
typedef struct codey_ble_srv_obj
{
  mp_obj_base_t                base;
  struct codey_ble_prof_obj    *prof;
  char                         *name;
  bool                         is_start;
  size_t                       attr_num;
  uint16_t                     *hd_tab;
  esp_gatts_attr_db_t          *attr_tab;
  struct codey_ble_srv_obj     *next_srv;
} codey_ble_srv_obj_t;

/******************************************************************************
 DECLARE PUBLIC FUNCTIONS
 ******************************************************************************/
extern const mp_obj_type_t codey_ble_srv_type;

extern bool codey_ble_srv_get_uuid_c(codey_ble_srv_obj_t *srv, esp_bt_uuid_t *uuid_out);
extern bool codey_ble_srv_start_c(codey_ble_srv_obj_t *srv);
extern bool codey_ble_srv_stop_c(codey_ble_srv_obj_t *srv);

#endif /* _CODEY_BLE_SRV_H_ */

