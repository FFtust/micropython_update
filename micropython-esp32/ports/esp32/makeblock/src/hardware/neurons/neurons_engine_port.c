/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for neuron engine port module
 * @file    neurons_engine_port.c
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
#include "esp_heap_caps.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/gpio.h"

#include "codey_utils.h"
#include "extmod/vfs_fat.h"
#include "codey_sys.h"
#include "codey_esp32_resouce_manager.h"
#include "codey_config.h"

#include "neurons_engine_port.h"
#include "neurons_engine_lib_struct.h"
#include "neurons_engine_list_maintain.h"
#include "codey_neurons_universal_protocol.h"
#include "codey_neurons_deal.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG                            ("NEURONS_ENGINE_PYTHON")
#define NEU_ENGINE_READ_WAIT_TICK_MAX  (200)
#define NEURONS_ENGINE_READ_LOCK       neurons_engine_read_enter_t();
#define NEURONS_ENGINE_READ_UNLOCK     neurons_engine_read_exit_t();

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/  
typedef struct
{
  mp_obj_base_t base;
}neuron_engine_port_obj_t;

typedef struct
{
  uint8_t dev_id;
  uint8_t block_id;
  uint8_t sub_id;
  uint8_t read_cmd_id;
}neurons_engine_read_cmd_t;

typedef struct
{
  SemaphoreHandle_t read_sema;
  SemaphoreHandle_t read_sema_mutex;
  neurons_engine_read_cmd_t neurons_engine_read_cmd;
  uint32_t wait_ticks;
}neurons_engine_read_control_t;

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
static neuron_engine_port_obj_t s_neuron_engine_port_obj = {.base = {&neurons_engine_port_type}};
static uint8_t s_special_command_buffer[20] = {0};
static uint8_t s_special_command_bytes_num = 0;
static neurons_engine_read_control_t s_neurons_engine_read_control;
static bool s_neuron_engine_port_init_flag = false;

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void neurons_engine_read_control_init_t(void)
{
  if(!s_neuron_engine_port_init_flag)
  {
    s_neurons_engine_read_control.read_sema = xSemaphoreCreateBinary();
    s_neurons_engine_read_control.read_sema_mutex = xSemaphoreCreateBinary();
    s_neurons_engine_read_control.wait_ticks = NEU_ENGINE_READ_WAIT_TICK_MAX;
    s_neurons_engine_read_control.neurons_engine_read_cmd.dev_id = 0;
    s_neurons_engine_read_control.neurons_engine_read_cmd.block_id = 0;
    s_neurons_engine_read_control.neurons_engine_read_cmd.sub_id = 0;
    s_neurons_engine_read_control.neurons_engine_read_cmd.read_cmd_id = 0;
    
    s_neuron_engine_port_init_flag = true;
  }
}

void neurons_engine_read_control_deinit_t(void)
{
  if(s_neuron_engine_port_init_flag)
  {
    xSemaphoreGive(s_neurons_engine_read_control.read_sema);
    xSemaphoreGive(s_neurons_engine_read_control.read_sema_mutex);
    s_neurons_engine_read_control.neurons_engine_read_cmd.dev_id = 0;
    s_neurons_engine_read_control.neurons_engine_read_cmd.block_id = 0;
    s_neurons_engine_read_control.neurons_engine_read_cmd.sub_id = 0;
    s_neurons_engine_read_control.neurons_engine_read_cmd.read_cmd_id = 0;
  }
}

void neurons_engine_read_control_set_cmd_t(uint8_t dev_id, uint8_t block_id, uint8_t sub_id, uint8_t read_cmd_id)
{
  // printf("***dev id: %d, type: %d, sub type: %d, respond id %d\n", dev_id, block_id, sub_id, read_cmd_id);
  s_neurons_engine_read_control.neurons_engine_read_cmd.dev_id = dev_id;
  s_neurons_engine_read_control.neurons_engine_read_cmd.block_id = block_id;
  s_neurons_engine_read_control.neurons_engine_read_cmd.sub_id = sub_id;
  s_neurons_engine_read_control.neurons_engine_read_cmd.read_cmd_id = read_cmd_id;
}

void neurons_engine_read_reset_wait_status_t()
{ 
  xSemaphoreTake(s_neurons_engine_read_control.read_sema, 0);
}

bool neurons_engine_read_wait_respond_t()
{
  if(xSemaphoreTake(s_neurons_engine_read_control.read_sema, NEU_ENGINE_READ_WAIT_TICK_MAX / portTICK_PERIOD_MS) == pdTRUE)
  {
    return true; 
  }
  else
  {
    return false;
  }
}

/* the following two fuctions must be called together */
void neurons_engine_read_enter_t()
{
  MP_THREAD_GIL_EXIT();
  xSemaphoreTake(s_neurons_engine_read_control.read_sema_mutex, portMAX_DELAY);
  MP_THREAD_GIL_ENTER();
}

