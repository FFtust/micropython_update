/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for event module
 * @file    codey_event_mechanism.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/08/31
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
 * This file is a drive codey_event module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/08/31      1.0.0              build the new.
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
#include "esp_heap_caps.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/gpio.h"
#include "codey_config.h"
#include "codey_event_mechanism.h"

/******************************************************************************
 DEFINE MACRO 
 ******************************************************************************/
#define TAG               ("codey_event")
#define CODEY_EVE_MAX_NUM (16)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/  
typedef struct
{
  mp_obj_base_t base;
}codey_event_obj_t;

typedef struct
{
  uint8_t event_id;
  codey_event_type_t event_type;
  volatile bool      is_event_register;
  volatile bool      eve_occured;
  union eve_para_t
  {
    uint8_t byte_val[64];
    float   float_val; 
  }eve_para;

}codey_eve_single_t;

struct codey_eve_manager_t
{
  codey_eve_single_t *codey_eve_single[CODEY_EVE_MAX_NUM];
  uint8_t codey_eve_cur_num;
}codey_eve_manager;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
static codey_event_obj_t s_codey_event_obj = {.base={ &codey_event_type}};
static uint8_t s_codey_eve_id = 0; 
static bool s_codey_eve_started = false;
static bool s_codey_eve_triggerd[EVENT_TYPE_MAX] = {0};
/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC bool    codey_eve_get_occured_flag_t(uint8_t eve_id);
STATIC void    codey_eve_set_occured_flag_t(uint8_t eve_id, bool sta);
STATIC uint8_t codey_eve_get_event_number_t(void);
STATIC int8_t  codey_eve_register_t(codey_event_type_t eve_type, void *para);
STATIC bool    codey_eve_trigger_check(uint8_t eve_id, void *para);
STATIC void    codey_eve_set_start_flag_t(bool);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_eve_init_t(void)
{
  codey_eve_manager.codey_eve_cur_num = 0;
  codey_eve_set_start_flag_t(false);
  memset(s_codey_eve_triggerd, 0, EVENT_TYPE_MAX);
  ESP_LOGI(TAG, "event init succeed");
}

void codey_eve_deinit_t(void)
{
  for(uint8_t i = 0; i < CODEY_EVE_MAX_NUM; i++)
  {
    if(codey_eve_manager.codey_eve_single[i] != NULL)
    {
      codey_eve_manager.codey_eve_single[i]->is_event_register = false;
      heap_caps_free(codey_eve_manager.codey_eve_single[i]);
      codey_eve_manager.codey_eve_single[i] = NULL; 
    }
    
  }
  s_codey_eve_id = 0;
  codey_eve_manager.codey_eve_cur_num = 0;
  codey_eve_set_start_flag_t(false);
  memset(s_codey_eve_triggerd, 0, EVENT_TYPE_MAX);
  ESP_LOGI(TAG, "event deinit succeed");
}

void codey_eve_trigger_t(codey_event_type_t eve_type, void *para)
{
  for(uint8_t i = 0; i < codey_eve_manager.codey_eve_cur_num; i++)
  {
    if(!codey_eve_manager.codey_eve_single[i])
    {
      continue;
    }
    if(eve_type == codey_eve_manager.codey_eve_single[i]->event_type)
    {
      if(codey_eve_manager.codey_eve_single[i]->is_event_register == true)
      {
        if(!codey_eve_trigger_check(i, para))
        { 
          continue;
        }
        codey_eve_set_occured_flag_t(i, true);
        ESP_LOGI(TAG, "eve triggered %d \n", i);
      }
    }
  }
}

bool codey_eve_get_start_flag_t(void)
{
  return s_codey_eve_started;
}

void codey_eve_set_triggerd_flag_t(codey_event_type_t eve_type, bool sta)
{
  s_codey_eve_triggerd[eve_type] = sta;
}

bool codey_eve_get_triggerd_flag_t(codey_event_type_t eve_type)
{
  return s_codey_eve_triggerd[eve_type];
}


/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC void codey_eve_set_start_flag_t(bool sta)
{
  s_codey_eve_started = sta;
}

STATIC bool codey_eve_get_occured_flag_t(uint8_t eve_id)
{
  return codey_eve_manager.codey_eve_single[eve_id]->eve_occured;
}

