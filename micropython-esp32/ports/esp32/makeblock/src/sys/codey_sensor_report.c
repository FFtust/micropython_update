/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief       Sensor report
 * @file        codey_sensor_report.c
 * @author      Leo lu
 * @version     V1.0.0
 * @date        2017/09/29
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
 * Leo lu             2017/10/10      1.0.0              Build the new.
 * </pre>
 *
 */
 
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "driver/uart.h"
#include "codey_utils.h"

#include "esp_task.h"
#include "esp_log.h"
#include "codey_neurons_ftp.h"
#include "codey_ble_sys_dev_func.h"
#include "codey_sensor_report.h"
#include "codey_comm_protocol.h"
#include "codey_sensor_value_update.h"
#include "codey_battery_check.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/ 
#undef    TAG
#define   TAG                           ("SENSOR_REPORT")

#define   UART                          (UART_NUM_0)
#define   SENSOR_REPORT_TX_BUF_SIZE     (16)
#define   SENSOR_NUM                    (11)
#define   REPORT_DATA_SIZE              (8)
#define   REPORT_MIN_DELAY_MS           (200)
#define   SENSOR_REPORT_TASK_STACK_LEN  (2048)
#define   SENSOR_REPORT_TASK_PRIORITY   (ESP_TASK_PRIO_MIN + 2)
#define   GET_TICK                      (xTaskGetTickCount() * (1000 / configTICK_RATE_HZ))
                                            
/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/  
typedef enum
{
  REQ_REPORT = 1,
  REQ_STOP,
  REPORT,
  RSP_OK,
  RSP_ERR,
}sensor_report_cmd_e;

typedef enum
{
  SOUND_ID = 1,
  LIGHT_ID,
  DAIL_ID,
  GYRO_X_DGR_ID,
  GYRO_Y_DGR_ID,
  GYRO_Z_DGR_ID,
  GYRO_X_G_ID,
  GYRO_Y_G_ID,
  GYRO_Z_G_ID,
  POWER_LEVLE,
  LOCAL_TIME_ID = 100,
}sensor_report_id_e;

typedef void (*sensor_get_data_pf)(uint8_t *output_data);

typedef struct
{
  bool        on_or_off;
  uint16_t    sensor_id;
  uint16_t    report_data_len;
  uint32_t    last_report_tick;
  uint8_t     last_report_data[REPORT_DATA_SIZE];
  uint8_t     cur_report_data[REPORT_DATA_SIZE];
  sensor_get_data_pf get_data;
}sensor_report_info_t;

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
static void codey_sensor_report_main_loop(void * parameter);
static void codey_sensor_report_update(void);
static void codey_sensor_report_to_host(void);
static void codey_sensor_report_send(sensor_report_info_t *sensor_report_info);
static bool codey_sensor_report_req(uint16_t sensor_id);
static bool codey_sensor_report_stop(uint16_t sensor_id);
static sensor_report_info_t *codey_sensor_report_find_info_by_id(uint16_t sensor_id);
static void codey_sensor_report_sound_get_data(uint8_t *output_data);
static void codey_sensor_report_light_get_data(uint8_t *output_data);
static void codey_sensor_report_dail_get_data(uint8_t *output_data);
static void codey_sensor_report_gyro_x_dgr_get_data(uint8_t *output_data);
static void codey_sensor_report_gyro_y_dgr_get_data(uint8_t *output_data);
static void codey_sensor_report_gyro_z_dgr_get_data(uint8_t *output_data);
static void codey_sensor_report_gyro_x_g_get_data(uint8_t *output_data);
static void codey_sensor_report_gyro_y_g_get_data(uint8_t *output_data);
static void codey_sensor_report_gyro_z_g_get_data(uint8_t *output_data);
static void codey_sensor_report_power_level_get_data(uint8_t *output_data);
static void codey_sensor_report_local_time_get_data(uint8_t *output_data);

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/  
static bool s_module_init = false;
static uint8_t s_sensor_report_tx_buf[SENSOR_REPORT_TX_BUF_SIZE];
static sensor_report_info_t s_sensor_report_info_tab[SENSOR_NUM] = 
{
  { false,    SOUND_ID,         sizeof(float),    0,    {0},    {0},   codey_sensor_report_sound_get_data },
  { false,    LIGHT_ID,         sizeof(float),    0,    {0},    {0},   codey_sensor_report_light_get_data },
  { false,    DAIL_ID,          sizeof(float),    0,    {0},    {0},   codey_sensor_report_dail_get_data },
  { false,    GYRO_X_DGR_ID,    sizeof(float),    0,    {0},    {0},   codey_sensor_report_gyro_x_dgr_get_data },
  { false,    GYRO_Y_DGR_ID,    sizeof(float),    0,    {0},    {0},   codey_sensor_report_gyro_y_dgr_get_data },
  { false,    GYRO_Z_DGR_ID,    sizeof(float),    0,    {0},    {0},   codey_sensor_report_gyro_z_dgr_get_data },
  { false,    GYRO_X_G_ID,      sizeof(float),    0,    {0},    {0},   codey_sensor_report_gyro_x_g_get_data },
  { false,    GYRO_Y_G_ID,      sizeof(float),    0,    {0},    {0},   codey_sensor_report_gyro_y_g_get_data },
  { false,    GYRO_Z_G_ID,      sizeof(float),    0,    {0},    {0},   codey_sensor_report_gyro_z_g_get_data },
  { false,    POWER_LEVLE,      sizeof(float),    0,    {0},    {0},   codey_sensor_report_power_level_get_data },
  { false,    LOCAL_TIME_ID,    sizeof(float),    0,    {0},    {0},   codey_sensor_report_local_time_get_data },
};

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_sensor_report_init(void)
{

  xTaskCreatePinnedToCore(codey_sensor_report_main_loop, 
                          "sensor_report_task", 
                          SENSOR_REPORT_TASK_STACK_LEN, 
                          NULL, 
                          SENSOR_REPORT_TASK_PRIORITY, 
                          NULL, 
                          0 
                         );

  s_module_init = true;                          

}