void neurons_engine_read_exit_t()
{
  MP_THREAD_GIL_EXIT();
  xSemaphoreGive(s_neurons_engine_read_control.read_sema_mutex);
  MP_THREAD_GIL_ENTER();
}

/* this function should be called in neurons package parsing */
/* asynchronous */
void neurons_engine_read_check_t(uint8_t dev_id, uint8_t block_id, uint8_t sub_id, uint8_t read_cmd_id)
{
  // printf("dev id: %d, type: %d, sub type: %d, respond id %d\n", dev_id, block_id, sub_id, read_cmd_id);
  if(s_neurons_engine_read_control.neurons_engine_read_cmd.dev_id == dev_id
     && s_neurons_engine_read_control.neurons_engine_read_cmd.sub_id == sub_id
     && s_neurons_engine_read_control.neurons_engine_read_cmd.read_cmd_id == read_cmd_id
     && s_neurons_engine_read_control.neurons_engine_read_cmd.block_id == block_id
  	 )
  {
    xSemaphoreGive(s_neurons_engine_read_control.read_sema);
  }
}

uint8_t neurons_engine_get_special_command_bytes_t(uint8_t *buff, uint8_t *len)
{
  if(s_special_command_bytes_num == 0)
  {
    return 0;
  }
  else
  {
    memcpy((void *)buff, (const void *)s_special_command_buffer, s_special_command_bytes_num);
    (*len) = s_special_command_bytes_num;
    s_special_command_bytes_num = 0;
    return 1;
  }
}

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
STATIC mp_obj_t neurons_engine_port_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
  
  // setup the object
  neuron_engine_port_obj_t *self = &s_neuron_engine_port_obj;
  self->base.type = &neurons_engine_port_type;
  
  return self;
}

