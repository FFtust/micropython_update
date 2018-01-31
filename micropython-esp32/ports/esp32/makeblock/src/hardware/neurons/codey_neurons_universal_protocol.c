/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock neurons protocol module
 * @file    codey_neurons_universal_protocol.c
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
 * This file is a drive for neurons protocol module. Just as a universal protocol parsing file.
 * Not bingding to any peripheral.
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/08/14      1.0.0              build the new.
 * </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "esp_log.h"
#include "codey_ringbuf.h"

#include "driver/uart.h"

#include "codey_ble_sys_dev_func.h"
#include "codey_uarts_data_deal.h"
#include "codey_neurons_universal_protocol.h"
#include "codey_config.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG                     ("neu_protocol")
#define FIRMWARE_VERSION        (001)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
static void neurons_read_value_by_type_t(neurons_commmd_package_t *package, uint8_t cmd_data_index,
                                         neurons_data_type_t cmd_data_type, void *value);
static bool neurons_package_check_sum_t(neurons_commmd_package_t *package);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
/* data type transform start */ 
void value_to_neurons_byte_8(void *val, uint8_t *byte_out, uint16_t *byte_out_size) // 0 - 128
{
  byte_out[0] = *(uint8_t *)(val);
  (*byte_out_size) = 1;
}
 
void value_to_neurons_byte_16(void *val, uint8_t *byte_out, uint16_t *byte_out_size) // -128 - 128  
{
  uint8_t val_7bit[2] = {0};
  val_1_byte val1byte;
  
  val1byte.char_val = *(int8_t*)val;
  val_7bit[0] = val1byte.byte_val[0] & 0x7f;
  val_7bit[1] = (val1byte.byte_val[0] >> 7) & 0x7f;
   
  memcpy(byte_out, val_7bit, 2);
  (*byte_out_size) = 2;
}

void value_to_neurons_short_16(void *val, uint8_t *byte_out, uint16_t *byte_out_size) // 0 - 32768
{
  uint8_t val_7bit[2]={0};
  val_2_byte val2byte;
  
  val2byte.short_val = *(uint16_t *)val;
  val_7bit[0] = val2byte.byte_val[0] & 0x7f;
  val_7bit[1] = ((val2byte.byte_val[1] << 1) | (val2byte.byte_val[0] >> 7)) & 0x7f;
    
  memcpy(byte_out, val_7bit, 2);
  (*byte_out_size) = 2;
}

void value_to_neurons_short_24(void *val, uint8_t *byte_out, uint16_t *byte_out_size) // -32768 - 32768 
{
  uint8_t val_7bit[3]={0};
  val_2_byte val2byte;

  val2byte.short_val = *(int16_t *)val;
  val_7bit[0] = val2byte.byte_val[0] & 0x7f;
  val_7bit[1] = ((val2byte.byte_val[1] << 1) | (val2byte.byte_val[0] >> 7)) & 0x7f;
  val_7bit[2] = (val2byte.byte_val[1] >> 6) & 0x7f;

  memcpy(byte_out, val_7bit, 3);
  (*byte_out_size) = 3;
}

void value_to_neurons_long_40(void *val, uint8_t *byte_out, uint16_t *byte_out_size) 
{
  uint8_t val_7bit[5]={0};
  val_4_byte val4byte;

  val4byte.long_val = *(long *)val;
  val_7bit[0] = val4byte.byte_val[0] & 0x7f;
  val_7bit[1] = ((val4byte.byte_val[1] << 1) | (val4byte.byte_val[0] >> 7)) & 0x7f;
  val_7bit[2] = ((val4byte.byte_val[2] << 2) | (val4byte.byte_val[1] >> 6)) & 0x7f;
  val_7bit[3] = ((val4byte.byte_val[3] << 3) | (val4byte.byte_val[2] >> 5)) & 0x7f;
  val_7bit[4] = (val4byte.byte_val[3] >> 4) & 0x7f;

  memcpy(byte_out, val_7bit, 5);
  (*byte_out_size) = 5;
}

