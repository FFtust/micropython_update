/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     Baudrate setting
 * @file      codey_firewarem.c
 * @author    Leo
 * @version V1.0.0
 * @date      2017/12/15
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
 *   Leo              2017/12/15      1.0.0            build the new.
 * </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "codey_sys.h"
#include "codey_firmware.h"
#include "neurons_engine_list_maintain.h"


/********************************************************************
 DEFINE MACROS
 ********************************************************************/
#undef  TAG
#define TAG  ("BD_SETTING")


/********************************************************************
 DEFINE PRIVATE DATAS
 ********************************************************************/
typedef enum
{
  BD_NULL = 0,
  BD_9600 = 1,
  BD_19200 = 2,
  BD_38400 = 3,
  BD_57600 = 4,    
  BD_115200 = 5,
  BD_256000 = 6,
  BD_512000 = 7,
  BD_921600 = 8,
} baudrate_e;

typedef enum
{
  BD_READ = 0x00,
  BD_SET = 0x01,
  BD_ACK = 0x81,
} baudrate_cmd_e;

typedef enum
{
  BD_OK = 0x00,
  BD_ERR = 0x01,
} baudrate_cmd_ack_e;


/********************************************************************
 DEFINE PUBLIC DATAS
 ********************************************************************/
baudrate_e codey_comm_baudrate_to_enum(uint32_t baudrate);
uint32_t codey_comm_enum_to_baudrate(baudrate_e bd_e);
bool codey_comm_set_uart_baudrate(uart_port_t uart_num, baudrate_e bd_e);
baudrate_e codey_comm_get_uart_baudrate(uart_port_t uart_num);


/********************************************************************
 DEFINE PUBLIC FUNCTIONS
 ********************************************************************/
void codey_comm_baudrate_setting(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  switch(data[0])
  {
    case BD_READ:
    if(2 == len)
    {
      output_buf[0] = data[0];
      output_buf[1] = data[1];
      output_buf[2] = codey_comm_get_uart_baudrate(data[1]);
      ESP_LOGD(TAG, "port: %u", data[1]);
      ESP_LOGD(TAG, "bdrate: %u", codey_comm_enum_to_baudrate(output_buf[1]));
      *output_len = 3;
    }
    else
    {
      output_buf[0] = BD_ACK;
      output_buf[1] = BD_ERR;
      *output_len = 2;
    }
    break;
    
    case BD_SET:
    if(3 == len)
    {
      if(codey_comm_set_uart_baudrate(data[1], data[2]))
      {
        output_buf[0] = data[0];
        output_buf[1] = data[1];
        output_buf[2] = BD_OK;      
      }
      else
      {
        output_buf[0] = data[0];
        output_buf[1] = data[1];
        output_buf[2] = BD_ERR; 
      }
      *output_len = 3;
    }
    else
    {
      output_buf[0] = BD_ACK;
      output_buf[1] = BD_ERR;
      *output_len = 2;
    }
    break;
  }
}

baudrate_e codey_comm_baudrate_to_enum(uint32_t baudrate)
{
  if (9600-9600/50 <= baudrate && baudrate <= 9600+9600/50)
  {
    return BD_9600;
  }
  else if(19200-19200/50 <= baudrate && baudrate <= 19200+19200/50)
  {
    return BD_19200;
  }
  else if(38400-38400/50 <= baudrate && baudrate <= 38400+38400/50)
  {
    return BD_38400;
  }
  else if(57600-57600/50 <= baudrate && baudrate <= 57600+57600/50)
  {
    return BD_57600;
  }
  else if(115200-115200/50 <= baudrate && baudrate <= 115200+115200/50)
  {
    return BD_115200;
  }
  else if(256000-256000/50 <= baudrate && baudrate <= 256000+256000/50)
  {
    return BD_256000;
  }
  else if(512000-512000/50 <= baudrate && baudrate <= 512000+512000/50)
  {
    return BD_512000;
  }
  else if(921000-921000/50 <= baudrate && baudrate <= 921000+921000/50)
  {
    return BD_921600;
  }
  else
  {
    return BD_NULL;
  }
}

uint32_t codey_comm_enum_to_baudrate(baudrate_e bd_e)
{
  if (BD_9600 == bd_e)
  {
    return 9600;
  }
  else if(BD_19200 == bd_e)
  {
    return 19200;
  }
  else if(BD_38400 == bd_e)
  {
    return 38400;
  }
  else if(BD_57600 == bd_e)
  {
    return 57600;
  }
  else if(BD_115200 == bd_e)
  {
    return 115200;
  }
  else if(BD_256000 == bd_e)
  {
    return 256000;
  }
  else if(BD_512000 == bd_e)
  {
    return 512000;
  }
  else if(BD_921600 == bd_e)
  {
    return 921600;
  }
  else
  {
    return 0;
  }
}

bool codey_comm_set_uart_baudrate(uart_port_t uart_num, baudrate_e bd_e)
{
  uint32_t baudrate;

  baudrate = codey_comm_enum_to_baudrate(bd_e);
  ESP_LOGD(TAG, "uart set baudrate to %u", baudrate);
  if(baudrate)
  {
    return ESP_OK==uart_set_baudrate(uart_num, baudrate);
  }
  else
  {
    return false;
  }
}

baudrate_e codey_comm_get_uart_baudrate(uart_port_t uart_num)
{
  uint32_t baudrate;
  
  uart_get_baudrate(uart_num, &baudrate);
  ESP_LOGD(TAG, "uart_get_baudrate() return %u", baudrate);
  return codey_comm_baudrate_to_enum(baudrate);
}