STATIC void codey_eve_set_occured_flag_t(uint8_t eve_id, bool sta)
{
  codey_eve_manager.codey_eve_single[eve_id]->eve_occured = sta;
}

STATIC uint8_t codey_eve_get_event_number_t(void)
{
  return s_codey_eve_id;
}

STATIC int8_t codey_eve_register_t(codey_event_type_t eve_type, void *para)
{
  ESP_LOGI(TAG, "c eve id is %d, type is %d\n", s_codey_eve_id, eve_type);
  if(s_codey_eve_id >= CODEY_EVE_MAX_NUM)
  {
    ESP_LOGE(TAG, "event register error");
    return -1;
  }
  
  codey_eve_manager.codey_eve_single[s_codey_eve_id] = (codey_eve_single_t *)heap_caps_malloc(sizeof(codey_eve_single_t), MALLOC_CAP_8BIT);
  if(codey_eve_manager.codey_eve_single[s_codey_eve_id] == NULL)
  {
    ESP_LOGE(TAG, "eve register failed, alloc memory fialed\n");
  }
  codey_eve_manager.codey_eve_single[s_codey_eve_id]->is_event_register = true;
  codey_eve_manager.codey_eve_single[s_codey_eve_id]->event_type = eve_type;
  codey_eve_manager.codey_eve_single[s_codey_eve_id]->event_id = s_codey_eve_id;
  codey_eve_manager.codey_eve_single[s_codey_eve_id]->eve_occured = false;
  if(para != NULL)
  {
    if(eve_type <= 6) /* button  no para*/
    {
      ;
    }
    else if(eve_type <= 10) /* light & sound */
    {
      codey_eve_manager.codey_eve_single[s_codey_eve_id]->eve_para.float_val = *(float *)para;
    }
    else if(eve_type == EVE_MESSAGE) /* message */
    {
      strcpy((char *)codey_eve_manager.codey_eve_single[s_codey_eve_id]->eve_para.byte_val,(char *)para);
    }
    else /* NO PARA */
    {
      ; 
    }
  }
  s_codey_eve_id ++;
  codey_eve_manager.codey_eve_cur_num = s_codey_eve_id;
  return (s_codey_eve_id - 1);
}

STATIC bool codey_eve_trigger_check(uint8_t eve_id, void *para)
{
  if(s_codey_eve_id >= CODEY_EVE_MAX_NUM)
  {
   return false;
  }
  /* light and sound event need parameter, different with other events, use a special flag array */
  static bool light_sound_triggerd_flag[CODEY_EVE_MAX_NUM] = {0}; 
  if(codey_eve_manager.codey_eve_single[eve_id]->event_type == EVE_LIGHT_OVER 
     || codey_eve_manager.codey_eve_single[eve_id]->event_type == EVE_LIGHT_UNDER)
  {
    if(!light_sound_triggerd_flag[eve_id]
        && (codey_eve_manager.codey_eve_single[eve_id]->eve_para.float_val > *(float *)para))
    {
      light_sound_triggerd_flag[eve_id] = true;
      return true;
    }
    else 
    {
      if(codey_eve_manager.codey_eve_single[eve_id]->eve_para.float_val <= *(float *)para)
      {
        light_sound_triggerd_flag[eve_id] = false;
      }
      return false;
    }
  }
  else if(codey_eve_manager.codey_eve_single[eve_id]->event_type == EVE_SOUND_OVER
          || codey_eve_manager.codey_eve_single[eve_id]->event_type == EVE_SOUND_UNDER)
  {
    if(!light_sound_triggerd_flag[eve_id]
         && (codey_eve_manager.codey_eve_single[eve_id]->eve_para.float_val < *(float *)para))
    {
      light_sound_triggerd_flag[eve_id] = true;
      return true;
    }
    else
    {
      if(codey_eve_manager.codey_eve_single[eve_id]->eve_para.float_val > *(float *)para)
      {
        light_sound_triggerd_flag[eve_id] = false;
      }
      return false;
    }  
  }
  else if(codey_eve_manager.codey_eve_single[eve_id]->event_type == EVE_MESSAGE)
  {
    if(!strcmp((char *)(codey_eve_manager.codey_eve_single[eve_id]->eve_para.byte_val), (char *)para))
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return true;
  }
  return true;
}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
STATIC mp_obj_t codey_event_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
  
  // setup the object
  codey_event_obj_t *self = &s_codey_event_obj;
  self->base.type = &codey_event_type;
  
  return self;
}