STATIC mp_obj_t neurons_engine_port_init(mp_obj_t self_in)
{
  neurons_engine_read_control_init_t();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(neurons_engine_port_init_obj, neurons_engine_port_init);

STATIC mp_obj_t neurons_engine_port_deinit(mp_obj_t self_in)
{
  neurons_engine_read_control_deinit_t();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(neurons_engine_port_deinit_obj, neurons_engine_port_deinit);

STATIC mp_obj_t neurons_engine_send_special_command(mp_uint_t n_args, const mp_obj_t *args)
{
  uint8_t cmd_num = n_args - 1;
  if(cmd_num <= 0)
  {
    return mp_const_none;
  }
  s_special_command_bytes_num = cmd_num;
  for(uint8_t i = 0; i < cmd_num; i++)
  {
    s_special_command_buffer[i] = mp_obj_get_int(args[i + 1]);
  }
  codey_give_data_recv_sem();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(neurons_engine_send_special_command_obj, 3, 19, neurons_engine_send_special_command);


STATIC mp_obj_t neurons_engine_port_send_command(mp_uint_t n_args, const mp_obj_t *args)
{
  /* type, [sub_type], index, [cmd_id, parameter, para2, ...] */
  if(!neurons_engine_get_lib_found_flag_t())
  {
    ESP_LOGE(TAG, "neurons engine lib not found");
    return mp_const_none;
  }
  size_t list_args_len;
  mp_obj_t *list_args_items;
  
  uint8_t type = mp_obj_get_int(args[1]);
  uint8_t sub_type = mp_obj_get_int(args[2]);
  uint8_t index = mp_obj_get_int(args[3]);
  uint8_t cmd_id = 0;

  mp_obj_get_array(args[4], &list_args_len, &list_args_items);
  cmd_id = mp_obj_get_int(list_args_items[0]);
  neurons_comnad_type_t neurons_comnad_type;
  /* if less parameter in, set the default value to zero */
  memset(&neurons_comnad_type, 0, sizeof(neurons_comnad_type));
  neurons_comnad_type.type_id = type;
  neurons_comnad_type.sub_type_id = sub_type;
  neurons_comnad_type.index = index;
  neurons_comnad_type.cmd_id = cmd_id;

  list_args_len = (list_args_len > CMD_DATA_BUFFER_LEN) ? CMD_DATA_BUFFER_LEN : list_args_len;
  neurons_comnad_type.cmd_data_len = list_args_len;
  for(uint8_t i = 1; i < list_args_len; i++)
  {
    neurons_comnad_type.cmd_data[i - 1] = mp_obj_get_float(list_args_items[i]);
  }
  MP_THREAD_GIL_EXIT();
  neurons_engine_command_buffer_push(&neurons_comnad_type);
  MP_THREAD_GIL_ENTER();
  codey_give_data_recv_sem();
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(neurons_engine_port_send_command_obj, 3, 19, neurons_engine_port_send_command);

STATIC mp_obj_t neurons_engine_port_read_data(mp_uint_t n_args, const mp_obj_t *args)
{
  if(!neurons_engine_get_lib_found_flag_t())
  {
    ESP_LOGE(TAG, "neurons engine lib not found");
    return mp_const_none;
  }

  size_t list_args_len;
  mp_obj_t *list_args_items;
    
  uint8_t type = mp_obj_get_int(args[1]);
  uint8_t sub_type = mp_obj_get_int(args[2]);
  uint8_t index = mp_obj_get_int(args[3]);
  uint8_t respond_id = 0;
  
  mp_obj_get_array(args[4], &list_args_len, &list_args_items);
  respond_id = mp_obj_get_int(list_args_items[0]);
  ESP_LOGD(TAG, "respond id is %d", respond_id);

#if 0
  for(uint8_t i = 0; i < list_args_len; i++)
  {
    ESP_LOGI(TAG, "list data %d is %d\n", i, mp_obj_get_int(list_args_items[i]));
  }
#endif

  uint8_t respond_type_len = 0;
  uint8_t respond_data_type[RES_RESPOND_TYPE_MAX];  
  uint8_t respond_data_offset = 0;
  neurons_engine_online_block_info_t *list_node_info = NULL;

  list_node_info = neurons_engine_get_main_list_block_info_by_index_t(index, type, sub_type);
  ESP_LOGD(TAG, "list node respond buffer addr is %p", list_node_info);
  if(list_node_info == NULL)
  {
    ESP_LOGE(TAG, "not found block: index is %d, type is%d, sub is %d",index, type, sub_type);    
    return mp_const_none;;
  }  
  neurons_engine_get_respond_info_t(list_node_info, respond_id, &respond_data_offset, respond_data_type, &respond_type_len);
  ESP_LOGD(TAG, "respond_data_offset is %d", respond_data_offset);

  if(respond_type_len == 0)
  {
    ESP_LOGE(TAG, "respond type valid , return");
    return mp_const_none;
  }
  ESP_LOGD(TAG, "respond type len is %d", respond_type_len);
  uint8_t list_num = 0;
     
  for(uint8_t u = 1; u < respond_type_len; u++)
  {
    /* special type */
    if((respond_data_type[u] & 0xf0) == ALTERABLE_NUM_BYTE)
    { 
      ESP_LOGD(TAG, "special respond buffer num %d, u is %d", mp_obj_get_int(list_args_items[u - 1]), u - 1);
      list_num += mp_obj_get_int(list_args_items[u - 1]);
      /* the next data is not type */
      u++;
    }
    else if((respond_data_type[u] & 0xf0) != 0)
    { 
      list_num += ((respond_data_type[u] & 0xf0) >> 4) ; 
    }
    else
    {
      list_num += 1;
    }
    ESP_LOGD(TAG, "list num is  %d", list_num);
  }

  /* wait the respond */
  neurons_engine_read_reset_wait_status_t();
  NEURONS_ENGINE_READ_LOCK
  neurons_engine_read_control_set_cmd_t(list_node_info->dev_id, type, sub_type, respond_id);
    
  /* send the read command first, and wait for the respond */
  neurons_comnad_type_t neurons_comnad_type;
  /* if less parameter in, set the default value to zero */
  memset(&neurons_comnad_type, 0, sizeof(neurons_comnad_type));
  neurons_comnad_type.type_id = type;
  neurons_comnad_type.sub_type_id = sub_type;
  neurons_comnad_type.index = index;
  neurons_comnad_type.cmd_id = respond_id;

  list_args_len = (list_args_len > CMD_DATA_BUFFER_LEN) ? CMD_DATA_BUFFER_LEN : list_args_len;
  neurons_comnad_type.cmd_data_len = list_args_len;
  for(uint8_t i = 1; i < list_args_len; i++)
  {
    neurons_comnad_type.cmd_data[i - 1] = mp_obj_get_float(list_args_items[i]);
  }
  MP_THREAD_GIL_EXIT();
  neurons_engine_command_buffer_push(&neurons_comnad_type);
  MP_THREAD_GIL_ENTER();
  codey_give_data_recv_sem();

  if(neurons_engine_read_wait_respond_t() == false)
  {
    NEURONS_ENGINE_READ_UNLOCK
    return mp_const_none; 
  }
  
  mp_obj_list_t *newlist = mp_obj_new_list(list_num, NULL);
  float data_value = 0.0;
  uint8_t respond_data_index = 0;
  uint8_t type_data_index = 0;
  for(uint8_t j = 1; j < respond_type_len; j++)
  {  
    uint8_t type_number = 0;
    /* special data type dealing */
    if((respond_data_type[j] & 0xf0) == ALTERABLE_NUM_BYTE)
    {
      /* the first type can't be  ALTERABLE_NUM_BYTE */
      if(j == 1)
      {
        ESP_LOGE(TAG, "the first byte can't be ALTERABLE_NUM_BYTE");
        return mp_const_none;
      }
      type_number = mp_obj_get_int(list_args_items[j - 1]);
      ESP_LOGD(TAG, "the number of alterable type is %d, type is %d", type_number, (respond_data_type[j] & 0x0f));
    }
    else
    {
      ESP_LOGD(TAG, "special type  %d", (respond_data_type[j] & 0xf0));
      type_number = ((respond_data_type[j] & 0xf0) >> 4);
      type_number = (type_number == 0 ? 1 : type_number);
    }
    
    if(list_node_info->respond_data_info.respond_data_buffer == NULL)
    {
      NEURONS_ENGINE_READ_UNLOCK
      return mp_const_none;
    }
    
    switch((respond_data_type[j] & 0x0f))
    {
      case BYTE_8_T:  
        for(uint8_t i = 0; i < type_number; i++)
        {
          data_value = byte_8_to_value((uint8_t *)(list_node_info->respond_data_info.respond_data_buffer + 
                                        respond_data_offset + respond_data_index));
          respond_data_index += BYTE_8_LEN;
          newlist->items[type_data_index++] = mp_obj_new_int(data_value);
        }
      break;
      
      case BYTE_16_T: 
        for(uint8_t i = 0; i < type_number; i++)
        {
          data_value = byte_16_to_value((uint8_t *)(list_node_info->respond_data_info.respond_data_buffer + 
                                        respond_data_offset + respond_data_index));
          respond_data_index += BYTE_16_LEN;
          newlist->items[type_data_index++] = mp_obj_new_int(data_value);
        }
      break;
      
      case SHORT_16_T:
        for(uint8_t i = 0; i < type_number; i++)
        {
          data_value = short_16_to_value((uint8_t *)(list_node_info->respond_data_info.respond_data_buffer + 
                                        respond_data_offset + respond_data_index));
          respond_data_index += SHORT_16_LEN;
          newlist->items[type_data_index++] = mp_obj_new_int(data_value);
        }
      break;
      case SHORT_24_T: 
        for(uint8_t i = 0; i < type_number; i++)
        {
          data_value = short_24_to_value((uint8_t *)(list_node_info->respond_data_info.respond_data_buffer + 
                                        respond_data_offset + respond_data_index));
          respond_data_index += SHORT_24_LEN;
          newlist->items[type_data_index++] = mp_obj_new_int(data_value);
        }
      break;
      case LONG_40_T: 
        for(uint8_t i = 0; i < type_number; i++)
        {
          data_value = long_40_to_value((uint8_t *)(list_node_info->respond_data_info.respond_data_buffer + 
                                        respond_data_offset + respond_data_index));
          respond_data_index += LONG_40_LEN;
          newlist->items[type_data_index++] = mp_obj_new_int(data_value);
        }
      break;
      case FLOAT_40_T:
        for(uint8_t i = 0; i < type_number; i++)
        {
          data_value = float_40_to_value((uint8_t *)(list_node_info->respond_data_info.respond_data_buffer + 
                                        respond_data_offset + respond_data_index));
          respond_data_index += FLOAT_40_LEN;
          newlist->items[type_data_index++] = mp_obj_new_float(data_value);
        }
      break;
      default:
        ESP_LOGE(TAG, "not recognize this type");    
      break;
    }
  }
  NEURONS_ENGINE_READ_UNLOCK
  	
  return (mp_obj_t)(newlist);  
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(neurons_engine_port_read_data_obj, 3, 19, neurons_engine_port_read_data);

STATIC void neurons_engine_port_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC mp_obj_t neurons_engine_port_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  
  return mp_const_none;
  
}

STATIC const mp_map_elem_t neurons_engine_port_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_init),               (mp_obj_t)&neurons_engine_port_init_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),             (mp_obj_t)&neurons_engine_port_deinit_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_send),               (mp_obj_t)&neurons_engine_port_send_command_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_read),               (mp_obj_t)&neurons_engine_port_read_data_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_send_special),       (mp_obj_t)&neurons_engine_send_special_command_obj },
};

STATIC MP_DEFINE_CONST_DICT(neurons_engine_port_locals_dict, neurons_engine_port_locals_dict_table);

const mp_obj_type_t neurons_engine_port_type = 
{
  { &mp_type_type },
  .name = MP_QSTR_neurons_engine,
  .print = neurons_engine_port_print,
  .call = neurons_engine_port_call,
  .make_new = neurons_engine_port_make_new,
  .locals_dict = (mp_obj_t)&neurons_engine_port_locals_dict,
};
