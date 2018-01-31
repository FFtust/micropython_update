/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     Super varible python object
 * @file      codey_super_var.c
 * @author    leo
 * @version   V1.0.0
 * @date      2017/09/13
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
 *  leo             2017/09/13      1.0.0                 build the new.
 *  leo             2017/10/19      1.0.1                 1) Fix a bug in super_var. A object return by mp_obj_new_xxx()
 *                                                        may been free in python, but a C program can NOT be awar of 
 *                                                        that when useing a object which is nolonger exist.
 *  leo             2017/10/20      1.0.2                 1) Fix a bug: more than one thread accessing the same super variable
 *                                                        need to protect
 *  leo             2017/12/27      1.0.3                 1) Reuse INT data, do NOT change INT to FLOAT anymore.
                                                          2) Add super var clear interface, after python restart
 * </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "py/mpstate.h"
#include "py/runtime.h"
#include "esp_log.h"
#include "py/nlr.h"
#include "py/obj.h"
#include "py/objstr.h"

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_task.h"
#include "driver/uart.h"
#include "codey_utils.h"
#include "codey_ble_sys_dev_func.h"

#include "codey_super_var.h"
#include "py/stackctrl.h"
#include "codey_comm_protocol.h"


/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/ 
#undef    TAG
#define   TAG                           ("super_var")

#define   UART                          (UART_NUM_0)
#define   FRAME_HEADER                  (0xF2)
#define   FRAME_END                     (0xF5)
#define   TX_BUF_SIZE                   (256)

#define   SUPER_VAR_TAKE_LIST_SEM()     do\
                                        {\
                                          xSemaphoreTake(s_super_var_list_sem, portMAX_DELAY);\
                                          /* ESP_LOGI(TAG, "SUPER_VAR_LIST_TAKE");*/\
                                        } while(0)
#define   SUPER_VAR_GIVE_LIST_SEM()     do\
                                        {\
                                          /* ESP_LOGI(TAG, "SUPER_VAR_LIST_GIVE");*/\
                                          xSemaphoreGive(s_super_var_list_sem);\
                                        } while(0)
#define   SUPER_VAR_TAKE_SEM(s)       do\
                                        {\
                                          xSemaphoreTake(s->sem, portMAX_DELAY);\
                                          /*ESP_LOGI(TAG, "SUPER_VAR_TAKE");*/\
                                        } while(0)
#define   SUPER_VAR_GIVE_SEM(s)       do\
                                        {\
                                          /*ESP_LOGI(TAG, "SUPER_VAR_GIVE");*/\
                                          xSemaphoreGive(s->sem);\
                                        } while(0)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/  
typedef enum
{
  SUPER_VAR_NONE_T = 0,
  SUPER_VAR_CHAR_T,
  SUPER_VAR_UN_CHAR_T,
  SUPER_VAR_SHORT_T,
  SUPER_VAR_UN_SHORT_T,
  SUPER_VAR_INT_T,
  SUPER_VAR_UN_INT_T,
  SUPER_VAR_LL_T,
  SUPER_VAR_UN_LL_T,
  SUPER_VAR_FLOAT_T,
  SUPER_VAR_DOUBLE_T,
  SUPER_VAR_BOOL_T,
  SUPER_VAR_STR_T,
  SUPER_VAR_LIST_T,
  
  SUPER_VAR_NUM,
}super_var_type_t;

enum
{
  HEAD_S = 0,
  LEN_S,
  DATA_S,
  CHECK_S,
  END_S,
};

typedef struct
{
  super_var_type_t  type;
  uint8_t           size;
  char *            desc;
}super_var_type_desc_t;

typedef struct codey_super_var_obj
{
  mp_obj_base_t base;
  char *name;
  super_var_type_t type;
  uint8_t value_len;
  uint8_t *value;
  SemaphoreHandle_t sem;
  struct codey_super_var_obj *next;
}codey_super_var_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATAS
 ******************************************************************************/