void codey_sensor_report_request(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint8_t ack;
  uint16_t sensor_id;

  if(!s_module_init)
  {
    codey_sensor_report_init();
  }

  switch(data[0])
  {
    case REQ_REPORT:
      sensor_id = *(uint16_t *)(data + 1);
      ack = codey_sensor_report_req(sensor_id)?1:0;
      if(ack) 
      {
        output_buf[0] = RSP_OK;
      }
      else
      {
        output_buf[0] = RSP_ERR;
      }
      *(uint16_t *)(output_buf + 1) = sensor_id;
      *output_len = 3;
    break;

    case REQ_STOP:
      sensor_id = *(uint16_t *)(data + 1);
      ack = codey_sensor_report_stop(sensor_id)?1:0;
      if(ack) 
      {
        output_buf[0] = RSP_OK;
      }
      else
      {
        output_buf[0] = RSP_ERR;
      }
      *(uint16_t *)(output_buf + 1) = sensor_id;
      *output_len = 3;    
    break;
  
    case REPORT:
      // Do nothing
      *output_len = 0;
      break;

    case RSP_OK:
      // Do nothing
      *output_len = 0;
    break;

    case RSP_ERR:
      // Do nothing
      *output_len = 0;
    break;
  }
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
static void codey_sensor_report_main_loop(void * parameter)
{

  while(1)
  {

    codey_sensor_report_update();

    codey_sensor_report_to_host();
    
    vTaskDelay(REPORT_MIN_DELAY_MS/portTICK_PERIOD_MS);
  }
}

static void codey_sensor_report_update(void)
{
  uint32_t i;

  for(i = 0; i < SENSOR_NUM; i++)
  {
    if(s_sensor_report_info_tab[i].on_or_off && s_sensor_report_info_tab[i].get_data)
    {
      s_sensor_report_info_tab[i].get_data(s_sensor_report_info_tab[i].cur_report_data);
    }
  }
}

static void codey_sensor_report_to_host(void)
{
  uint32_t i;

  for(i = 0; i < SENSOR_NUM; i++)
  {
    if(s_sensor_report_info_tab[i].on_or_off)
    {
      if((GET_TICK - s_sensor_report_info_tab[i].last_report_tick) > REPORT_MIN_DELAY_MS)
      {
        if(0 != memcmp(s_sensor_report_info_tab[i].last_report_data, 
                          s_sensor_report_info_tab[i].cur_report_data, 
                          s_sensor_report_info_tab[i].report_data_len))
        {
          codey_sensor_report_send(&(s_sensor_report_info_tab[i]));
          s_sensor_report_info_tab[i].last_report_tick = GET_TICK;
          memcpy( s_sensor_report_info_tab[i].last_report_data, 
                  s_sensor_report_info_tab[i].cur_report_data, 
                  s_sensor_report_info_tab[i].report_data_len);
        }
      }
    }
  }
}

static void codey_sensor_report_send(sensor_report_info_t *sensor_report_info)
{
  uint32_t data_len;

  // 1) action_id
  s_sensor_report_tx_buf[0] = REPORT;
  // 2) sensor_id
  *(uint16_t *)(s_sensor_report_tx_buf + 1) = sensor_report_info->sensor_id;
  // 3) sensor_value
  memcpy(s_sensor_report_tx_buf + 3, sensor_report_info->cur_report_data, sensor_report_info->report_data_len);

  data_len = 3 + sensor_report_info->report_data_len;
  codey_comm_build_frame(SENSOR_REPORT_ID, s_sensor_report_tx_buf, &data_len);
  uart_write_bytes(UART, (const char *)s_sensor_report_tx_buf, data_len);
  codey_ble_dev_put_data(s_sensor_report_tx_buf, data_len);  
}

static bool codey_sensor_report_req(uint16_t sensor_id)
{
  sensor_report_info_t *sensor_report_info;

  sensor_report_info = codey_sensor_report_find_info_by_id(sensor_id);
  if(!sensor_report_info)
  {
    return false;
  }

  sensor_report_info->on_or_off = true;
  memset(sensor_report_info->last_report_data, 0, REPORT_DATA_SIZE);
  return true;
}

static bool codey_sensor_report_stop(uint16_t sensor_id)
{
  sensor_report_info_t *sensor_report_info;

  sensor_report_info = codey_sensor_report_find_info_by_id(sensor_id);
  if(!sensor_report_info)
  {
    return false;
  }

  sensor_report_info->on_or_off = false;
  return true;
}

static sensor_report_info_t *codey_sensor_report_find_info_by_id(uint16_t sensor_id)
{
  uint32_t i;

  for(i = 0; i < SENSOR_NUM; i++)
  {
    if(s_sensor_report_info_tab[i].sensor_id == sensor_id)
    {
      return &(s_sensor_report_info_tab[i]);
    }
  }

  return NULL;
}

static void codey_sensor_report_sound_get_data(uint8_t *output_data)
{
  *((float *)(output_data)) = codey_sound_sensor_value_t();
}

static void codey_sensor_report_light_get_data(uint8_t *output_data)
{
  *((float *)(output_data)) = codey_light_sensor_value_t();
}

static void codey_sensor_report_dail_get_data(uint8_t *output_data)
{
  *((float *)(output_data)) = codey_dail_sensor_value_t();
}

static void codey_sensor_report_gyro_x_dgr_get_data(uint8_t *output_data)
{
  *((float *)(output_data)) = codey_gyro_sensor_value_t(1);
}

static void codey_sensor_report_gyro_y_dgr_get_data(uint8_t *output_data)
{ 
  *((float *)(output_data)) = codey_gyro_sensor_value_t(2);
}

static void codey_sensor_report_gyro_z_dgr_get_data(uint8_t *output_data)
{ 
  *((float *)(output_data)) = codey_gyro_sensor_value_t(3);
}

static void codey_sensor_report_gyro_x_g_get_data(uint8_t *output_data)
{
  *((float *)(output_data)) = codey_gyro_sensor_value_t(4);
}

static void codey_sensor_report_gyro_y_g_get_data(uint8_t *output_data)
{  
  *((float *)(output_data)) = codey_gyro_sensor_value_t(5);
}

static void codey_sensor_report_gyro_z_g_get_data(uint8_t *output_data)
{  
  *((float *)(output_data)) = codey_gyro_sensor_value_t(6);
}

static void codey_sensor_report_power_level_get_data(uint8_t *output_data)
{  
  *((float *)(output_data)) = codey_battery_check_get_capacity();
}

static void codey_sensor_report_local_time_get_data(uint8_t *output_data)
{ 
  *((float *)(output_data)) = (float)codey_timer_value_t();
}
