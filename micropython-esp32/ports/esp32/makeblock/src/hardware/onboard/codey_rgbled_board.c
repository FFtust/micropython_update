/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for rgb led module
 * @file    codey_rgbled_board.c
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
 * This file is a drive rgbled_board module.
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

#include "driver/timer.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "codey_rgbled_board.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG      ("codey_rgb")
#define MAX_DUTY (1024)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
typedef struct 
{
  uint8_t channel;
  uint8_t io;
  uint8_t mode;
  uint8_t timer_idx;
}codey_rgbled_info_t;

typedef struct
{
  uint8_t rgb_r;
  uint8_t rgb_g;
  uint8_t rgb_b;
}codey_rgbled_data_t;

typedef struct
{
  mp_obj_base_t    base;
  codey_rgbled_data_t led_data;;
}codey_rgbled_board_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
static codey_rgbled_board_obj_t s_codey_rgbled_board_obj = {.base = {&codey_rgbled_board_type}};
static bool s_codey_rgbled_board_init_flag = false;
static bool s_codey_rgbled_red_board_init_flag = false;
static uint32_t s_codey_rgbled_intensity[3] = { 0, 0, 0 };
static uint8_t s_codey_rgbled_last_value[3] = {0, 0, 0};

static codey_rgbled_info_t s_ledc_ch[3] =
{
  { // R
    .channel   = CODEY_RGBLED_CH0_CHANNEL,
    .io        = CODEY_RGBLED_CH0_GPIO,
    .mode      = CODEY_RGBLED_MODE,
    .timer_idx = CODEY_RGBLED_TIMER
  },
  { // G
    .channel   = CODEY_RGBLED_CH1_CHANNEL,
    .io        = CODEY_RGBLED_CH1_GPIO,
    .mode      = CODEY_RGBLED_MODE,
    .timer_idx = CODEY_RGBLED_TIMER
  },
  { // B
    .channel   = CODEY_RGBLED_CH2_CHANNEL,
    .io        = CODEY_RGBLED_CH2_GPIO,
    .mode      = CODEY_RGBLED_MODE,
    .timer_idx = CODEY_RGBLED_TIMER
  },
};                                                       
/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC uint32_t  codey_rgbled_board_intensity_to_duty_t(uint16_t intensity);
STATIC void      codey_rgbled_board_red_config_t(void) ; 
STATIC void      codey_rgbled_board_set_rgb_separately_t(uint8_t val, uint8_t index);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
/* driver for signl RGB led */
void codey_rgbled_board_config_t(void)  
{  
  if(s_codey_rgbled_board_init_flag == false)
  {  
    int ch;
    ledc_timer_config_t ledc_timer = 
    {
      .bit_num    = LEDC_TIMER_10_BIT, //set timer counter bit number
      .freq_hz    = 5000,              //set frequency of pwm
      .speed_mode = CODEY_RGBLED_MODE,   //timer mode,
      .timer_num  = CODEY_RGBLED_TIMER    //timer index
    };
    ledc_timer_config(&ledc_timer);
    
    for(ch = 1; ch < 3; ch++)  // red channel will be config later
    {
      ledc_channel_config_t ledc_channel = 
      {
        .channel    = s_ledc_ch[ch].channel,
        .duty       = 0,
        .gpio_num   = s_ledc_ch[ch].io,
        .intr_type  = LEDC_INTR_DISABLE,
        .speed_mode = s_ledc_ch[ch].mode,
        .timer_sel  = s_ledc_ch[ch].timer_idx,
      };
      ledc_channel_config(&ledc_channel);
    }
    s_codey_rgbled_board_init_flag = true;
  }
}

/* the value rgb_r & _g &_b shoule be set between 1-100 */
void codey_rgbled_board_set_color_t(uint8_t rgb_r, uint8_t rgb_g, uint8_t rgb_b)
{  
  codey_rgbled_board_set_rgb_separately_t(rgb_r, 0);
  codey_rgbled_board_set_rgb_separately_t(rgb_g, 1);
  codey_rgbled_board_set_rgb_separately_t(rgb_b, 2);
}

uint8_t codey_rgbled_board_get_color_intensity_t(uint8_t rgb)
{
  if((rgb > 0) && (rgb <= 2))
  {
    return s_codey_rgbled_last_value[rgb];    
  }
  return 0;
}

