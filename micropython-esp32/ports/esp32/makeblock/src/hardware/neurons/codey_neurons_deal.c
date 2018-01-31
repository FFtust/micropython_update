/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock codey communication module
 * @file    codey_communition.c
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
 * This file is a drive for neurons module.
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

#include "py/mphal.h"
#include "py/mpstate.h"

#include "driver/uart.h"

#include "esp_log.h"
#include "codey_ringbuf.h"
#include "codey_ble_sys_dev_func.h"
#include "codey_uarts_data_deal.h"
#include "codey_esp32_resouce_manager.h"

#include "codey_neurons_universal_protocol.h"
#include "codey_neurons_ftp.h"
#include "codey_gyro_board.h"
#include "codey_neurons_deal.h"
#include "codey_sys.h"
#include "codey_battery_check.h"

#include "codey_rgbled_board.h"
#include "codey_utils.h" 
#include "neurons_engine_list_maintain.h"
#include "neurons_engine_port.h"
#include "codey_comm_protocol.h"
#include "codey_config.h"
#include "codey_sys_operation.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define   TAG                   ("codey_communication")
#define   MAX_RSP_LEN           (256 + 128)
#define   MAIN_PY_SCRIPT_NAME   ("/flash/main.py")
#define   MAX_DEAL_TIME_MS      (50)

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
static uint32_t s_rsp_len;
static uint8_t  s_rsp_buf[MAX_RSP_LEN];
static char s_rx_file_name[MAX_NEU_FTP_FILE_NAME_LEN];
static neurons_command_frame_t s_neurons_command_codey_car_frame;

/******************************************************************************
 DEFINE PUBLIC DATAS
 ******************************************************************************/
neurons_command_frame_t  g_neurons_command_codey_local_frame; // f0 xx 5E ... F7

/******************************************************************************
 DECLARE PRIVATE FUNCTION
 ******************************************************************************/
STATIC void codey_neurons_init_t();
STATIC void neurons_uart1_command_package_parse(neurons_commmd_package_t *package, uint8_t peripheral);
STATIC void codey_neurons_uart1_command_package_put(neurons_commmd_package_t *package, uint8_t *c);

