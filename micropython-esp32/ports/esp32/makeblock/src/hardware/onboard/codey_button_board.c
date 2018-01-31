/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for button module
 * @file    codey_button_board.c
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

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_task.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/gpio.h"

#include "codey_sys.h"
#include "codey_button_board.h"
#include "codey_event_mechanism.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG                           ("codey_button")
#define SECOND_TRIGGER_INTERVAL       (700) 
#define THIRD_OVER_TRIGGER_INTERVAL   (100) 
/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/  
typedef struct
{
  mp_obj_base_t base;
} codey_button_board_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATAS
 ******************************************************************************/
static codey_button_board_obj_t s_codey_button_board_obj = {.base = {&codey_button_board_type }};
static bool s_codey_button_board_init_flag = false;
static uint8_t s_codey_button_board_value  = 0;

/******************************************************************************
 DECLARE PUBLIC FUNCTIONS  (neurons will call this functions so make it public )
 ******************************************************************************/
void    codey_button_board_config_t(void);
uint8_t codey_button_board_get_status_t(uint button_io);
uint8_t codey_button_board_get_all_status_t(void);
bool    codey_is_button_released_t(uint8_t button_id);
bool    codey_is_button_pressed_t(uint8_t button_id);
uint8_t codey_button_board_read_status_t(void);

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC bool codey_button_board_id_check_t(uint button_id);
STATIC void codey_button_board_event_listening(uint8_t cur_value);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_button_board_config_t(void)  
{
  if(s_codey_button_board_init_flag == false)
  {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;   
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1 << BUTTON1_IO) ;// GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE; 
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1 << BUTTON2_IO) ;// GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf); 

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;     
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1 << BUTTON3_IO) ;// GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf); 
   
    s_codey_button_board_init_flag = true;
  }
}

uint8_t codey_button_board_get_status_t(uint button_io)
{
  if(button_io == BUTTON3_IO)  // the  gpios of button3 are pull_up,but the others are pull_down
  {                                                   // this  just make them a unification case
    return (uint8_t)(!gpio_get_level(button_io));
  }
  else
  {
    return (uint8_t)((gpio_get_level(button_io)));
  }
}
/* this function will be called in the sensors update task */
uint8_t codey_button_board_get_all_status_t(void)
{
  uint8_t temp = 0;
  temp = (uint8_t)(codey_button_board_get_status_t(BUTTON1_IO));
  temp = temp | ((uint8_t)(codey_button_board_get_status_t(BUTTON2_IO))) << 1;
  temp = temp | ((uint8_t)(codey_button_board_get_status_t(BUTTON3_IO))) << 2;

  codey_button_board_event_listening(temp);
  s_codey_button_board_value = temp;
  return temp;
}

/* users should get button status by call this function */
uint8_t codey_button_board_read_status_t()
{
  return s_codey_button_board_value;
}

bool codey_is_button_pressed_t(uint8_t button_id)
{
  uint8_t value = 0x00;
  switch(button_id)
  {
    case 1:
      value = codey_button_board_read_status_t() & 0x01 ;
    break;
    case 2:
      value = codey_button_board_read_status_t() & 0x02;
    break;
    case 3:
      value = codey_button_board_read_status_t() & 0x04;
    break;
  }

  if(value == 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool codey_is_button_released_t(uint8_t button_id)
{
  uint8_t value = 0;
  switch(button_id)
  {
   case 1:
     value = codey_button_board_read_status_t() & 0x01;
   break;
   case 2:
     value = codey_button_board_read_status_t() & 0x02;
   break;
   case 3:
     value = codey_button_board_read_status_t() & 0x04;
   break;
  }
  
  if(value == 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC bool codey_button_board_id_check_t(uint button_id)
{
  if((button_id > 0 && button_id <= BUTTON_NUM) || button_id == 255)  // 255 means all buttons
  {
    return true;
  }
  else
  {
    return false;
  }
}

STATIC void codey_button_board_event_listening(uint8_t cur_value)
{
  if(!codey_eve_get_start_flag_t())
  {
    return;
  }
  static uint32_t last_tick[BUTTON_NUM] = {0, 0 ,0};
  static uint32_t  trigger_times[BUTTON_NUM] = {0, 0, 0};
  for(uint8_t i = 0; i < BUTTON_NUM; i++)
  {
    if((cur_value & (1 << i)) != 0) //press
    {
      if(trigger_times[i] == 0)
      {
        codey_eve_trigger_t(2 * i + 1, NULL); // every button has two event types, press and release
        last_tick[i] = millis();
        trigger_times[i]++;
      }
      else if(trigger_times[i] == 1)
      {
        if((millis() - last_tick[i]) > SECOND_TRIGGER_INTERVAL)
        {
          codey_eve_trigger_t(2 * i + 1, NULL);
          trigger_times[i]++;
          last_tick[i] = millis();
        }
        else
        {
          continue;
        }
      }
      else
      {
        if((millis() - last_tick[i]) > THIRD_OVER_TRIGGER_INTERVAL)
        {
          codey_eve_trigger_t(2 * i + 1, NULL);
          trigger_times[i]++;
          last_tick[i] = millis();
        }
        else
        {
          continue;
        }
      }
      
    }
    else if((cur_value & (1 << i)) == 0)  //release
    {
      trigger_times[i] = 0;
      last_tick[i] = 0;
    }
  }
}

/* binding to micropython */
STATIC mp_obj_t codey_button_board_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
  
  // setup the object
  codey_button_board_obj_t *self = &s_codey_button_board_obj;
  self->base.type = &codey_button_board_type;
  
  return self;
}

STATIC mp_obj_t codey_button_board_value(mp_obj_t self_in, mp_obj_t arg1)
{
  uint8_t button_id = 0;
  button_id = mp_obj_get_int(arg1);
  if(codey_button_board_id_check_t(button_id))
  {
    uint8_t value = 0;
    switch(button_id)
    {
      case 1:
        value = (codey_button_board_read_status_t() & 0x01) ? 1 : 0;
      break;
      case 2:
        value = (codey_button_board_read_status_t() & 0x02) ? 1 : 0;
      break;
      case 3:
        value = (codey_button_board_read_status_t() & 0x04) ? 1 : 0;
      break;
      case 255:
        {
          value = codey_button_board_read_status_t();
        }
      break;
      default:
      break;
    }
    return mp_obj_new_int(value);
  }

  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_button_board_value_obj, codey_button_board_value);

STATIC void codey_button_board_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC mp_obj_t codey_button_board_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  
  return mp_const_none;
}

STATIC const mp_map_elem_t codey_button_board_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_value),               (mp_obj_t)&codey_button_board_value_obj },
};

STATIC MP_DEFINE_CONST_DICT(codey_button_board_locals_dict, codey_button_board_locals_dict_table);

const mp_obj_type_t codey_button_board_type =
{
  { &mp_type_type },
  .name = MP_QSTR_button_board,
  .print = codey_button_board_print,
  .call = codey_button_board_call,
  .make_new = codey_button_board_make_new,
  .locals_dict = (mp_obj_t)&codey_button_board_locals_dict,
};