void codey_rgbled_board_reload_t(void)
{
  codey_rgbled_board_set_color_t(s_codey_rgbled_last_value[0], s_codey_rgbled_last_value[1], s_codey_rgbled_last_value[2]);
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
/* init the red led when firstly used */
STATIC void codey_rgbled_board_red_config_t(void)  
{
  if(s_codey_rgbled_red_board_init_flag == false)
  {  
    ledc_channel_config_t ledc_channel = 
    {
      .channel    = s_ledc_ch[0].channel,
      .duty       = 0,
      .gpio_num   = s_ledc_ch[0].io,
      .intr_type  = LEDC_INTR_DISABLE,
      .speed_mode = s_ledc_ch[0].mode,
      .timer_sel  = s_ledc_ch[0].timer_idx,
    };
    ledc_channel_config(&ledc_channel);
    s_codey_rgbled_red_board_init_flag = true;
  }
}

STATIC uint32_t  codey_rgbled_board_intensity_to_duty_t(uint16_t intensity)
{
  uint32_t temp = 0;
  temp = (intensity * 4);
  temp = temp >= MAX_DUTY ? MAX_DUTY : temp;
  return temp;
}

/* the value rgb_r & _g &_b shoule be set between 1-100 */
/* index should be 0-2 */
STATIC void codey_rgbled_board_set_rgb_separately_t(uint8_t val, uint8_t index)
{  
  if(index > 2)
  {
    return;
  }
  if(index == 0) // red
  {
    s_codey_rgbled_intensity[0] = MAX_DUTY - codey_rgbled_board_intensity_to_duty_t(val);
    if(s_codey_rgbled_intensity[0] != 0)
    {
      codey_rgbled_board_red_config_t();
    }
  }
  else
  {
    s_codey_rgbled_intensity[index] = codey_rgbled_board_intensity_to_duty_t(val);
  }

  ledc_set_duty(s_ledc_ch[index].mode, s_ledc_ch[index].channel, s_codey_rgbled_intensity[index]);//frequebce is 5000
  ledc_update_duty(s_ledc_ch[index].mode, s_ledc_ch[index].channel);

}

/******************************************************************************/
STATIC mp_obj_t codey_rgbled_board_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
  
  // setup the object
  codey_rgbled_board_obj_t *self = &s_codey_rgbled_board_obj;
  self->base.type = &codey_rgbled_board_type;
  return self;
}

STATIC mp_obj_t codey_rgbled_board_stop(mp_obj_t self_in)
{ 
  codey_rgbled_board_stop_t();
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_rgbled_board_stop_obj, codey_rgbled_board_stop);

STATIC mp_obj_t codey_rgbled_board_separately(mp_obj_t self_in, mp_obj_t arg1, mp_obj_t arg2)
{
  uint8_t val = mp_obj_get_int(arg1);
  uint8_t index = mp_obj_get_int(arg2);
  
  codey_rgbled_board_set_rgb_separately_t(val, index);
  s_codey_rgbled_last_value[index] = val;
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(codey_rgbled_board_separately_obj, codey_rgbled_board_separately);


STATIC mp_obj_t codey_rgbled_board_set_color(mp_uint_t n_args, const mp_obj_t *args)
{
  codey_rgbled_board_obj_t *self = args[0];

  self->led_data.rgb_r = abs(mp_obj_get_int(args[1]));
  self->led_data.rgb_g = abs(mp_obj_get_int(args[2]));
  self->led_data.rgb_b = abs(mp_obj_get_int(args[3]));
  s_codey_rgbled_last_value[0] = self->led_data.rgb_r;
  s_codey_rgbled_last_value[1] = self->led_data.rgb_g;
  s_codey_rgbled_last_value[2] = self->led_data.rgb_b;
  
  codey_rgbled_board_set_color_t(self->led_data.rgb_r, self->led_data.rgb_g, self->led_data.rgb_b);
 
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(codey_rgbled_board_set_color_obj, 4, 4, codey_rgbled_board_set_color);

STATIC void codey_rgbled_board_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC mp_obj_t codey_rgbled_board_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  
  return mp_const_none; 
}

STATIC const mp_map_elem_t codey_rgbled_board_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_color),       (mp_obj_t)&codey_rgbled_board_set_color_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_r_g_b),       (mp_obj_t)&codey_rgbled_board_separately_obj },  
};

STATIC MP_DEFINE_CONST_DICT(codey_rgbled_board_locals_dict, codey_rgbled_board_locals_dict_table);

const mp_obj_type_t codey_rgbled_board_type =
{
  { &mp_type_type },
  .name = MP_QSTR_rgbled_board,
  .print = codey_rgbled_board_print,
  .call = codey_rgbled_board_call,
  .make_new = codey_rgbled_board_make_new,
  .locals_dict = (mp_obj_t)&codey_rgbled_board_locals_dict,
};
