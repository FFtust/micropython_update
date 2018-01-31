/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   codey sensor update
 * @file    codey_sensor_value_update.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/05/031
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
 * This file is a drive for sensor value update module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/05/31      1.0.0              build the new.
 * </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "py/mpstate.h"
#include "py/runtime.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"	

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/adc.h"
#include "esp_log.h"

#include "codey_sys.h"
#include "codey_h_class.h"
#include "codey_neurons_universal_protocol.h"

#include "codey_sys_operation.h"
#include "codey_h_class.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define   TAG                         ("SENSORS_UPDATE")
#define   AD_VALUE_TO_100             (100.0 / 4095)
#define   TIME_TO_MINUTE              (1.0 / 1000)
#define   ACC_OF_GRAVITY              (9.8)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
static uint32_t s_codey_timer_start = 0;

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
uint8_t codey_get_button_A_t(void)
{
  return codey_button_board_get_status_t(BUTTON1_IO);
}

uint8_t codey_get_button_B_t(void)
{
  return codey_button_board_get_status_t(BUTTON2_IO);
}

uint8_t codey_get_button_C_t(void)
{
  return codey_button_board_get_status_t(BUTTON3_IO);
}

float codey_get_timer_value_t(void)
{ 
  float t = 0;
  t = (millis() - s_codey_timer_start) / 1000.0;
  return t;
}

void codey_reset_timer_t(void)
{ 
  s_codey_timer_start = millis();
}

uint32_t codey_timer_value_t(void)
{
  return (millis() - s_codey_timer_start);
}

uint8_t codey_button_value_t(void)
{
  return codey_button_board_read_status_t();
}

float codey_light_sensor_value_t(void)
{
  return (float)(codey_light_sensor_board_read_value_t() * AD_VALUE_TO_100);
}

float codey_sound_sensor_value_t(void)
{
  return (float)(codey_sound_sensor_board_read_value_t() * AD_VALUE_TO_100);
}

float codey_gyro_sensor_value_t(uint8_t index)
{
  float const_gain = 1.0 / codey_gyro_board_get_static_acc_t();
  gyro_data_for_neurons_t gyro_data;
  gyro_data = codey_gyro_board_read_data_t();
 
  switch(index)
  {
    case 1:
      return gyro_data.angle_roll;
    break;
    case 2:
      return gyro_data.angle_pitch;
    break;
    case 3:
      return gyro_data.angle_yaw;
    break;
    case 4:
      {
        float temp = gyro_data.acc_x;
        temp = const_gain * temp;
        temp = (temp > 1.0) ? 1.0 : temp;
        temp = (temp < -1.0) ? -1.0 : temp;
        temp = temp * ACC_OF_GRAVITY;
        return temp;
      }
    break;
    case 5:
      {
        float temp = gyro_data.acc_y;
        temp = const_gain * temp;
        temp = (temp > 1.0) ? 1.0 : temp;
        temp = (temp < -1.0) ? -1.0 : temp;
        temp = temp * ACC_OF_GRAVITY;
        return temp;
      }
    break;
    case 6:
      {
        float temp = gyro_data.acc_z;
        temp = const_gain * temp;
        temp = (temp > 1.0) ? 1.0 : temp;
        temp = (temp < -1.0) ? -1.0 : temp;
        temp = temp * ACC_OF_GRAVITY;
        return temp;
      }
    break;
    default:
      return 0;
  }
}

float codey_dail_sensor_value_t(void)
{
  return (float)(codey_potentionmeter_board_read_value_t() * AD_VALUE_TO_100);
}
 
void codey_sensors_init_t(void)
{
  codey_battery_check_config_t();
  /* as soon as power on, set the GPIO15 to high level */ 
  codey_board_power_on_set_t();
 
  codey_button_board_config_t();
  codey_sound_sensor_board_config_t();
  codey_light_sensor_board_config_t();
  codey_potentionmeter_board_config_t();
  codey_rgbled_board_config_t();
  codey_8bit_voice_board_config_t();
  codey_ledmatrix_initialize_t();
  codey_rmt_board_tx_init_t();
  codey_rmt_board_rx_init_t();

  codey_gyro_board_init_t();
}

void codey_sensors_update_t(void)
{
  codey_button_board_get_all_status_t();
  codey_sound_sensor_board_get_value_t();
  codey_light_sensor_board_get_value_t();
  codey_potentionmeter_board_get_value_t();
  codey_battery_check_get_value_t();
  /* rmt and gyro is special */
  codey_gyro_board_get_data_t();
  /* when get rmt data here, the I2c error occured */
  /* the temporary solution is to create a task to receive rmt data */
}

void codey_sensors_update_task_t(void *parameter)
{
  ESP_LOGI(TAG, "get value start execute");
  while(1)
  {  
    codey_task_set_status_t(CODEY_SENSORS_GET_VALUE_TASK_ID, CODEY_TASK_WAIT_TO_EXECUTE); 
    if(codey_task_get_enable_flag_t(CODEY_SENSORS_GET_VALUE_TASK_ID))
    {
      /* this func will not be blocked */ 
      codey_task_set_status_t(CODEY_SENSORS_GET_VALUE_TASK_ID, CODEY_TASK_EXECUTING);
      codey_sensors_update_t();
    }
    else
    {
      codey_task_set_status_t(CODEY_SENSORS_GET_VALUE_TASK_ID, CODEY_TASK_WAIT_TO_EXECUTE);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