static bool s_super_var_module_init = false;
static SemaphoreHandle_t s_super_var_list_sem;
static codey_super_var_obj_t *s_super_var_header = NULL;
static uint8_t s_tx_buf[TX_BUF_SIZE];
const mp_obj_type_t codey_super_var_type;
const super_var_type_desc_t super_var_type_desc_tab[] = 
{
  { SUPER_VAR_CHAR_T,         1,        "char" },
  { SUPER_VAR_UN_CHAR_T,      1,        "un char" },
  { SUPER_VAR_SHORT_T,        2,        "short" },
  { SUPER_VAR_UN_SHORT_T,     2,        "un short" },
  { SUPER_VAR_INT_T,          4,        "int" },
  { SUPER_VAR_UN_INT_T,       4,        "un int" },  
  { SUPER_VAR_LL_T,           8,        "long long" }, 
  { SUPER_VAR_UN_LL_T,        8,        "un long long" }, 
  { SUPER_VAR_FLOAT_T,        4,        "float" }, 
  { SUPER_VAR_DOUBLE_T,       8,        "double" }, 
  { SUPER_VAR_BOOL_T,         1,        "bool" }, 
  { SUPER_VAR_STR_T,          0,        "string" }
};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
static codey_super_var_obj_t *codey_super_var_create(const char *name);
static void codey_super_var_frame_parse(uint8_t *data, uint8_t len);
static void codey_super_var_update_value(codey_super_var_obj_t *super_var, super_var_type_t val_type, const uint8_t *val_data, uint8_t value_len);
static void codey_super_var_add_to_list(codey_super_var_obj_t *super_var);
static codey_super_var_obj_t *codey_super_var_find_by_name(const char *super_var_name);
static void codey_super_var_build_data(codey_super_var_obj_t *super_var, uint8_t *buf, uint32_t *out_len);
static void codey_super_var_send_value(codey_super_var_obj_t *super_var);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_super_var_init(void)
{
  if(!s_super_var_module_init)
  {
    // ESP_LOGI(TAG, "codey_super_var_init");
    s_super_var_list_sem = xSemaphoreCreateCounting(1, 1);
    s_super_var_module_init = true;
  }
}
 
void codey_super_var(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  codey_super_var_init();
  
  if(!output_buf || !output_len)
  {
    return;
  }

  codey_super_var_frame_parse(data, len);
  *output_len = 0;
}

void codey_super_var_clear( void )
{
  codey_super_var_obj_t *node;

  codey_super_var_init();
  SUPER_VAR_TAKE_LIST_SEM();  
  if(s_super_var_header)
  {
    node = s_super_var_header;
    while(node)
    {
      SUPER_VAR_TAKE_SEM(node);
      memset(node->value, 0, node->value_len);
      SUPER_VAR_GIVE_SEM(node);
      node = node->next;      
    }
  }
  SUPER_VAR_GIVE_LIST_SEM();
}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
static codey_super_var_obj_t *codey_super_var_create(const char *name)
{
  codey_super_var_obj_t *super_var;

  super_var = malloc(sizeof(codey_super_var_obj_t));
  if(super_var)
  {
    super_var->name = malloc(strlen(name)+1);
    strcpy(super_var->name, name);
    super_var->base.type = &codey_super_var_type;
    super_var->type = SUPER_VAR_NONE_T;
    super_var->value = NULL;
    super_var->value_len = 0;
    super_var->sem = xSemaphoreCreateCounting(1, 1);
    super_var->next = NULL;
  }
  
  return super_var;
}
 
static void codey_super_var_frame_parse(uint8_t *data, uint8_t len)
{  
  codey_super_var_obj_t *super_var;
  size_t super_var_name_len, super_var_value_len;
  super_var_type_t super_var_type;
  uint8_t *super_var_name;
  uint8_t *super_var_value;

  super_var_name = data;
  super_var_name_len = strlen((const char *)super_var_name);
  if(super_var_name_len > 0xff)
  {
    return;
  }

  if(len <= (super_var_name_len + 2))
  {
    return;
  }

  super_var_value_len = len - (super_var_name_len + 2);
  super_var_type = data[super_var_name_len + 1];
  super_var_value = data + super_var_name_len + 2;  

  super_var = codey_super_var_find_by_name((const char *)super_var_name);
  if(!super_var)
  {
    super_var = codey_super_var_create((const char *)super_var_name);
    if(super_var)
    {
      codey_super_var_update_value(super_var, super_var_type, super_var_value, super_var_value_len);
      codey_super_var_add_to_list(super_var);
    }
    else
    {
      return;
    }
  }
  else
  {
    // ESP_LOGI(TAG, "Already has this super var: %s, just update it value", (const char *)super_var_name);
    codey_super_var_update_value(super_var, super_var_type, super_var_value, super_var_value_len);
  }
}

