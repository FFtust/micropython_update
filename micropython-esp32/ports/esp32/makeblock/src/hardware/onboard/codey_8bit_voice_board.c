/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for speaker module
 * @file    codey_8bit_voice_board.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/05/23
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
 * This file is a drive button_board module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/05/22      1.0.0              build the new.
 * </pre>
 *
 */
    
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "py/mpstate.h"
#include "py/runtime.h"
#include "esp_log.h"

#include "py/nlr.h"
#include "py/obj.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/gpio.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/dac.h"
#include "py/mphal.h"

#include "codey_sys.h"
#include "codey_esp32_resouce_manager.h"

#include "codey_8bit_voice_board.h"
#include "codey_voice.h"

#include "esp_system.h"

#include "esp_heap_caps.h"
#include "codey_utils.h"

#include "soc/rtc_io_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "rtc_io.h"
#include "timer.h"

#include "extmod/vfs_fat.h"
#include "codey_sys_operation.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define   TAG                         ("music_func")
#define   MUSIC_VOLUME_MAX_FOR_MUSIC  (100) // add by fftust
#define   MUSIC_VOLUME_MAX_FOR_NOTE   (30) // add by fftust
#define   MUSIC_FILE_PATH             ("/music/")
#define   MUSIC_FILE_NAME_LEN_MAX     (32)
 
/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
typedef struct
{
  mp_obj_base_t base;
}codey_8bit_voice_board_obj_t;

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
static codey_8bit_voice_board_obj_t s_codey_8bit_voice_board_obj = {.base = {&codey_8bit_voice_board_type}};

