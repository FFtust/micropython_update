/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Utility for common use
 * @file    codey_utils.c
 * @author  Leo
 * @version V1.0.0
 * @date    2017/06/09
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
 * This file include some system function.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *   Leo lu           2017/06/00      1.0.0            build the new.
 * </pre>
 *
 */
 
#include <stdint.h>
#include <stdio.h>
#include "py/nlr.h"
#include "py/obj.h"
#include "py/binary.h"
#include "py/mpstate.h"
#include "py/runtime.h"
#include "objarray.h"
#include "codey_utils.h"

/*****************************************************************
 DEFINE MACROS
******************************************************************/

/*****************************************************************
 DEFINE TYPES & CONSTANTS
******************************************************************/
const uint8_t s_hex_table[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

/*****************************************************************
DEFINE PRIVATE DATAS
******************************************************************/
static union
{
  uint8_t   byteVal[4];
  float     floatVal;
  long      longVal;
}s_val;

static union
{
  uint8_t   byteVal[2];
  int16_t   shortVal;
}s_valShort;

/*****************************************************************
 DECLARE PRIVATE FUNCTIONS
******************************************************************/

/*****************************************************************
 DEFINE PUBLIC FUNCTIONS
******************************************************************/
void codey_print_hex(uint8_t *data, uint32_t len)
{
  uint32_t i;
  
  for(i = 0; i < len; i++)
  {
    printf("%02x ", data[i]);
    fflush(stdout);
  }
}

void codey_to_hex_str(uint8_t *in_data, uint16_t len, uint8_t *out_data)
{
  uint16_t i;
  for(i = 0; i < len; i++)
  {
    out_data[2 * i] = s_hex_table[in_data[i] >> 4 & 0x0F];
    out_data[2 * i + 1] = s_hex_table[in_data[i] & 0x0F];
  }
}

bool codey_get_data_from_int_list_obj(mp_obj_list_t *mp_list, uint8_t *pbuf)
{
  uint32_t index;

  if(!mp_list || !pbuf)
  {
    return false;
  }

  for(index = 0; index < mp_list->len; index++)
  {
    pbuf[index] = (uint8_t)mp_obj_get_int(((mp_obj_t *)(mp_list->items))[index]);
  }

  return true;
}

mp_obj_t codey_build_int_list_obj(uint8_t *data, uint32_t len)
{
  uint32_t index;
  mp_obj_t obj;
  mp_obj_list_t *list;

  if(!data || !len)
  {
    return NULL;
  }

  list = mp_obj_new_list(0, NULL);
  if(!list)
  {
    return NULL;
  }
  
  for(index = 0; index < len; index++)
  {
    obj = mp_obj_new_int(data[index]);
    if(obj)
    {
      mp_obj_list_append(list, obj);
    }
    else
    {
      return NULL;
    }
  }

  return list;
}

void codey_ble_print_attr(esp_attr_desc_t *attr)
{
  if(!attr)
  {
    return;
  }

  /*
  ESP_LOGI(TAG, "\r\n******************\r\n");
  ESP_LOGI(TAG, "uuid_len: %d\n", attr->uuid_length);
  ESP_LOGI(TAG, "uuid_p:");
  codey_print_hex(attr->uuid_p, attr->uuid_length);
  ESP_LOGI(TAG, "\n");
  ESP_LOGI(TAG, "perm: %d\n", attr->perm);
  ESP_LOGI(TAG, "max_length: %d\n", attr->max_length);
  ESP_LOGI(TAG, "length: %d\n", attr->length);
  ESP_LOGI(TAG, "value: ");
  codey_print_hex(attr->value, attr->length);
  ESP_LOGI(TAG, "\n");
  ESP_LOGI(TAG, "******************\r\n");
  */
}

void codey_ble_free_gatt_table(esp_gatts_attr_db_t *gatt_table, size_t size)
{
  uint32_t free_index;
  esp_attr_desc_t *attr;
  
  if(gatt_table)
  {
    for(free_index = 0; free_index < size; free_index++)
    {
      attr = &(gatt_table[free_index].att_desc);
      if(attr->uuid_p)
      {
        vPortFree(attr->uuid_p);
      }
      if(attr->value)
      {
        vPortFree(attr->value);
      }
    }
    vPortFree(gatt_table);
  }
}

bool codey_ble_compare_attr(esp_attr_desc_t *a, esp_attr_desc_t *b)
{
  if(a->uuid_length == b->uuid_length)
  {
    if(0 == memcmp(a->uuid_p, b->uuid_p, a->uuid_length))
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
    return false;
  }
}

bool codey_ble_compare_uuid(esp_bt_uuid_t *a, esp_bt_uuid_t *b)
{
  if(a->len == b->len)
  {
    if(0 == memcmp((uint8_t *)(&(a->uuid)), (uint8_t *)(&(b->uuid)), a->len))
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
    return false;
  }
}

void codey_ble_create_srv_uuid_from_mp_attr(mp_obj_list_t *attr_in, esp_bt_uuid_t *uuid_out)
{
  size_t index;
  mp_obj_list_t *srv_uuid_list;

  srv_uuid_list = attr_in->items[5];
  uuid_out->len = srv_uuid_list->len;

  for(index = 0; index < srv_uuid_list->len; index++)
  {
    ((uint8_t *)(&uuid_out->uuid))[index] = mp_obj_get_int(srv_uuid_list->items[index]);
  }
}

void codey_ble_print_uuid(esp_bt_uuid_t *uuid)
{
  printf("UUID: ");
  codey_print_hex((uint8_t *) & (uuid->uuid), uuid->len);
  printf("\n");
}

uint8_t *codey_get_path_from_full_file_name(uint8_t *full_file_name)
{
  size_t check_idx;
  uint8_t *path;

  /*
  A full file name:  "/flash/music/cat.wav"
  It's path is: "/flash/music"
  */

  path = NULL;
  check_idx = strlen((char *)full_file_name) - 1;
  while(check_idx >= 0)
  {
    if(full_file_name[check_idx] == '/')
    {
      // string len = idx + 1; so nead buffer is string len + ternimal = idx + 1 + 1 
      path = (uint8_t *)malloc(check_idx + 1 + 1);
      if(path)
      {
        memcpy(path, full_file_name, check_idx);
        path[check_idx] = 0;
      }
      break;
    }
    else
    {
      check_idx--;
    }
  }

  return path;
}

uint8_t *codey_get_name_from_full_file_name(uint8_t *full_file_name)
{
  size_t check_idx;
  size_t len;
  uint8_t *name;

  /*
  A full file name:  "/flash/music/cat.wav"
  It's name is: "cat.wav"
  */

  name = NULL;
  len = strlen((char *)full_file_name);
  check_idx = len - 1;
  while(check_idx >= 0)
  {
    if(full_file_name[check_idx] == '/')
    {
      // string len = len - idx - 1;  so nead buffer is string len + ternimal =  len - idx - 1 + 1
      name = (uint8_t *)malloc(len - check_idx);
      if(name)
      {
        memcpy(name, full_file_name + check_idx + 1, len - check_idx - 1);
        name[len - check_idx - 1] = 0;
      }
      break;
    }
    else
    {
      check_idx--;
    }
  }

  return name;
}

uint8_t codey_read_buffer(uint8_t *data_buffer, int32_t index)
{
  return *(data_buffer + index);
}

int16_t codey_read_short(uint8_t *data_buffer, int32_t idx)
{
  s_valShort.byteVal[0] = codey_read_buffer(data_buffer, idx);
  s_valShort.byteVal[1] = codey_read_buffer(data_buffer, idx + 1);
  return s_valShort.shortVal; 
}

long codey_read_long(uint8_t *data_buffer, int32_t idx)
{
  s_val.byteVal[0] = codey_read_buffer(data_buffer, idx);
  s_val.byteVal[1] = codey_read_buffer(data_buffer, idx + 1);
  s_val.byteVal[2] = codey_read_buffer(data_buffer, idx + 2);
  s_val.byteVal[3] = codey_read_buffer(data_buffer, idx + 3);
  return s_val.longVal;
}

float codey_read_float(uint8_t *data_buffer, int32_t idx)
{
  s_val.byteVal[0] = codey_read_buffer(data_buffer, idx);
  s_val.byteVal[1] = codey_read_buffer(data_buffer, idx + 1);
  s_val.byteVal[2] = codey_read_buffer(data_buffer, idx + 2);
  s_val.byteVal[3] = codey_read_buffer(data_buffer, idx + 3);
  return s_val.floatVal;
}

int lcm(int a, int b)
{
  int max = (a >= b ? a : b);
  int min = (a < b ? a : b);
  int i;
  
  for(i = 1; ; ++i)
  {
    if((max * i) % min == 0)
    {
      return (max * i);
    }
  }
}

uint8_t codey_calc_add_check_sum(uint8_t *data, uint32_t len)
{
  uint32_t i;
  uint8_t check_sum;

  check_sum = 0;
  for(i = 0; i < len; i++)
  {
    check_sum += data[i];
  }

  return check_sum;
}

// End of file
/*******************************************************************/