static void codey_super_var_build_data(codey_super_var_obj_t *super_var, uint8_t *buf, uint32_t *out_len)
{
  uint32_t idx = 0; 

  SUPER_VAR_TAKE_SEM(super_var);
  // 1) name  
  strcpy((char *)(buf + idx), super_var->name);
  idx += strlen(super_var->name);
  buf[idx++] = '\0';

  // 2) type
  buf[idx++] = super_var->type;

  // 3) value
  memcpy(buf + idx, super_var->value, super_var->value_len);
  idx += super_var->value_len;
  SUPER_VAR_GIVE_SEM(super_var);

  *out_len = idx;
}

static void codey_super_var_send_value(codey_super_var_obj_t *super_var)
{  
  uint32_t data_len;

  codey_super_var_build_data(super_var, s_tx_buf, &data_len);
  codey_comm_build_frame(SUPER_VAR_ID, s_tx_buf, &data_len);
  uart_write_bytes(UART, (const char *)s_tx_buf, data_len);
  codey_ble_dev_put_data(s_tx_buf, data_len);  
}

static void codey_super_var_update_value(codey_super_var_obj_t *super_var, super_var_type_t val_type, const uint8_t *val_data, uint8_t value_len)
{
  SUPER_VAR_TAKE_SEM(super_var);
  super_var->type = val_type;
  if(super_var->value)
  {
    free(super_var->value);
  }
  super_var->value = malloc(value_len);
  super_var->value_len = value_len;
  memcpy(super_var->value, val_data, value_len);
  SUPER_VAR_GIVE_SEM(super_var);
}

static void codey_super_var_add_to_list(codey_super_var_obj_t *super_var)
{
  codey_super_var_obj_t *node;

  if(!super_var)
  {
    return;
  }

  SUPER_VAR_TAKE_LIST_SEM();  
  if(!s_super_var_header)
  {
    s_super_var_header = super_var;
  }
  else
  {
    node = s_super_var_header;
    while(node->next)
    {
      node = node->next;
    }
    node->next = super_var;
  }
  SUPER_VAR_GIVE_LIST_SEM();
}

static codey_super_var_obj_t *codey_super_var_find_by_name(const char *super_var_name)
{
  codey_super_var_obj_t *node;

  if(!super_var_name)
  {
    return NULL;
  }

  SUPER_VAR_TAKE_LIST_SEM();
  node = s_super_var_header;
  while(node)
  {
    if(0 == strcmp(node->name, super_var_name))
    {
      break;
    }
    else
    {
      node = node->next;
    }
  }
  SUPER_VAR_GIVE_LIST_SEM();

  return node;
}

STATIC mp_obj_t codey_super_var_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  const char *name;
  codey_super_var_obj_t *self;
  const char *name_string;
  size_t name_string_len;
  int init_value;

  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  codey_super_var_init();

  self = mp_const_none;
  if(1 == n_args && MP_OBJ_IS_STR_OR_BYTES(all_args[0]))
  {
    name = mp_obj_str_get_str(all_args[0]);
    self = codey_super_var_find_by_name(name);
    if(self)
    {
      return self;
    }
    else
    {
      name_string = mp_obj_str_get_data(all_args[0], &name_string_len);
      self = codey_super_var_create(name_string);
      init_value = 0;
      codey_super_var_update_value(self, SUPER_VAR_INT_T, (const uint8_t *)(&init_value), sizeof(init_value));
      codey_super_var_add_to_list(self);
    }
  }

  return self;
}