void value_to_neurons_float_40(void *val, uint8_t *byte_out, uint16_t *byte_out_size)
{
  uint8_t val_7bit[5]={0};
  val_4_byte val4byte;

  val4byte.float_val = *(float *)val;
  val_7bit[0] = val4byte.byte_val[0] & 0x7f;
  val_7bit[1] = ((val4byte.byte_val[1] << 1) | (val4byte.byte_val[0] >> 7)) & 0x7f;
  val_7bit[2] = ((val4byte.byte_val[2] << 2) | (val4byte.byte_val[1] >> 6)) & 0x7f;
  val_7bit[3] = ((val4byte.byte_val[3] << 3) | (val4byte.byte_val[2] >> 5)) & 0x7f;
  val_7bit[4] = (val4byte.byte_val[3] >> 4) & 0x7f;

  memcpy(byte_out, val_7bit, 5);
  (*byte_out_size) = 5;
}

uint8_t byte_8_to_value(uint8_t *value_bytes)//fftust: 1 bytes
{
  val_1_byte val1byte;
  
  val1byte.byte_val[0] = value_bytes[0] & 0x7f;
  return val1byte.char_val;
}

int8_t byte_16_to_value(uint8_t *value_bytes)//fftust: 2 bytes
{
  uint8_t temp;
  val_1_byte val1byte;

  val1byte.byte_val[0] = value_bytes[0] & 0x7f;
  temp = value_bytes[1] << 7;
  val1byte.byte_val[0] |= temp;
  return (int8_t)(val1byte.char_val);
}

uint16_t short_16_to_value(uint8_t *value_bytes) // 2  bytes
{
  uint8_t temp;
  val_2_byte val2byte;

  val2byte.byte_val[0] = value_bytes[0] & 0x7f;
  temp = value_bytes[1] << 7;
  val2byte.byte_val[0] |= temp;

  val2byte.byte_val[1] = (value_bytes[1] >> 1) & 0x7f;

  return (uint16_t)(val2byte.short_val);
}

short short_24_to_value(uint8_t *value_bytes) // 3 bytes
{
  uint8_t temp;
  val_2_byte val2byte;

  val2byte.byte_val[0] = value_bytes[0] & 0x7f;
  temp = value_bytes[1] << 7;
  val2byte.byte_val[0] |= temp;
  val2byte.byte_val[1] = (value_bytes[1] >> 1) & 0x7f;

  temp = (value_bytes[2] << 6);
  val2byte.byte_val[1] |= temp;

  return val2byte.short_val;
}

float float_40_to_value(uint8_t *value_bytes) // 5 bytes 
{
  uint8_t temp;
  val_4_byte val4byte;

  val4byte.byte_val[0] = value_bytes[0] & 0x7f;
  temp = value_bytes[1] << 7;
  val4byte.byte_val[0] |= temp;

  val4byte.byte_val[1] =  (value_bytes[1] >> 1) & 0x7f;
  temp = (value_bytes[2] << 6);
  val4byte.byte_val[1] += temp;

  val4byte.byte_val[2] =  (value_bytes[2] >> 2) & 0x7f;
  temp = (value_bytes[3] << 5);
  val4byte.byte_val[2] += temp;

  val4byte.byte_val[3] =  (value_bytes[3] >> 3) & 0x7f;
  temp = (value_bytes[4] << 4);
  val4byte.byte_val[3] += temp;

  return val4byte.float_val;
}

long long_40_to_value(uint8_t *value_bytes) // 5 bytes
{
  uint8_t temp;
  val_4_byte val4byte;

  val4byte.byte_val[0] = value_bytes[0] & 0x7f;
  temp = value_bytes[1] << 7;
  val4byte.byte_val[0] |= temp;

  val4byte.byte_val[1] =  (value_bytes[1] >> 1) & 0x7f;
  temp = (value_bytes[2] << 6);
  val4byte.byte_val[1] += temp;

  val4byte.byte_val[2] =  (value_bytes[2] >> 2) & 0x7f;
  temp = (value_bytes[3] << 5);
  val4byte.byte_val[2] += temp;

  val4byte.byte_val[3] =  (value_bytes[3] >> 3) & 0x7f;
  temp = (value_bytes[4] << 4);
  val4byte.byte_val[3] += temp;

  return val4byte.long_val;
}
/* data type transform end */ 

