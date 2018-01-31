/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Utility for common use
 * @file    codey_utils.h
 * @author  Leo lu
 * @version V1.0.0
 * @date    2017/06/09
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
 * This file is a header for ring function.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *   Leo lu          2017/06/09       1.0.0              build the new.
 * </pre>
 *
 */
  
#ifndef _CODEY_UTILS_H_
#define _CODEY_UTILS_H_
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "py/nlr.h"
#include "py/obj.h"
#include "py/binary.h"
#include "py/mpstate.h"
#include "py/runtime.h"
//#include "objarray.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_heap_caps.h"

/*****************************************************************
 DEFINE PUBLIC FUNCTION
 ******************************************************************/
extern void codey_print_hex(uint8_t *data, uint32_t len);
extern void codey_to_hex_str(uint8_t *in_data, uint16_t len, uint8_t *out_data);
extern bool codey_get_data_from_int_list_obj(mp_obj_list_t *mp_list, uint8_t *pbuf);
extern mp_obj_t codey_build_int_list_obj(uint8_t *data, uint32_t len);
extern void codey_ble_print_attr(esp_attr_desc_t* attr);
extern bool codey_ble_compare_attr(esp_attr_desc_t *a, esp_attr_desc_t *b);
extern bool codey_ble_compare_uuid(esp_bt_uuid_t *a, esp_bt_uuid_t *b);
extern void codey_ble_create_srv_uuid_from_mp_attr(mp_obj_list_t *attr_in, esp_bt_uuid_t *uuid_out);
extern void codey_ble_print_uuid(esp_bt_uuid_t *uuid);
extern uint8_t *codey_get_path_from_full_file_name(uint8_t *full_file_name);
extern uint8_t *codey_get_name_from_full_file_name(uint8_t *full_file_name);
extern uint8_t codey_read_buffer(uint8_t *data_buffer, int32_t index);
extern int16_t codey_read_short(uint8_t *data_buffer, int32_t idx);
extern long codey_read_long(uint8_t *data_buffer, int32_t idx);
extern float codey_read_float(uint8_t *data_buffer, int32_t idx);
extern int lcm(int a, int b);
extern uint8_t codey_calc_add_check_sum(uint8_t *data, uint32_t len);

#endif /* _CODEY_UTILS_H_ */
