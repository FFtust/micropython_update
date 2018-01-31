/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for hardware version  module
 * @file    codey_hardware_version.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/05/21
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
 * This file is a drive hardware_version module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/05/21       1.0.0              build the new.
 * </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>	

#include "py/mpstate.h"
#include "py/runtime.h"

#include "driver/uart.h"
#include "driver/adc.h"
#include "soc/uart_struct.h"
#include "uart.h"

#include "codey_hardware_version.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
typedef struct
{
  mp_obj_base_t base;
}codey_hardware_version_obj_t;

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
static codey_hardware_version_obj_t s_codey_hardware_version_obj = {{&codey_hardware_version_type}};
static bool s_codey_hardware_version_init_flag = false;

/******************************************************************************
 DEFINE PUBLIC DATAS
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC void  codey_hardware_version_config_t(void);
STATIC float codey_hardware_version_get_value_t(void);
STATIC float codey_hardware_version_value_parse_t(float ad_value);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC void codey_hardware_version_config_t(void)
{
  if(s_codey_hardware_version_init_flag == false)
  {
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(CODEY_HARDWARE_VERSION_CHANNEL, ADC_ATTEN_11db);
    s_codey_hardware_version_init_flag = true;
  }
}

STATIC float codey_hardware_version_get_value_t(void)
{
  float value;
  uint16_t val_max = 0;
  uint16_t val_min = 4096;
  uint16_t adc_data;
  float sum = 0;
  for(uint8_t i = 0; i < 10; i++)
  {
    adc_data = adc1_get_voltage(CODEY_HARDWARE_VERSION_CHANNEL);
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

  return value;
}

STATIC float codey_hardware_version_value_parse_t(float ad_value) // pasre the adc value.retuen the version information ex:ad_value 3723 means V1.0.1
{                                                                                   // return float 1.01
  if(abs(ad_value - 3723) < 100)
  {
    return 1.01; 
  }
  else
  {
    return 0.00;
  }
}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
STATIC mp_obj_t codey_hardware_version_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  // setup the object
  codey_hardware_version_obj_t *self = &s_codey_hardware_version_obj;
  self->base.type = &codey_hardware_version_type;
  
  return self;
}

STATIC mp_obj_t codey_hardware_version_init(mp_obj_t self_in)
{
  codey_hardware_version_config_t();

  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_hardware_version_init_obj, codey_hardware_version_init);

STATIC mp_obj_t codey_hardware_version_value(mp_obj_t self_in)
{
  float value = 0;
  value = codey_hardware_version_get_value_t();
  return mp_obj_new_float(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_hardware_version_value_obj, codey_hardware_version_value);

STATIC mp_obj_t codey_hardware_version_version(mp_obj_t self_in)
{
  float value;
  value = codey_hardware_version_value_parse_t(codey_hardware_version_get_value_t());
  
  return mp_obj_new_float(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_hardware_version_version_obj, codey_hardware_version_version);

STATIC void codey_hardware_version_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
 
}

STATIC mp_obj_t codey_hardware_version_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);

  return mp_const_none;
}

STATIC const mp_map_elem_t codey_hardware_version_locals_dict_table[] =
{  
  { MP_OBJ_NEW_QSTR(MP_QSTR_init),                (mp_obj_t)&codey_hardware_version_init_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_value),               (mp_obj_t)&codey_hardware_version_value_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_version),             (mp_obj_t)&codey_hardware_version_version_obj },
};

STATIC MP_DEFINE_CONST_DICT(codey_hardware_version_locals_dict, codey_hardware_version_locals_dict_table);

const mp_obj_type_t codey_hardware_version_type =
{
  { &mp_type_type },
  .name = MP_QSTR_hardware_version,
  .print = codey_hardware_version_print,
  .call = codey_hardware_version_call,
  .make_new = codey_hardware_version_make_new,
  .locals_dict = (mp_obj_t)&codey_hardware_version_locals_dict,
};