/******************************************************************************
 DEFINE PUBLIC DATAS
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
 
/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_8bit_voice_board_config_t(void)  
{
  dac_output_enable(DAC_CHANNEL_1);
  dac_output_voltage(DAC_CHANNEL_1, 0);
}

void codey_8bit_voice_board_play_note_directly_t(uint32_t freq, uint32_t time_ms, uint8_t vol)
{
  uint32_t t_us = 0;
  t_us = 1000000 / freq;
  uint32_t num = (time_ms * 1000) / t_us;
  for(uint32_t i = 0; i < num / 2; i++)
  {
    dac_output_voltage(DAC_CHANNEL_1, vol);
    ets_delay_us(t_us);
    dac_output_voltage(DAC_CHANNEL_1, 0);
    ets_delay_us(t_us);
  }
}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
STATIC mp_obj_t codey_8bit_voice_board_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
  // setup the object
  codey_8bit_voice_board_obj_t *self = &s_codey_8bit_voice_board_obj;
  self->base.type = &codey_8bit_voice_board_type;
  return self;
}

STATIC mp_obj_t codey_8bit_voice_board_init(mp_obj_t self_in)
{
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_8bit_voice_board_init_obj, codey_8bit_voice_board_init);

STATIC mp_obj_t codey_8bit_voice_board_deinit(mp_obj_t self_in)
{ 
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_8bit_voice_board_deinit_obj, codey_8bit_voice_board_deinit);

STATIC mp_obj_t codey_8bit_voice_board_set_volume(mp_obj_t self_in, mp_obj_t arg1)
{
  if(!codey_task_get_enable_flag_t(CODEY_MUSIC_PLAY_TASK_ID))
  {
    return mp_const_none;
  }

  float vol = mp_obj_get_float(arg1);
  codey_voice_set_volume_t(vol);
  
  return mp_const_true; 
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_8bit_voice_board_set_volume_obj, codey_8bit_voice_board_set_volume);

STATIC mp_obj_t codey_8bit_voice_board_get_volume(mp_obj_t self_in)
{
  float vol = codey_voice_get_volume_t();
  
  return mp_obj_new_float(vol);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_8bit_voice_board_get_volume_obj, codey_8bit_voice_board_get_volume);

STATIC mp_obj_t codey_8bit_voice_board_change_volume(mp_obj_t self_in, mp_obj_t arg1)
{
  if(!codey_task_get_enable_flag_t(CODEY_MUSIC_PLAY_TASK_ID))
  {
    return mp_const_none;
  }
  float vol = mp_obj_get_float(arg1);
  codey_voice_change_volume_t(vol);

  return mp_const_true; 
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_8bit_voice_board_change_volume_obj, codey_8bit_voice_board_change_volume);

STATIC mp_obj_t codey_8bit_voice_board_play(mp_obj_t self_in, mp_obj_t arg1)
{
  if(!codey_task_get_enable_flag_t(CODEY_MUSIC_PLAY_TASK_ID))
  {
    return mp_const_none;
  }
  size_t len;
  const char *file_name = mp_obj_str_get_data(arg1, &len);
  char file_whole_name[MUSIC_FILE_NAME_LEN_MAX] = MUSIC_FILE_PATH; 
  if(len > MUSIC_FILE_NAME_LEN_MAX - strlen(MUSIC_FILE_PATH)) // 7 is the lenth of MUSIC_FILE_PATH
  {
    return mp_const_false;
  }
  else
  {
    strcat((char *)file_whole_name, file_name);
    MP_THREAD_GIL_EXIT();
    codey_voice_stop_play();
    codey_voice_play_file(file_whole_name);
    MP_THREAD_GIL_ENTER();
  } 
  
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_8bit_voice_board_play_obj, codey_8bit_voice_board_play);

STATIC mp_obj_t codey_8bit_voice_board_play_to_stop(mp_obj_t self_in, mp_obj_t arg1)
{
  if(!codey_task_get_enable_flag_t(CODEY_MUSIC_PLAY_TASK_ID))
  {
    return mp_const_none;
  }
  size_t len;
  const char *file_name = mp_obj_str_get_data(arg1, &len);
  char file_whole_name[MUSIC_FILE_NAME_LEN_MAX] = MUSIC_FILE_PATH; 
  if(len > MUSIC_FILE_NAME_LEN_MAX - strlen(MUSIC_FILE_PATH)) // 7 is the lenth of MUSIC_FILE_PATH
  {
    return mp_const_false;
  }
  else
  {
    strcat((char *)file_whole_name, file_name);
    MP_THREAD_GIL_EXIT();
    codey_voice_stop_play();
    codey_voice_play_file_to_stop(file_whole_name);
    MP_THREAD_GIL_ENTER();
  } 


  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_8bit_voice_board_play_to_stop_obj, codey_8bit_voice_board_play_to_stop);

STATIC mp_obj_t codey_8bit_voice_board_play_note(mp_obj_t self_in, mp_obj_t arg1, mp_obj_t arg2)
{
  if(!codey_task_get_enable_flag_t(CODEY_MUSIC_PLAY_TASK_ID))
  {
    return mp_const_none;
  }

  uint32_t frequency = mp_obj_get_int(arg1);
  uint32_t time_ms = mp_obj_get_int(arg2);

  MP_THREAD_GIL_EXIT();
  codey_voice_stop_play();
  codey_voice_play_note(frequency, time_ms, BUZZER, MUSIC_VOLUME_MAX_FOR_NOTE);
  MP_THREAD_GIL_ENTER();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(codey_8bit_voice_board_play_note_obj, codey_8bit_voice_board_play_note);

STATIC mp_obj_t codey_8bit_voice_board_play_note_to_stop(mp_obj_t self_in, mp_obj_t arg1, mp_obj_t arg2)
{ 
  if(!codey_task_get_enable_flag_t(CODEY_MUSIC_PLAY_TASK_ID))
  {
    return mp_const_none;
  }

  uint32_t frequency = mp_obj_get_int(arg1);
  uint32_t time_ms = mp_obj_get_int(arg2);
  MP_THREAD_GIL_EXIT();
  codey_voice_stop_play();
  codey_voice_play_note_to_stop(frequency, time_ms, BUZZER, MUSIC_VOLUME_MAX_FOR_NOTE);
  MP_THREAD_GIL_ENTER();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(codey_8bit_voice_board_play_note_to_stop_obj, codey_8bit_voice_board_play_note_to_stop);

STATIC mp_obj_t codey_8bit_voice_board_stop_all(mp_obj_t self_in)
{
  MP_THREAD_GIL_EXIT();
  codey_voice_stop_play();
  MP_THREAD_GIL_ENTER();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_8bit_voice_board_stop_all_obj, codey_8bit_voice_board_stop_all);

STATIC void codey_8bit_voice_board_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC mp_obj_t codey_8bit_voice_board_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  return mp_const_none;
}

STATIC const mp_map_elem_t codey_8bit_voice_board_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_play),                (mp_obj_t)&codey_8bit_voice_board_play_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_play_to_stop),        (mp_obj_t)&codey_8bit_voice_board_play_to_stop_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_init),                (mp_obj_t)&codey_8bit_voice_board_init_obj },    
  { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),              (mp_obj_t)&codey_8bit_voice_board_deinit_obj },  
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_volume),          (mp_obj_t)&codey_8bit_voice_board_set_volume_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_volume),          (mp_obj_t)&codey_8bit_voice_board_get_volume_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_change_volume),       (mp_obj_t)&codey_8bit_voice_board_change_volume_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_play_note),           (mp_obj_t)&codey_8bit_voice_board_play_note_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_play_note_to_stop),   (mp_obj_t)&codey_8bit_voice_board_play_note_to_stop_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_stop_all),            (mp_obj_t)&codey_8bit_voice_board_stop_all_obj },
};

STATIC MP_DEFINE_CONST_DICT(codey_8bit_voice_board_locals_dict, codey_8bit_voice_board_locals_dict_table);

const mp_obj_type_t codey_8bit_voice_board_type =
{
  { &mp_type_type },
  .name = MP_QSTR_8bit_voice_board,
  .print = codey_8bit_voice_board_print,
  .call = codey_8bit_voice_board_call,
  .make_new = codey_8bit_voice_board_make_new,
  .locals_dict = (mp_obj_t)&codey_8bit_voice_board_locals_dict,
};