STATIC mp_obj_t codey_super_var_set_value(mp_obj_t self_in, mp_obj_t new_value)
{
  mp_obj_t ret;
  uint8_t bool_val;
  float float_val;
  int int_val;
  const char *str_val;
  size_t str_val_len;
  codey_super_var_obj_t *self = self_in;

  codey_super_var_init();
  
  if(new_value == mp_const_false || new_value == mp_const_true)
  {
    bool_val = (new_value == mp_const_true)?1:0;
    codey_super_var_update_value(self, SUPER_VAR_BOOL_T, (const uint8_t *)(&bool_val), sizeof(bool_val));
    ret = mp_const_true;
    codey_super_var_send_value(self);
  }
  else if(MP_OBJ_IS_INT(new_value))
  {
    int_val = mp_obj_get_int(new_value);
    codey_super_var_update_value(self, SUPER_VAR_INT_T, (const uint8_t *)(&int_val), sizeof(int));
    ret = mp_const_true;
    codey_super_var_send_value(self);
  }
  else if(MP_OBJ_IS_STR(new_value))
  {
    str_val = mp_obj_str_get_data(new_value, &str_val_len);
    codey_super_var_update_value(self, SUPER_VAR_STR_T, (const uint8_t *)str_val, str_val_len);
    ret = mp_const_true;
    codey_super_var_send_value(self);
  }
  else if(mp_obj_is_float(new_value))
  {
    float_val = mp_obj_get_float(new_value);
    codey_super_var_update_value(self, SUPER_VAR_FLOAT_T, (const uint8_t *)(&float_val), sizeof(float_val));
    ret = mp_const_true;
    codey_super_var_send_value(self);
  }
  else
  {
    ret = mp_const_false;
  }

  return ret;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_super_var_set_value_obj, codey_super_var_set_value);

STATIC mp_obj_t codey_super_var_get_value(mp_obj_t self_in)
{
  mp_obj_t ret;
  codey_super_var_obj_t *self = self_in;

  codey_super_var_init();

  ret = mp_const_true;
  SUPER_VAR_TAKE_SEM(self);
  switch(self->type)
  {
    case SUPER_VAR_NONE_T:
    break;
  
    case  SUPER_VAR_CHAR_T:
    break;
  
    case  SUPER_VAR_UN_CHAR_T:
    break;
  
    case  SUPER_VAR_SHORT_T:
    break;
  
    case  SUPER_VAR_UN_SHORT_T:
    break;
  
    case  SUPER_VAR_INT_T:
      ret = mp_obj_new_int(*(int *)(self->value));
    break;
  
    case  SUPER_VAR_UN_INT_T:
    break;
  
    case  SUPER_VAR_LL_T:
    break;
  
    case  SUPER_VAR_UN_LL_T:
    break;
  
    case  SUPER_VAR_FLOAT_T:
      ret = mp_obj_new_float(*(float *)(self->value));
    break;
  
    case  SUPER_VAR_DOUBLE_T:
    break;
  
    case  SUPER_VAR_BOOL_T:
      ret = (self->value[0])?mp_const_true:mp_const_false;
    break;
  
    case  SUPER_VAR_STR_T:
      ret = mp_obj_new_str((const char *)(self->value), self->value_len, false);
    break;

    case SUPER_VAR_LIST_T:
    break;
  
    case SUPER_VAR_NUM:
    break;
  }
  SUPER_VAR_GIVE_SEM(self);

  return ret;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_super_var_get_value_obj, codey_super_var_get_value);

STATIC mp_obj_t codey_super_var_get_name(mp_obj_t self_in)
{
  codey_super_var_obj_t *self = self_in;

  codey_super_var_init();
  
  return self->name;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_super_var_get_name_obj, codey_super_var_get_name);

STATIC const mp_map_elem_t codey_super_var_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_value),               (mp_obj_t)&codey_super_var_set_value_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_value),               (mp_obj_t)&codey_super_var_get_value_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_name),                (mp_obj_t)&codey_super_var_get_name_obj }
};
STATIC MP_DEFINE_CONST_DICT(codey_super_var_locals_dict, codey_super_var_locals_dict_table);

const mp_obj_type_t codey_super_var_type =
{
  { &mp_type_type },
  .name = MP_QSTR_super_var,
  .make_new = codey_super_var_new,
  .locals_dict = (mp_obj_t)&codey_super_var_locals_dict,
};