/* universal function */
/* if there is not sub_sr_id, then set it to 0 */
void neurons_frame_struct_init_t(neurons_command_frame_t *cmd_frame, uint8_t head, uint8_t dev_id,
                                 uint8_t ser_id, uint8_t s_ser_id, uint8_t end)
{
  cmd_frame->head = head;
  cmd_frame->device_id = dev_id;
  cmd_frame->service_id = ser_id;
  cmd_frame->sub_service_id = s_ser_id;
  cmd_frame->cmd_data = NULL;
  /* there is a data region between sub_id and end, it is different for different command */
  cmd_frame->end = end;
}

void neurons_frame_struct_set_by_type_t(neurons_command_frame_t *cmd_frame, neurons_frame_data_index_t index,
                                        uint8_t value)
{
 switch(index)
 {
   case NEURONS_FRAME_HEAD:
     cmd_frame->head = value;
   break;
   case NEURONS_FRAME_DEV_ID:
     cmd_frame->device_id = value;
   break;
   case NEURONS_FRAME_SER_ID:
     cmd_frame->service_id = value;
   break;
   case NEURONS_FRAME_SUB_SER_ID:
     cmd_frame->sub_service_id = value;
   break;
   case NEURONS_FRAME_END:
     cmd_frame->end = value;
   break;
   default:
   
   break;
 }
}

/* only NEURONS_FRAME_CMD_DATA will use the last two parameters, for other index, the last two parameter should be 0 & NULL*/
void neurons_buffer_add_data_t(neurons_commmd_package_t *package, neurons_data_type_t cmd_data_type, void *cmd_data)
{
  uint16_t len = 0;
  uint8_t byte_value[5] = {0};
  switch(cmd_data_type)
  {
    case BYTE_8_T:  
      {
        uint8_t data_s = *(uint8_t *)cmd_data ;
        value_to_neurons_byte_8(&data_s, byte_value, &len);
        for(uint8_t i = 0; i <len; i++)
        {
          package->cmd_package_info.data[package->data_in_index++] = byte_value[i];
        }
      }
    break;
    case BYTE_16_T: 
      {
        int8_t data_s = *(int8_t *)cmd_data ;
        value_to_neurons_byte_16(&data_s, byte_value, &len);
        for(uint8_t i = 0; i <len; i++)
        {
          package->cmd_package_info.data[package->data_in_index++] = byte_value[i];
        }
      }
    break;
    case SHORT_16_T:
      {
        uint16_t data_s = *(uint16_t *)cmd_data ;
        value_to_neurons_short_16(&data_s, byte_value, &len);
        for(uint8_t i = 0; i <len; i++)
        {
          package->cmd_package_info.data[package->data_in_index++] = byte_value[i];
        }
      }
    break;
    case SHORT_24_T: 
      {
        int16_t data_s = *(int16_t *)cmd_data ;
        value_to_neurons_short_24(&data_s, byte_value, &len);
        for(uint8_t i = 0; i <len; i++)
        {
          package->cmd_package_info.data[package->data_in_index++] = byte_value[i];
        }
      }
    break;
    case LONG_40_T: 
      {
        long data_s = *(long *)cmd_data ;
        value_to_neurons_long_40(&data_s, byte_value, &len);
        for(uint8_t i = 0; i <len; i++)
        {
          package->cmd_package_info.data[package->data_in_index++] = byte_value[i];
        }
      }
    break;
    case FLOAT_40_T:
      {
        float data_s = *(float *)cmd_data ;
        value_to_neurons_float_40(&data_s, byte_value, &len);
        for(uint8_t i = 0; i <len; i++)
        {
          package->cmd_package_info.data[package->data_in_index++] = byte_value[i];
        }
      }
    break;
    default:
        
    break;
 }

}

/* for a fixed neurons device, the head & dev_id & ser_id is fixed too. sub_ser_id is needed or not is not certain */ 
void neurons_buffer_add_head_frame_t(neurons_command_frame_t *cmd_frame, neurons_commmd_package_t *package, bool add_sub_ser_id)
{
  neurons_buffer_add_data_t(package, BYTE_8_T, &cmd_frame->head);
  neurons_buffer_add_data_t(package, BYTE_8_T, &cmd_frame->device_id);
  neurons_buffer_add_data_t(package, BYTE_8_T, &cmd_frame->service_id);
  /* if there is a sub_ser_id, add sub_ser_id */
  if(add_sub_ser_id == true)
  {
    neurons_buffer_add_data_t(package, BYTE_8_T, &cmd_frame->sub_service_id);
  }

}