STATIC mp_obj_t codey_event_deinit(mp_obj_t self_in)
{
  codey_eve_deinit_t();

  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_event_deinit_obj, codey_event_deinit);

STATIC mp_obj_t codey_event_init(mp_obj_t self_in)
{
  codey_eve_init_t();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_event_init_obj, codey_event_init);

STATIC mp_obj_t codey_event_start_trigger(mp_obj_t self_in)
{
  codey_eve_set_start_flag_t(true);
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_event_start_trigger_obj, codey_event_start_trigger);

STATIC mp_obj_t codey_event_trigger(mp_obj_t self_in, mp_obj_t arg1)
{
  codey_event_type_t eve_type = mp_obj_get_int(arg1);
  codey_eve_trigger_t(eve_type, NULL);
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_event_trigger_obj, codey_event_trigger);

STATIC mp_obj_t codey_event_register(size_t n_args, const mp_obj_t *args)
{
  uint8_t eve_type = mp_obj_get_int(args[1]);
  int8_t ret = -1; 
  if(eve_type != EVE_MESSAGE)
  {
    float threshold = 0;
    threshold = mp_obj_get_float(args[2]);
    ret = codey_eve_register_t(eve_type, &threshold);
  }
  else
  {
    size_t len;
    const char *msg_str = mp_obj_str_get_data(args[2], &len);
    ret = codey_eve_register_t(EVE_MESSAGE, (void *)msg_str);
  }  

  return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(codey_event_register_obj, 3, 3, codey_event_register);

STATIC mp_obj_t codey_eve_set_occured_flag(mp_obj_t self_in, mp_obj_t arg1, mp_obj_t arg2)
{
  uint8_t eve_id = mp_obj_get_int(arg1); 
  uint8_t sta = mp_obj_get_int(arg2); 

  codey_eve_set_occured_flag_t(eve_id, (bool)(sta));
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(codey_eve_set_occured_flag_obj, codey_eve_set_occured_flag);

STATIC mp_obj_t codey_eve_get_occured_flag(mp_obj_t self_in, mp_obj_t arg1)
{
  uint8_t eve_id = mp_obj_get_int(arg1); 
  bool sta = false;

  sta = codey_eve_get_occured_flag_t(eve_id);
  return mp_obj_new_bool(sta);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_eve_get_occured_flag_obj, codey_eve_get_occured_flag);

STATIC mp_obj_t codey_eve_get_event_number(mp_obj_t self_in)
{
  uint8_t num = 0;

  num = codey_eve_get_event_number_t();
  return mp_obj_new_int(num);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_eve_get_event_number_obj, codey_eve_get_event_number);

STATIC void codey_event_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
  
}

STATIC mp_obj_t codey_event_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  
  return mp_const_none;
}

STATIC const mp_map_elem_t codey_event_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_init),               (mp_obj_t)&codey_event_init_obj},
  { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),             (mp_obj_t)&codey_event_deinit_obj},
  { MP_OBJ_NEW_QSTR(MP_QSTR_register),           (mp_obj_t)&codey_event_register_obj},
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_occured_flag),   (mp_obj_t)&codey_eve_set_occured_flag_obj},
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_occured_flag),   (mp_obj_t)&codey_eve_get_occured_flag_obj},
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_event_number),   (mp_obj_t)&codey_eve_get_event_number_obj},
  { MP_OBJ_NEW_QSTR(MP_QSTR_event_trigger),      (mp_obj_t)&codey_event_trigger_obj},
  { MP_OBJ_NEW_QSTR(MP_QSTR_start_trigger),(mp_obj_t)&codey_event_start_trigger_obj},
};

STATIC MP_DEFINE_CONST_DICT(codey_event_locals_dict, codey_event_locals_dict_table);

const mp_obj_type_t codey_event_type =
{
  { &mp_type_type },
  .name = MP_QSTR_codey_eve,
  .print = codey_event_print,
  .call = codey_event_call,
  .make_new = codey_event_make_new,
  .locals_dict = (mp_obj_t)&codey_event_locals_dict,
};
