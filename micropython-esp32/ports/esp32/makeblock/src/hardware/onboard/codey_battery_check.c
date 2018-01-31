/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for power manager module
 * @file    codey_battery_check.c
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
 * This file is a drive battery_check module.
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

#include "esp_log.h"    
#include "driver/uart.h"
#include "driver/adc.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/gpio.h"

#include "codey_sys.h"
#include "esp_adc_cal.h"
#include "codey_gyro_board.h"
#include "codey_battery_check.h"
#include "codey_config.h"
#include "codey_rgbled_board.h"
#include "codey_ledmatrix.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG                        ("codey_battery")
#define V_REF                      (1100)
#define BATTERY_CAPACITY_TABLE_LEN (13)
#define POWER_OFF_BATTERY_VOLTAGE  (3.4)
#define POWER_OFF_BATTERY_CAPACITY (15)
#define BATTERY_VOLTAGE_OFFSET     (0.2)
#define ADC_VALUE_TO_VOL           (0.001934) // (1 / 4096) * 1.1 * 3.6 * 2;

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
typedef struct
{
  float battery_voltage;
  uint8_t battery_capacity;
}battrey_capacity_table_t;

typedef struct
{
  mp_obj_base_t base;
}codey_battery_check_obj_t;

const battrey_capacity_table_t capacity_table[15] =
{
  { 4.2, 100 },
  { 4.08, 90 },
  { 4.00, 80 },
  { 3.93, 70 },
  { 3.87, 60 },
  { 3.82, 50 },
  { 3.79, 40 },
  { 3.77, 30 },
  { 3.73, 20 },
  { 3.70, 15 },
  { 3.68, 10 },
  { 3.50, 5  },
  { 2.50, 0  }
};

const uint8_t indicate_face_table1[16] = {0, 0, 0, 0x7e, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7e, 0x18, 0, 0, 0};
const uint8_t indicate_face_table2[16] = {0, 0, 0, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7e, 0x18, 0, 0, 0};

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
static codey_battery_check_obj_t s_codey_battery_check_obj = {.base = {&codey_battery_check_type}};
static bool s_codey_battery_check_init_flag = false;
static esp_adc_cal_characteristics_t s_characteristics;
static uint8_t s_codey_battery_capacity = 100;
static float s_codey_battery_check_value = 0.0;
 
/******************************************************************************
 DEFINE PUBLIC DATA
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
float codey_battery_check_get_value_t(void)
{
  float value;
  uint16_t val_max = 0;
  uint16_t val_min = 4096;
  uint16_t adc_data;
  float sum = 0;
  for(uint8_t i = 0; i < 10; i++)
  {
    adc_data = (float)adc1_to_voltage(CODEY_BATTERY_CHECK_CHANNEL, &s_characteristics);
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
  sum = sum-val_max - val_min;
  value = sum / 8;
  s_codey_battery_check_value = value;
  
  return value;  
}

float codey_battery_check_get_voltage_t(void)
{
  float vol = 0;
  vol = s_codey_battery_check_value;
  vol = ADC_VALUE_TO_VOL * vol;
  vol += BATTERY_VOLTAGE_OFFSET;
  return vol;
}

uint8_t codey_battery_check_get_capacity(void)
{
  uint8_t index = 0;
  float vol = codey_battery_check_get_voltage_t();
  for(; index < BATTERY_CAPACITY_TABLE_LEN; index++)
  {
    if(vol > capacity_table[index].battery_voltage)
    {
      break;
    }
  }
  s_codey_battery_capacity = capacity_table[index].battery_capacity;
  return s_codey_battery_capacity;
}

void codey_battery_low_capacity_indicate_t(void)
{
  /* add codes here */
  codey_rgbled_board_set_color_t(100, 0, 0);
  for(uint8_t i = 0; i < 10; i++)
  {
    codey_ledmatrix_show_faceplate_t(indicate_face_table1);
    ets_delay_us(600000);
    codey_ledmatrix_show_faceplate_t(indicate_face_table2);
    ets_delay_us(600000); 
  }
  codey_rgbled_board_set_color_t(0, 0, 0);
  codey_ledmatrix_screen_clean_t();
  
}