void neurons_buffer_add_end_frame_t(neurons_command_frame_t *cmd_frame, neurons_commmd_package_t *package)
{
  uint8_t check_out = 0;
  for(uint8_t i = 1; i < package->data_in_index; i++)
  {
    check_out += package->cmd_package_info.data[i];
  }
  check_out  = check_out & 0x7f;
  neurons_buffer_add_data_t(package, BYTE_8_T, &check_out);
  neurons_buffer_add_data_t(package, BYTE_8_T, &cmd_frame->end);
  // ESP_LOGD("^^^^^", "checkout is %d", check_out);
  // ESP_LOGD("^^^^^", "end is is %d", cmd_frame->end);
  package->data_in_index = 0; // reset
}

void neurons_show_package_t(neurons_commmd_package_t *package)
{
  
  ESP_LOGD(TAG, "package length is %d\n", package->data_in_index);
  for(uint8_t i = 0; i < package->data_in_index; i++)
  {
    ESP_LOGD(TAG, "package datat %d  is %d\n", i, package->cmd_package_info.data[i]);
  }
}

void neurons_command_package_add_bytes_t(neurons_commmd_package_t *package, uint8_t *data, uint16_t size)
{
  uint8_t *p_temp = data;
  for(uint8_t i = 0; i < size; i++)
  {
    package->cmd_package_info.data[(package->data_in_index)++] = *(p_temp++);
  }

}

void neurons_command_package_reset(neurons_commmd_package_t *package)
{
  memset((void *)(package), 0, sizeof(neurons_commmd_package_t));
  
}

void neurons_command_package_put_t(neurons_commmd_package_t *package, uint8_t *c, uint8_t head, uint8_t end,
                                   neurons_command_parse_cb nc_cb, uint8_t peripheral, uint8_t protocol_index)
{
  static bool package_in_begin[10] = {false, false, false, false, false, false,false, false, false, false, false, false}; // most 10 neurons protocol
  if(protocol_index >= 10)
  {
    ESP_LOGE(TAG, "protocol index over");
    return;
  
  }
  
  if((*c) == head)
  {
    neurons_command_package_reset(package);
    package_in_begin[protocol_index] = true;
    neurons_command_package_add_bytes_t(package, c, 1);
  }
  else if(package_in_begin[protocol_index] == true)
  {
    neurons_command_package_add_bytes_t(package, c, 1);
    if((*c) == end)
    {
      package_in_begin[protocol_index] = false;
      if(neurons_package_check_sum_t(package))
      {
        nc_cb(package, peripheral); /* execute the call back function */
        // ESP_LOGD("neurons protocol", "received end\n");
      }

    }
  }
  else
  {
    ;
  }

}

void neurons_command_packege_send_all(neurons_commmd_package_t *package, uint8_t peripheral)
{
  uint16_t len = 0;
  for(len = 0; package->cmd_package_info.data[len] != END_ESP32_ONLINE; len++);
  
  if(peripheral == 1) /* uart 0 */
  {
    codey_uart0_send_chars(package->cmd_package_info.data, len + 1);
  }
  else if(peripheral == 2) /* uart 1 */
  {
    codey_uart1_send_chars(package->cmd_package_info.data, len + 1);
  }  
  else if(peripheral == 3) /* blue tooth */
  {
    codey_ble_dev_put_data(package->cmd_package_info.data, len + 1);
  }

}