/******************************************************************************
 DECLARE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_neurons_deal(void * parameter)
{
  bool uart0_more_data;
  bool uart1_more_data;
  bool ble_more_data;
  uint32_t deal_start_tick;
  
  while(!codey_task_get_enable_flag_t(CODEY_UART_DEAL_TASK_ID))
  {
    vTaskDelay(10);
  }
  
  codey_neurons_init_t(); 
  neurons_engine_command_buffer_init();

  neurons_commmd_package_t neurons_ble_commmd_rec_package;
  neurons_command_package_reset(&neurons_ble_commmd_rec_package);

  neurons_commmd_package_t neurons_uart0_commmd_rec_package;
  neurons_command_package_reset(&neurons_uart0_commmd_rec_package);

  neurons_commmd_package_t neurons_uart1_commmd_rec_package;
  neurons_command_package_reset(&neurons_uart1_commmd_rec_package);

  uint8_t c;
  neurons_comnad_type_t cmd_type;
  while(1)
  {  
    if(codey_take_data_recv_sem(MAX_DEAL_TIME_MS))
    { 
      ble_more_data = true;
      uart0_more_data = true;
      uart1_more_data = true;

      while(ble_more_data || uart0_more_data || uart1_more_data)
      {
        //---------- BLE channel ------------//
        deal_start_tick = millis();
        while((millis() - deal_start_tick) < MAX_DEAL_TIME_MS)
        {
          if(0 == codey_ble_dev_get_char(&c))
          {
            codey_comm_protocol_pump_char(BLE_DATA_TAG, c, s_rsp_buf, &s_rsp_len);
            if(s_rsp_len)
            {
              codey_ble_dev_put_data(s_rsp_buf, s_rsp_len);
            }
          }
          else
          {
            ble_more_data = false;
            break;
          }
        }

        //---------- UART0 channel ------------//
        deal_start_tick = millis();
        while((millis() - deal_start_tick) < MAX_DEAL_TIME_MS) 
        {
          if(1 == codey_uart0_buffer_get_char(&c))
          {
            codey_comm_protocol_pump_char(UART0_DATA_TAG, c, s_rsp_buf, &s_rsp_len);
            if(s_rsp_len)
            {
              codey_uart0_send_chars(s_rsp_buf, s_rsp_len);
            }
          }
          else
          {
            uart0_more_data = false;
            break;
          }
        }

        //---------- UART1 channel ------------//
        if(codey_get_main_py_exec_status_t() != CODEY_MAIN_PY_UPDATE)
        {
          deal_start_tick = millis();
          while((millis() - deal_start_tick) < MAX_DEAL_TIME_MS)
          {
            if(1 == codey_uart1_buffer_get_char(&c))
            {
              if(codey_get_main_py_exec_status_t() == CODEY_MAIN_PY_UPDATE)
              {
                break;
              }
              codey_neurons_uart1_command_package_put(&neurons_uart1_commmd_rec_package, &c);
            }
            else 
            {
              uart1_more_data = false;
              break;
            }
          }
        
          if(neurons_engine_get_lib_found_flag_t())
          {
            while(neurons_engine_command_buffer_pop(&cmd_type))
            {
              neurons_engine_send_single_command_t(&cmd_type);  
              /* if we use while, this delay should not be used */
              // vTaskDelay(20 / portTICK_PERIOD_MS);
            } 
            /* we can use this function to send bytes data directly */
            extern uint8_t neurons_engine_get_special_command_bytes_t(uint8_t *, uint8_t *);
            uint8_t spe_cmd_buff[20] = {0};
            uint8_t spe_cmd_len = 0;
            if(neurons_engine_get_special_command_bytes_t(spe_cmd_buff, &spe_cmd_len))
            {
              codey_uart1_send_chars(spe_cmd_buff, spe_cmd_len);
            }
          }
          
          /* we can use this function to send bytes data directly */
          extern uint8_t neurons_engine_get_special_command_bytes_t(uint8_t *, uint8_t *);
          uint8_t spe_cmd_buff[20] = {0};
          uint8_t spe_cmd_len = 0;
          if(neurons_engine_get_special_command_bytes_t(spe_cmd_buff, &spe_cmd_len))
          {
            codey_uart1_send_chars(spe_cmd_buff, spe_cmd_len);
          }
        }
        else
        {
          uart_flush(UART_NUM_1);
          while(neurons_engine_command_buffer_pop(&cmd_type));
        }
        codey_neurons_get_recv_file_name(s_rx_file_name);
        if(0 == strcmp("NEU_FTY_READY", s_rx_file_name))
        {
          /* here stop rocky */
          neurons_engine_rocky_stop();
          codey_set_main_py_exec_status_t(CODEY_MAIN_PY_UPDATE);
          ESP_LOGE(TAG, "start receive new script");
          printf("start receive new script\n");
        }  
        else if(0 == strcmp(MAIN_PY_SCRIPT_NAME, s_rx_file_name))
        {
          ESP_LOGE(TAG, "received new script and set restart flag");
          printf("received new script and set restart flag\n");
          set_user_script_interrupt_reason(2);
          sys_set_restart_flag(true);
        }
      }
    }

    //------------ other routing -------------
    if(codey_get_main_py_exec_status_t() != CODEY_MAIN_PY_UPDATE)
    {
      if(neurons_engine_get_lib_found_flag_t())
      {
        neurons_engine_send_heart_package_t();
      }
    }
  }
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC void codey_neurons_init_t()
{
  /*                                                              F0            dev_id    ser_id                 no sub_ser   f7       */
  neurons_frame_struct_init_t(&g_neurons_command_codey_local_frame, START_ESP32_ONLINE, 0x00, CLASS_ESP32_SENSORS, 0x00, END_ESP32_ONLINE);
  /*                                                              F0            dev_id    ser_id          sub_ser             f7       */
  neurons_frame_struct_init_t(&s_neurons_command_codey_car_frame, START_ESP32_ONLINE, 0xff, CLASS_SENSOR, BLOCK_CODEY_CAR , END_ESP32_ONLINE);
}

/* UART1 receive the respond data,different from UART0 and blue tooth */
/* we need pass the righe package to neurons engine */
STATIC void neurons_uart1_command_package_parse(neurons_commmd_package_t *package, uint8_t peripheral)
{
  neurons_engine_parse_package_t(package);
}

STATIC void codey_neurons_uart1_command_package_put(neurons_commmd_package_t *package, uint8_t *c)
{
  neurons_command_package_put_t(package, c, START_ESP32_ONLINE, END_ESP32_ONLINE, neurons_uart1_command_package_parse, CODEY_UART1_ID, 2); 
}
