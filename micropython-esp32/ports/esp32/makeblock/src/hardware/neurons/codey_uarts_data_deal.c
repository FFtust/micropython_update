/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock UART module
 * @file    codey_uarts_data_deal.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/08/17
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
 * This file is a drive for makeblock UART module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/08/17      1.0.0              build the new.
 * </pre>
 *
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"

#include "codey_uarts_data_deal.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define UART0_BUFFER_RECEIVE_SIZE (512)
#define UART1_BUFFER_RECEIVE_SIZE (1024)

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
static bool s_uart0_opened = false;
static bool s_uart1_opened = false;
static SemaphoreHandle_t s_recv_data_sem = NULL; 

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_uart0_init(uint32_t baud_rate)
{
  if(s_uart0_opened == false)
  {
    int uart_num = UART_NUM_0;
    s_uart0_opened = true;
    uart_config_t uart_config =
    {
      .baud_rate = baud_rate,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 64,
    };
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, -1, -1, -1, -1); //fftust:for uART0,the pin is not allowed to change,so give -1 to set the default pins
    uart_driver_install(uart_num, UART0_BUFFER_RECEIVE_SIZE * 2, 0, 0, NULL, 0);
  }
}

void codey_uart1_init(uint32_t baud_rate)
{
  if(s_uart1_opened == false)
  {
    int uart_num = UART_NUM_1;
    s_uart1_opened = true;
    uart_config_t uart_config =
    {
      .baud_rate = baud_rate,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 64,
    };
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, 17, 16, -1, -1);//23 22
    uart_driver_install(uart_num, UART1_BUFFER_RECEIVE_SIZE * 2, 0, 0, NULL, 0);
  }
}

void codey_uart_change_baud_rate(uart_port_t port, uint32_t new_rate)
{
  uart_driver_delete(port);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  if(port == UART_NUM_1)
  {
    s_uart1_opened = false;
    codey_uart1_init(new_rate);
  }
  else if(port == UART_NUM_0)
  {
    s_uart0_opened = false;
    codey_uart0_init(new_rate); 
  }
  else
  {
    ;
  }
}

void codey_neurons_uart_rec_start(void)
{  
  codey_give_data_recv_sem_init();
  codey_uart0_init(115200);
  codey_uart1_init(115200);
}

int codey_uart0_buffer_get_char(uint8_t *c)
{
  int ret = uart_read_bytes(UART_NUM_0, c, 1, 1);
  return ret;  /* 0 indicate no data */
}

int codey_uart0_send_chars(uint8_t *buffer, uint16_t size)
{
  int ret = uart_write_bytes(UART_NUM_0, (const char *)buffer, size);
  return ret;  
}

int codey_uart1_buffer_get_char(uint8_t *c)
{
  int ret = uart_read_bytes(UART_NUM_1, c, 1, 1);
  return ret;    /* 0 indicate no data */
}

int codey_uart1_send_chars(uint8_t *buffer, uint16_t size)
{
  int ret = uart_write_bytes(UART_NUM_1, (const char *)buffer, size);
  return ret;  
}

void codey_give_data_recv_sem(void)
{
  xSemaphoreGive(s_recv_data_sem);
}

bool codey_take_data_recv_sem(uint32_t t_ms)
{
  if(xSemaphoreTake(s_recv_data_sem, t_ms / portTICK_PERIOD_MS) == pdTRUE)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void codey_give_data_recv_sem_from_isr(void)
{
  portBASE_TYPE HPTaskAwoken = pdFALSE;
  xSemaphoreGiveFromISR(s_recv_data_sem, &HPTaskAwoken);
}

void codey_give_data_recv_sem_init(void)
{
  s_recv_data_sem = xSemaphoreCreateCounting(1, 0);
}