/* only NEURONS_FRAME_CMD_DATA will use the last two parameters, for other index, the last two parameter should be 0 & NULL*/
void neurons_buffer_read_value_t(neurons_commmd_package_t *package, neurons_frame_data_index_t index,
                                 uint8_t cmd_data_index, neurons_data_type_t cmd_data_type, void *value)
{
  switch(index)
  {
    case NEURONS_FRAME_HEAD:
      *(uint8_t *)value = package->cmd_package_info.cmd_info.head;
    break;
    case NEURONS_FRAME_DEV_ID:
      *(uint8_t *)value = package->cmd_package_info.cmd_info.device_id;
    break;
    case NEURONS_FRAME_SER_ID:
      *(uint8_t *)value = package->cmd_package_info.cmd_info.service_id;
    break;
    case NEURONS_FRAME_SUB_SER_ID:
      *(uint8_t *)value = package->cmd_package_info.cmd_info.sub_service_id;
    break;
    case NEURONS_FRAME_CMD_DATA:
      neurons_read_value_by_type_t(package, cmd_data_index, cmd_data_type, value);
    break;
    case NEURONS_FRAME_END:
         
    break;
  }

}

/**************** common function *******************/
/* 1 for common command  2 for special block respond */
uint8_t neurons_engine_parse_respond_type_t(neurons_commmd_package_t *package)
{
  if(package->cmd_package_info.cmd_info.service_id >= CTL_SYSTEM_COMMAND_MIN
     && package->cmd_package_info.cmd_info.service_id <= CTL_SYSTEM_COMMAND_MAX)
  {
    return 1;
  }
  else if(package->cmd_package_info.cmd_info.service_id == CTL_GENERAL)
  {
    return 2;
  }
  else if(package->cmd_package_info.cmd_info.service_id >= CTL_GENERAL)
  {
    return 3;
  }
  else
  {
    return 0;
  }
}

uint8_t firmata_get_data_type_lenth_t(uint8_t data_type)
{
  switch(data_type)
  {
    case BYTE_8_T:
      return 1;
    break;
    case BYTE_16_T:
      return 2;
    break;
    case SHORT_16_T:
      return 2;
    break;
    case SHORT_24_T:
      return 3;
    break;
    case LONG_40_T:
      return 5;
    break;
    case FLOAT_40_T:
      return 5;
    break;
    case DOUBLE_72_T:
      return 9;
    break;
    default:
      return 0;
    break;
  }

}

void neurons_get_data_region_from_package_t(neurons_commmd_package_t *package, uint8_t *start_index, 
                                            uint8_t *length)
{
  uint8_t package_length = package->data_in_index;
  // ESP_LOGD(TAG, "package length is%d\n", package_length);
  /* if there is sub_type */
  *start_index = 5;
  *length = package_length - (*start_index) - 2;
  ESP_LOGD(TAG, "data region length is %d", *length);
  /* if there is no sub_type */
  // not deal now
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
static void neurons_read_value_by_type_t(neurons_commmd_package_t *package, uint8_t cmd_data_index,
                                         neurons_data_type_t cmd_data_type, void *value)
{
  switch(cmd_data_type)
  {
    case BYTE_8_T:  
      *((uint8_t *)value) = byte_8_to_value(&package->cmd_package_info.data[cmd_data_index]);
    break;
    case BYTE_16_T: 
      *((int8_t *)value) = byte_16_to_value(&package->cmd_package_info.data[cmd_data_index]);
    break;
    case SHORT_16_T:
      *((uint16_t *)value) = short_16_to_value(&package->cmd_package_info.data[cmd_data_index]);
      break;
    case SHORT_24_T: 
      *((int16_t *)value) = short_24_to_value(&package->cmd_package_info.data[cmd_data_index]);
      break;
    case LONG_40_T: 
      *((long *)value) = long_40_to_value(&package->cmd_package_info.data[cmd_data_index]);
      break;
    case FLOAT_40_T:
      *((float *)value) = float_40_to_value(&package->cmd_package_info.data[cmd_data_index]);
      break;
    default:
         
    break;
  }
}

static bool neurons_package_check_sum_t(neurons_commmd_package_t *package)
{
  uint8_t sum = 0;
  for(uint8_t i = 1; i < package->data_in_index - 2; i++)
  {
    sum += package->cmd_package_info.data[i];
  }
  sum = sum & 0x7f;
  if(sum == package->cmd_package_info.data[package->data_in_index - 2])
  {
    return true;
    // ESP_LOGD("neurons", "checkesum right\n");
  }
  else
  {
    ESP_LOGE("neurons", "*******checkesum wrong\n");
    return false;
  }

}