void codey_battery_low_capacity_check(void)
{
  float vol = codey_battery_check_get_voltage_t();
  if(vol <= POWER_OFF_BATTERY_VOLTAGE)
  {
    codey_battery_low_capacity_indicate_t();
    codey_board_power_off_set_t();
  }
}

void codey_battery_check_config_t(void)
{
  if(s_codey_battery_check_init_flag == false)
  {
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(CODEY_BATTERY_CHECK_CHANNEL, ADC_ATTEN_11db);
    esp_adc_cal_get_characteristics(V_REF, ADC_ATTEN_11db, ADC_WIDTH_12Bit, &s_characteristics);
        
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;   
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = ((uint64_t)1 << CODEY_POWER_KEY_IO) ;//GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;   
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = ((uint64_t)1 << CODEY_POWER_CONTROL_IO) ;//GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    s_codey_battery_check_init_flag = true;
  }
}

void codey_board_power_on_set_t(void)
{
  gpio_set_level(CODEY_POWER_CONTROL_IO, CODEY_POWER_ON_SET_STATUS);  
}

void codey_board_power_off_set_t(void)
{
  gpio_set_level(CODEY_POWER_CONTROL_IO, CODEY_POWER_OFF_SET_STATUS);  
}

bool codey_board_is_power_key_pressed_t(void)
{
  if(gpio_get_level(CODEY_POWER_KEY_IO) == CODEY_POWER_CHECK_PRESSED_STATUS)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/* this GPIO should be set to high level as soon as power on */
void codey_board_power_status_check_t(void)
{ 
  static uint32_t start_time = 0;
  static int last_button_sta = 0;
  uint8_t count = 0;
  if(codey_board_is_power_key_pressed_t())
  {
    if(last_button_sta == 0)
    {
      if(millis() > CODEY_POWER_OFF_START_TIME_MS)
      {
        start_time = millis();
        last_button_sta = 1;
      }
    }
  }
  else
  {
    if((last_button_sta == 1) && ((millis() - start_time) > CODEY_POWER_SWITCH_CONTINUE_TIME_MS))
    { 
      while(codey_gyro_board_get_ope_i2c_flag_t())
      {
        vTaskDelay(10);
        if(count++ > 20)
        {
          break;
        }
      }
      codey_board_power_off_set_t();
    }
    last_button_sta = 0;
  }
#if CODEY_LOW_ENERGY_POWER_OFF
  codey_battery_low_capacity_check();
#endif

}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
STATIC mp_obj_t codey_battery_check_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  // setup the object
  codey_battery_check_obj_t *self = &s_codey_battery_check_obj;
  self->base.type = &codey_battery_check_type;
  
  return self;
}

STATIC mp_obj_t codey_battery_check_value_voltage(mp_obj_t self_in)
{
  float vol = 0;
  vol = codey_battery_check_get_voltage_t();
  
  return mp_obj_new_float(vol);    
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_battery_check_value_voltage_obj, codey_battery_check_value_voltage);

STATIC mp_obj_t codey_battery_check_value_capacity(mp_obj_t self_in)
{
  uint8_t cap = 0;
  cap = codey_battery_check_get_capacity();
  
  return mp_obj_new_int(cap);    
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_battery_check_value_capacity_obj, codey_battery_check_value_capacity);

STATIC void codey_battery_check_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
 
}

STATIC mp_obj_t codey_battery_check_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);

  return mp_const_none;
}

STATIC const mp_map_elem_t codey_battery_check_locals_dict_table[] =
{  
  { MP_OBJ_NEW_QSTR(MP_QSTR_battery_vol),         (mp_obj_t)&codey_battery_check_value_voltage_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_battery_cap),         (mp_obj_t)&codey_battery_check_value_capacity_obj },

};

STATIC MP_DEFINE_CONST_DICT(codey_battery_check_locals_dict, codey_battery_check_locals_dict_table);

const mp_obj_type_t codey_battery_check_type =
{
  { &mp_type_type },
  .name = MP_QSTR_,
  .print = codey_battery_check_print,
  .call = codey_battery_check_call,
  .make_new = codey_battery_check_make_new,
  .locals_dict = (mp_obj_t)&codey_battery_check_locals_dict,
};
