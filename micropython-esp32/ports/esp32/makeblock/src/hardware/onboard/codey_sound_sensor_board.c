/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for sound sensor module
 * @file    codey_sound_sensor_board.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/05/26
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
 * This file is a drive sound_sensor_board module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/05/26      1.0.0              build the new.
 * </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "py/mpstate.h"
#include "py/runtime.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_adc_cal.h"

#include "codey_sound_sensor_board.h"
#include "codey_event_mechanism.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define V_REF           (1100)
#define VALUE_TO_SENSOR (100.0 / 4095)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
typedef struct
{
  mp_obj_base_t base;
}codey_sound_sensor_board_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
static codey_sound_sensor_board_obj_t s_codey_sound_sensor_board_obj = {.base = {&codey_sound_sensor_board_type}};
static bool s_codey_sound_sensor_board_init_flag = false;
static esp_adc_cal_characteristics_t s_characteristics;
static float s_codey_sound_sensor_board_value = 0.0;
/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC void codey_sound_sensor_event_listening(float cur_value);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_sound_sensor_board_config_t(void)
{
  if(s_codey_sound_sensor_board_init_flag == false)
  {
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(CODEY_SOUND_SENSOR_BOARD_CHANNEL, ADC_ATTEN_11db);
    esp_adc_cal_get_characteristics(V_REF, ADC_ATTEN_11db, ADC_WIDTH_12Bit, &s_characteristics);
    s_codey_sound_sensor_board_init_flag = true;
  }
}

float codey_sound_sensor_board_get_value_t(void)
{
  float value;
  uint16_t val_max = 0;
  uint16_t val_min = 4096;
  float adc_data;
  float sum = 0;
  for(uint8_t i = 0; i < 10; i++)
  {
    adc_data = adc1_get_raw(CODEY_SOUND_SENSOR_BOARD_CHANNEL);
    sum += adc_data;
    if(adc_data > val_max)
    {
      val_max = adc_data;
    }
    if(adc_data < val_min)
    { 
      val_min = adc_data;
    }
  }
  sum = sum - val_max - val_min;
  value = sum / 8;
  s_codey_sound_sensor_board_value = value;
  codey_sound_sensor_event_listening(value * VALUE_TO_SENSOR);

  return value;
}

float codey_sound_sensor_board_read_value_t(void)
{
  return s_codey_sound_sensor_board_value;
}

/******************************************************************************/
STATIC void codey_sound_sensor_event_listening(float cur_value)
{ 
  if(!codey_eve_get_start_flag_t())
  {
    return;
  }
  codey_eve_trigger_t(EVE_SOUND_OVER, &cur_value);
}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
STATIC mp_obj_t codey_sound_sensor_board_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  // setup the object
  codey_sound_sensor_board_obj_t *self = &s_codey_sound_sensor_board_obj;
  self->base.type = &codey_sound_sensor_board_type;
  return self;
}

STATIC mp_obj_t codey_sound_sensor_board_value(mp_obj_t self_in)
{
  float value = 0;
  value = codey_sound_sensor_board_read_value_t();

  return mp_obj_new_float(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_sound_sensor_board_value_obj, codey_sound_sensor_board_value);

STATIC void codey_sound_sensor_board_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
  
}

STATIC mp_obj_t codey_sound_sensor_board_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  
  return mp_const_none;
}

STATIC const mp_map_elem_t codey_sound_sensor_board_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_value),               (mp_obj_t)&codey_sound_sensor_board_value_obj },
};

STATIC MP_DEFINE_CONST_DICT(codey_sound_sensor_board_locals_dict, codey_sound_sensor_board_locals_dict_table);

const mp_obj_type_t codey_sound_sensor_board_type =
{
  { &mp_type_type },
  .name = MP_QSTR_sound_sensor_board,
  .print = codey_sound_sensor_board_print,
  .call = codey_sound_sensor_board_call,
  .make_new = codey_sound_sensor_board_make_new,
  .locals_dict = (mp_obj_t)&codey_sound_sensor_board_locals_dict,
};



