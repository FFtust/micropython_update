/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   The python module for makeblock docking station.
 * @file    modmakeblock.h
 * @author  Mark Yan
 * @version V1.0.0
 * @date    2017/03/03
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
 * The python module for makeblock docking station.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * Mark Yan         2017/03/02     1.0.0            build the new.
 * </pre>
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/mphal.h"
#include "esp_system.h"
#include "py/mpconfig.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/objlist.h"
#include "py/stream.h"
#include "py/mphal.h"
#include "uart.h"

#include "codey_h_class.h"
/// \module makeblock - functions related to the makeblock
///

/******************************************************************************/
// Micro Python bindings;
STATIC mp_obj_t makeblock_reset(void) 
{
  esp_restart();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(makeblock_reset_obj, makeblock_reset);

STATIC mp_obj_t makeblock_setup(void) 
{
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(makeblock_setup_obj, makeblock_setup);


STATIC mp_obj_t codey_get_timer_100ms(void) 
{
  float t = 0 ;
  t = codey_get_timer_value_t();
  return mp_obj_new_float(t);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(codey_get_timer_100ms_obj, codey_get_timer_100ms);

STATIC mp_obj_t codey_reset_timer(void) 
{
  codey_reset_timer_t();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(codey_reset_timer_obj, codey_reset_timer);

STATIC const mp_map_elem_t makeblock_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_reset),                     (mp_obj_t)(&makeblock_reset_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_setup),                     (mp_obj_t)(&makeblock_setup_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_timer_t),                   (mp_obj_t)(&codey_get_timer_100ms_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_reset_timer_t),             (mp_obj_t)(&codey_reset_timer_obj) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_gyro_board),                (mp_obj_t)&codey_gyro_board_type }, 
    { MP_OBJ_NEW_QSTR(MP_QSTR_music),                     (mp_obj_t)&codey_8bit_voice_board_type }, 
    { MP_OBJ_NEW_QSTR(MP_QSTR_button_board),              (mp_obj_t)&codey_button_board_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_light_sensor_board),        (mp_obj_t)&codey_light_sensor_board_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_potentionmeter_board),      (mp_obj_t)&codey_potentionmeter_board_type },   
    { MP_OBJ_NEW_QSTR(MP_QSTR_sound_sensor_board),        (mp_obj_t)&codey_sound_sensor_board_type }, 
    { MP_OBJ_NEW_QSTR(MP_QSTR_rmt_board),                 (mp_obj_t)&codey_rmt_board_type }, 
    { MP_OBJ_NEW_QSTR(MP_QSTR_wlan),                      (mp_obj_t)&codey_wlan_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_hardware_version),          (mp_obj_t)&codey_hardware_version_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_rgbled_board),              (mp_obj_t)&codey_rgbled_board_type }, 
    { MP_OBJ_NEW_QSTR(MP_QSTR_battery_check),             (mp_obj_t)&codey_battery_check_type },  
    { MP_OBJ_NEW_QSTR(MP_QSTR_message_board),             (mp_obj_t)&codey_message_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_codey_eve),                 (mp_obj_t)&codey_event_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_neurons_engine),            (mp_obj_t)&neurons_engine_port_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_super_var),                 (mp_obj_t)&codey_super_var_type},
    { MP_OBJ_NEW_QSTR(MP_QSTR_ledmatrix),                 (mp_obj_t)&codey_ledmatrix_type},    
};
STATIC MP_DEFINE_CONST_DICT(makeblock_module_globals, makeblock_module_globals_table);

const mp_obj_module_t makeblock_module = 
{
  .base = { &mp_type_module },
  .globals = (mp_obj_dict_t*)&makeblock_module_globals,
};
