/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for gyro module
 * @file    codey_gyro_board.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/20
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
 * This file is a drive gyro_sensor module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/03/20        1.0.0            build the new.
 *  Mark Yan          2017/03/31        1.0.0            update for available version.
 * </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>	

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "driver/i2c.h"	
#include "driver/gpio.h"

#include "py/mpstate.h"
#include "py/runtime.h"

#include "soc/uart_struct.h"
#include "uart.h"
#include "esp_log.h"

#include "codey_gyro_board.h"
#include "mphalport.h"
#include "codey_sys.h"
#include "codey_event_mechanism.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG                        ("codey_gyro")
#define MAX_I2C_RESTORE_SCL_CNT    (8 * 32)
#define PI                         (3.1415926)
#define GYRO_UPDATE_THRESHOLD_ACC  (10)  // acc
#define GYRO_UPDATE_THRESHOLD_GYRO (1)   // gyro  
#define ACC_FILTER                 (0.92)
#define TILT_ANGLE_FILTER          (0.5)
#define TILT_RANGE_GAIN            (0.2588) // cos(75)     

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
typedef struct
{
  mp_obj_base_t base;
}codey_gyro_board_obj_t; 

typedef struct
{
  double sensitivity; /* for 500 deg/s, check data sheet */
  uint16_t acc_earth;   
  double gyr_x_offs;
  double gyr_y_offs;
  double gyr_z_offs;
}codey_gyro_board_compensation_t;

/* event about gyro */
typedef enum
{
  SHAKE = 0,
  TILT_LEFT,
  TILT_RIGHT,
  TILT_FORWARD,
  TILT_BACK,
  SCREEN_UP,
  SCREEN_DOWN,
  FREE_FALL,
  EVENT_MAX
}codey_gyro_event_type_t;

typedef struct
{ 
  int16_t threshold_value_high;
  int16_t threshold_value_low;
  uint8_t event_occured_flag;
}codey_gyro_board_cb_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
static codey_gyro_board_obj_t s_codey_gyro_board_obj = {.base = {&codey_gyro_board_type}};
static codey_gyro_board_compensation_t s_codey_gyro_board_compensation;
static bool s_codey_gyro_board_init_flag = false;
static bool s_codey_gyro_board_ope_i2c_flag = false;
codey_gyro_board_cb_t s_codey_gyro_board_cb[EVENT_MAX];
static bool s_is_shaked = false;
static gyro_data_for_neurons_t s_gyro_value_struct;

/******************************************************************************
 DECLARE PUBLIC DATA
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC esp_err_t  gpio_drive_check(uint8_t gpio_num);
STATIC esp_err_t  i2c_bus_check(void);
STATIC esp_err_t i2c_bus_drive_restore(void);

STATIC esp_err_t  i2c_master_init_t(void);
STATIC esp_err_t  i2c_master_deinit_t(void);
STATIC esp_err_t  i2c_write_gyro_reg(uint8_t reg, uint8_t data);
STATIC esp_err_t  i2c_read_gyro_data(i2c_port_t i2c_num ,uint8_t start, uint8_t *buffer, uint8_t size);
STATIC esp_err_t  codey_gyro_board_device_calibration_t(void);
STATIC void codey_gyro_board_update_t(void);
STATIC void codey_gyro_board_event_init_t(void);
STATIC void codey_gyro_board_event_listening_t(int16_t ax, int16_t ay, int16_t az);
STATIC void codey_shake_detect_t(void);
STATIC float codey_gyro_board_pitch_angle_range_t(float angle);
STATIC float codey_gyro_board_roll_angle_range_t(float angle);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
float codey_gyro_board_get_static_acc_t(void)
{
  return (float)(s_gyro_value_struct.acc_earth);
}

bool codey_gyro_board_get_ope_i2c_flag_t(void)
{
  return s_codey_gyro_board_ope_i2c_flag;
}

void codey_gyro_board_set_ope_i2c_flag_t(bool sta)
{
  s_codey_gyro_board_ope_i2c_flag = sta;
}

void codey_gyro_board_get_data_t(void)
{
  codey_gyro_board_update_t();
  codey_shake_detect_t();

  codey_gyro_board_event_listening_t(s_gyro_value_struct.acc_x, s_gyro_value_struct.acc_y, s_gyro_value_struct.acc_z);
}

gyro_data_for_neurons_t codey_gyro_board_read_data_t(void)
{
  return s_gyro_value_struct;
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
*******************************************************************************/
STATIC esp_err_t gpio_drive_check(uint8_t gpio_num)
{
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;   
  io_conf.mode = GPIO_MODE_INPUT_OUTPUT_OD;
  io_conf.pin_bit_mask = (1 << gpio_num);
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;
  gpio_config(&io_conf);

  // output high and read must be high
  gpio_set_level(gpio_num, 1);
  if(1 != gpio_get_level(gpio_num))
  {
    ESP_LOGE("GPIO_DRIVE_CHECK", "gpio can NOT drive high");
    return ESP_FAIL;
  }

  // output low and read must be low
  gpio_set_level(gpio_num, 0);
  if(0 != gpio_get_level(gpio_num))
  {
    ESP_LOGE("GPIO_DRIVE_CHECK", "gpio can NOT drive low");
    return ESP_FAIL;
  }

  return ESP_OK;
}
 
STATIC esp_err_t i2c_bus_check(void)
{
  esp_err_t ret;
 
  ret = gpio_drive_check(I2C_SCL_IO);
  if(ESP_OK != ret)
  {
    ESP_LOGE(TAG, "I2C_SCL_IO drive check fail");
    return ret;
  }

  ret = gpio_drive_check(I2C_SDA_IO);
  if(ESP_OK != ret)
  {
    ESP_LOGE(TAG, "I2C_SDA_IO drive check fail");
    return ret;
  }
  return ret;
}
 
STATIC esp_err_t i2c_bus_drive_restore(void)
{
  uint32_t i2c_clk_cnt;
  gpio_config_t io_conf;
 
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;   
  io_conf.mode = GPIO_MODE_INPUT_OUTPUT_OD;
  io_conf.pin_bit_mask = (1 << I2C_SCL_IO);
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;
  gpio_config(&io_conf);

  i2c_clk_cnt = 0;  
  while(ESP_OK != i2c_bus_check() && i2c_clk_cnt++ < MAX_I2C_RESTORE_SCL_CNT)
  {
    gpio_set_level(I2C_SCL_IO, 0);
    vTaskDelay(2 / portTICK_PERIOD_MS);
    gpio_set_level(I2C_SCL_IO, 1);
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }

  if(i2c_clk_cnt < MAX_I2C_RESTORE_SCL_CNT)
  {
    return ESP_OK;
  }
  else
  {
    return ESP_FAIL;
  }
}
 
STATIC esp_err_t i2c_master_init_t(void)
{ 
  if(ESP_OK != i2c_bus_drive_restore())
  {
    return ESP_FAIL;
  }
  i2c_port_t i2c_master_port = I2C_NUM;
  i2c_config_t conf; 
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = I2C_SDA_IO;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = I2C_SCL_IO;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = I2C_FREQ_HZ;
  esp_err_t res = ESP_OK;
  res = res | i2c_param_config(i2c_master_port, &conf);
  res = res | i2c_driver_install(i2c_master_port, conf.mode, I2C_RX_BUF_DISABLE, I2C_TX_BUF_DISABLE, 0); 
  vTaskDelay(200 / portTICK_PERIOD_MS);
  
  return res;
}

STATIC esp_err_t i2c_master_deinit_t(void)
{ 
  i2c_port_t i2c_master_port = I2C_NUM;
  esp_err_t res=ESP_OK;
  res = res | i2c_reset_tx_fifo(i2c_master_port);
  res = res | i2c_reset_rx_fifo(i2c_master_port);
  res = res | i2c_driver_delete(i2c_master_port); 
  vTaskDelay(200 / portTICK_PERIOD_MS);
  
  return res;
}

STATIC esp_err_t i2c_write_gyro_reg(uint8_t reg, uint8_t data)
{
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();  // create a command link,command data will be added to this link and then sent at once
  i2c_master_start(cmd);                         // I2C logic has been packaged in these functions
  i2c_master_write_byte(cmd, (GYRO_DEFAULT_ADDRESS << 1) | WRITE_B, ACK_CHECK_E);
  i2c_master_write(cmd, &reg, 1, ACK_CHECK_E);
  i2c_master_write(cmd, &data, 1, ACK_CHECK_E);
  i2c_master_stop(cmd);
  esp_err_t res = i2c_master_cmd_begin(I2C_NUM, cmd, 100 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  
  return res;
}

STATIC esp_err_t i2c_read_gyro_data(i2c_port_t i2c_num, uint8_t start, uint8_t *buffer, uint8_t size)
{
  if(size == 0)
  {
    return ESP_OK;
  }

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (GYRO_DEFAULT_ADDRESS << 1) | WRITE_B, ACK_CHECK_E);
  i2c_master_write(cmd, &start, 1, ACK_CHECK_E);

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (GYRO_DEFAULT_ADDRESS << 1) | READ_B, ACK_CHECK_E);
  if(size > 1) 
  {
    i2c_master_read(cmd, buffer, size - 1, ACK_V);
  }
  i2c_master_read_byte(cmd, buffer + size - 1, NACK_V); //the lastest byte will not give a ASK
  i2c_master_stop(cmd);
  esp_err_t res = i2c_master_cmd_begin(i2c_num, cmd, 100 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  
  return res;
}

STATIC esp_err_t codey_gyro_board_device_calibration_t(void)
{
  uint8_t  s_i2c_data[14];
  uint16_t x = 0;
  uint16_t num = 500;
  long     xSum = 0, ySum = 0, zSum = 0;
  uint32_t acc_static_sum = 0;
  int16_t  temp[3];
  for(x = 0; x < num; x++)
  {
    uint8_t res =i2c_read_gyro_data(I2C_NUM,0x3b, s_i2c_data, 14);
    if(res != ESP_OK)
    {
      ESP_LOGE(TAG, "valibration read values error");
      return ESP_FAIL;
    }
    if(x >= 50 && x < 450)
    {
      temp[0] = (int16_t)((s_i2c_data[0] << 8) | s_i2c_data[1]);
      temp[1] = (int16_t)((s_i2c_data[2] << 8) | s_i2c_data[3]);
      temp[2] = (int16_t)((s_i2c_data[4] << 8) | s_i2c_data[5]);
      acc_static_sum += sqrt(pow(temp[0], 2) + pow(temp[1], 2) + pow(temp[2], 2));
      xSum += ((int16_t)(s_i2c_data[8] << 8) | s_i2c_data[9]);
      ySum += ((int16_t)(s_i2c_data[10] << 8) | s_i2c_data[11]);
      zSum += ((int16_t)(s_i2c_data[12] << 8) | s_i2c_data[13]);
    }
  }
  
#if CODEY_SENSOR_GYRO_ZERO_COMPENSATION_ENABLE
  s_codey_gyro_board_compensation.gyr_x_offs = xSum / (num - 100);
  s_codey_gyro_board_compensation.gyr_y_offs = ySum / (num - 100);
  s_codey_gyro_board_compensation.gyr_z_offs = zSum / (num - 100);
#endif
  s_gyro_value_struct.acc_earth = acc_static_sum / (num - 100);

  return ESP_OK;
}

bool codey_gyro_board_init_t(void)
{
  if(s_codey_gyro_board_init_flag == false)
  {
    s_codey_gyro_board_compensation.sensitivity = 65.5; // for 500 deg/s, check data sheet
   
    s_codey_gyro_board_compensation.gyr_x_offs = 0; // cilibration datas of gyro
    s_codey_gyro_board_compensation.gyr_y_offs = 0;
    s_codey_gyro_board_compensation.gyr_z_offs = 0;

    esp_err_t res = ESP_OK;
    res=res | i2c_master_init_t();
    uint8_t id = 0;
    uint16_t con = 0;
    do
    {
      i2c_read_gyro_data(I2C_NUM, 0x75, &id, 1);
      vTaskDelay(20 / portTICK_PERIOD_MS);
      if((con++) > 5)
      {
        ESP_LOGE(TAG, "makeblock gyro_init_err\n");
        return false;
      }
    }while(id != 0x98);

    res = res | i2c_write_gyro_reg(0x6b, 0x00);//close the sleep mode
    vTaskDelay(200 / portTICK_PERIOD_MS);
    res = res | i2c_write_gyro_reg(0x1a, 0x01);//configurate the digital low pass filter
    vTaskDelay(100 / portTICK_PERIOD_MS);
    res = res | i2c_write_gyro_reg(0x1b, 0x08);//set the gyro scale to 500 deg/s
    vTaskDelay(100 / portTICK_PERIOD_MS);   
    res = res | i2c_write_gyro_reg(0x19, 19);  //set the Sampling Rate   50Hz 49
  
    res = res | codey_gyro_board_device_calibration_t();
  
    if(res == ESP_OK)
    {
      /* if gyro init succeed, init gyro event */ 
      codey_gyro_board_event_init_t();
      s_codey_gyro_board_init_flag = true;
      ESP_LOGI(TAG, "codey gyro init succeed");
      return true;
    }
    else
    {
      return false;
    }
  }
  s_codey_gyro_board_init_flag = true;
  return true;
}

bool codey_gyro_board_deinit_t(void)
{   
  s_codey_gyro_board_compensation.gyr_y_offs = 0; // cilibration datas of gyro
  s_codey_gyro_board_compensation.gyr_y_offs = 0;
  s_codey_gyro_board_compensation.gyr_z_offs = 0;

  esp_err_t res = ESP_OK;
  res = res | i2c_master_deinit_t();

  if(res == ESP_OK)
  {
    s_codey_gyro_board_init_flag = false;
    return true;
  }
  else
  {
    s_codey_gyro_board_init_flag = false;
    return false;
  }
}

STATIC float codey_gyro_board_pitch_angle_range_t(float angle)
{
  /* -180 ~ 180 */
  if(angle > 180)
  {
    return angle - 360;
  }
  else
  {
    return angle;
  }
}

STATIC float codey_gyro_board_roll_angle_range_t(float angle)
{
  /* -90 ~ 90 */
  if(angle > 90.5 && angle < 269.5)
  {
    return 0;
  }
  else if(angle >= 269.5)
  {
    return angle - 360;
  }
  else
  {
    return angle;
  }
}

STATIC void codey_gyro_board_update_t(void)
{
  /* only when I2C init succeed, reading data from mpu6050 */
  if(!s_codey_gyro_board_init_flag)
  {
    return;
  }
  
  uint8_t s_i2c_data[14];
  static bool first_update = true;
  static volatile  uint32_t last_time = 0;
  double delta_t;
  double filter_coefficient; 
  static double pitch_static = 0.0;
  // static double yaw_static; // not used now 
  static double roll_static;
  static double s_gx = 0.0;
  static double s_gy = 0.0;
  static double s_gz = 0.0;
  static double s_gyro_x = 0.0;
  static double s_gyro_y = 0.0;
  static double s_gyro_z = 0.0;
  static double acc_x_static = 0.0;
  static double acc_y_static = 0.0;
  static double acc_z_static = 0.0;
  static double acc_x_static_last = 0.0;
  static double acc_y_static_last = 0.0;
  static double acc_z_static_last = 0.0;

  double s_acc_x = 0.0, s_acc_y = 0.0, s_acc_z = 0.0;
  int16_t acc_x_raw = 0, acc_y_raw = 0, acc_z_raw = 0;
  
  codey_gyro_board_set_ope_i2c_flag_t(true);
  delta_t = (double)(millis() - last_time) / 1000;    //chage to unit of second
  last_time = millis();

  /* read acc¡¢gyro¡¢temperature datas */
  uint8_t res = i2c_read_gyro_data(I2C_NUM, 0x3b, s_i2c_data, 14);
  if(res != ESP_OK)
  { 
    ESP_LOGE(TAG, "gyro read data eror");
    codey_gyro_board_set_ope_i2c_flag_t(false);
    return;
  }

  acc_x_raw = (int16_t)((s_i2c_data[0] << 8) | s_i2c_data[1]);
  acc_y_raw = (int16_t)((s_i2c_data[2] << 8) | s_i2c_data[3]);
  acc_z_raw = (int16_t)((s_i2c_data[4] << 8) | s_i2c_data[5]);  
  /* assemble 16 bit sensor data */
  if(first_update)
  {
    acc_x_static = acc_x_raw;
    acc_y_static = acc_y_raw;
    acc_z_static = acc_z_raw;  
  }
  else
  {
    acc_x_static = acc_x_static * ACC_FILTER + acc_x_raw * (1 - ACC_FILTER);
    acc_y_static = acc_y_static * ACC_FILTER + acc_y_raw * (1 - ACC_FILTER);
    acc_z_static = acc_z_static * ACC_FILTER + acc_z_raw * (1 - ACC_FILTER);  
  }
  int16_t temp = ((s_i2c_data[6] << 8) | s_i2c_data[7]); 
  s_gyro_value_struct.temperature = 36.53 + temp / 340.0; 
  s_gyro_x = ((int16_t)((s_i2c_data[8] << 8)  | s_i2c_data[9]) - s_codey_gyro_board_compensation.gyr_x_offs) / s_codey_gyro_board_compensation.sensitivity;
  s_gyro_y = ((int16_t)((s_i2c_data[10] << 8) | s_i2c_data[11]) - s_codey_gyro_board_compensation.gyr_y_offs) / s_codey_gyro_board_compensation.sensitivity;
  s_gyro_z = ((int16_t)((s_i2c_data[12] << 8) | s_i2c_data[13]) - s_codey_gyro_board_compensation.gyr_z_offs) / s_codey_gyro_board_compensation.sensitivity;  

  /* the direction is invert to the definition of codey */
  s_acc_x = -acc_x_raw;
  s_acc_y = -acc_y_raw;
  s_acc_z = acc_z_raw;
  s_gyro_value_struct.acc_x = s_gyro_value_struct.acc_x * 0.8 + s_acc_x * 0.2; 
  s_gyro_value_struct.acc_y = s_gyro_value_struct.acc_y * 0.8 + s_acc_y * 0.2;  
  s_gyro_value_struct.acc_z = s_gyro_value_struct.acc_z * 0.8 + s_acc_z * 0.2;
  if(s_acc_z >= 0)
  {
    pitch_static = atan2(-s_gyro_value_struct.acc_x, sqrt(pow(s_gyro_value_struct.acc_y, 2) + pow(s_gyro_value_struct.acc_z, 2))) * 180 / PI;
    roll_static = atan2(s_gyro_value_struct.acc_y, sqrt(pow(s_gyro_value_struct.acc_x, 2) + pow(s_gyro_value_struct.acc_z, 2))) * 180 / PI;
  }
  else
  {
    pitch_static = atan2(-s_gyro_value_struct.acc_x, -sqrt(pow(s_gyro_value_struct.acc_y, 2) + pow(s_gyro_value_struct.acc_z, 2))) * 180 / PI;
    roll_static = atan2(s_gyro_value_struct.acc_y, -sqrt(pow(s_gyro_value_struct.acc_x, 2) + pow(s_gyro_value_struct.acc_z, 2))) * 180 / PI;
  }
  /* the direction difinition of roll is invert to codey */
  roll_static = -roll_static;
  /* change the value to 0 - 360 degree */

  if(pitch_static < 0)
  {
    pitch_static = 360 + pitch_static;
  }
  if(roll_static < 0)
  {
    roll_static = 360 + roll_static;
  }

  if(first_update)
  {
    s_gx = roll_static;
    s_gy = pitch_static;
    s_gz = 0;
    first_update = false;
  }
  else
  {
    s_gx = s_gx + s_gyro_x * delta_t;
    s_gy = s_gy - s_gyro_y * delta_t;
    if(fabs(acc_x_static - acc_x_static_last) > GYRO_UPDATE_THRESHOLD_ACC
       || fabs(acc_y_static - acc_y_static_last) > GYRO_UPDATE_THRESHOLD_ACC
       || fabs(acc_z_static - acc_z_static_last) > GYRO_UPDATE_THRESHOLD_ACC 
       || fabs(s_gyro_z) > GYRO_UPDATE_THRESHOLD_GYRO)
    {
      s_gz = s_gz + s_gyro_z * delta_t;
    }
    if(s_gx < 0)
    {
      s_gx = s_gx + 360;
    }
    if(s_gy < 0)
    {
      s_gy = s_gy + 360;
    }
    /*
    if(s_gz < 0)
    {
      s_gz = s_gz + 360;
    }
    */
    if(s_gx >= 360)
    {
      s_gx = s_gx - 360;
    }
    if(s_gy >= 360)
    {
      s_gy = s_gy - 360;
    }
    /*
    if(s_gz > 360)
    {
      s_gz = s_gz - 360;
    }
    */
  }
  acc_x_static_last = acc_x_static;
  acc_y_static_last = acc_y_static;
  acc_z_static_last = acc_z_static;

  /* if we need to guarantee the accuracy of dynamic process, 
   * the filter parameter should be samll */
  filter_coefficient = TILT_ANGLE_FILTER; // 0.5 / (0.5 + delta_t);
  s_gx = s_gx * filter_coefficient + roll_static * (1 - filter_coefficient);
  s_gy = s_gy * filter_coefficient + pitch_static * (1 - filter_coefficient);
  codey_gyro_board_set_ope_i2c_flag_t(false);
  
  /* update the struct of gyro data */
  s_gyro_value_struct.angle_pitch = codey_gyro_board_pitch_angle_range_t(s_gy);
  s_gyro_value_struct.angle_roll = codey_gyro_board_roll_angle_range_t(s_gx);
  s_gyro_value_struct.angle_yaw = s_gz;
  s_gyro_value_struct.gyr_x = s_gyro_x;  
  s_gyro_value_struct.gyr_y = s_gyro_y;
  s_gyro_value_struct.gyr_z = s_gyro_z;
  // printf("***gx is %f, gy is %f, gz is %f\n", s_gx, s_gy, s_gz);
}

/* for shaked detect */
STATIC void codey_shake_detect_t(void)
{
  uint16_t temp = 0;
  static uint16_t acc_filter = 0;
  static bool first_flag = false;
  
  if(first_flag == false)
  {
    acc_filter = s_gyro_value_struct.acc_earth;
    first_flag = true;
  }
  
  temp = sqrt(pow(s_gyro_value_struct.acc_x, 2) + pow(s_gyro_value_struct.acc_y, 2) + pow(s_gyro_value_struct.acc_z, 2));
  acc_filter = acc_filter * 0.5 + temp * 0.5; 
  if(acc_filter > s_codey_gyro_board_cb[SHAKE].threshold_value_high || acc_filter < s_codey_gyro_board_cb[SHAKE].threshold_value_low)
  {
    s_is_shaked = true;
  }
  else
  {
    s_is_shaked = false;
  }
}
 
/* for gyro event machanism */
STATIC void codey_gyro_board_event_init_t(void)
{
  /* 0 for shake */
  s_codey_gyro_board_cb[SHAKE].event_occured_flag = false;
  s_codey_gyro_board_cb[SHAKE].threshold_value_high = s_gyro_value_struct.acc_earth * 1.5;
  s_codey_gyro_board_cb[SHAKE].threshold_value_low = s_gyro_value_struct.acc_earth * 0.7;
  
  /* 1 for tilt left */
  s_codey_gyro_board_cb[TILT_LEFT].event_occured_flag = false;
  s_codey_gyro_board_cb[TILT_LEFT].threshold_value_high = s_gyro_value_struct.acc_earth;
  s_codey_gyro_board_cb[TILT_LEFT].threshold_value_low = s_gyro_value_struct.acc_earth * TILT_RANGE_GAIN;
  
  /* 2 for tilt right */
  s_codey_gyro_board_cb[TILT_RIGHT].event_occured_flag = false;
  s_codey_gyro_board_cb[TILT_RIGHT].threshold_value_high = -s_gyro_value_struct.acc_earth * TILT_RANGE_GAIN;
  s_codey_gyro_board_cb[TILT_RIGHT].threshold_value_low = -s_gyro_value_struct.acc_earth;

  /* 3 for tilt forward */
  s_codey_gyro_board_cb[TILT_FORWARD].event_occured_flag = false;
  s_codey_gyro_board_cb[TILT_FORWARD].threshold_value_high = s_gyro_value_struct.acc_earth;
  s_codey_gyro_board_cb[TILT_FORWARD].threshold_value_low = s_gyro_value_struct.acc_earth * TILT_RANGE_GAIN;

  /* 4 for tilt back */
  s_codey_gyro_board_cb[TILT_BACK].event_occured_flag = false;
  s_codey_gyro_board_cb[TILT_BACK].threshold_value_high= -s_gyro_value_struct.acc_earth * TILT_RANGE_GAIN;
  s_codey_gyro_board_cb[TILT_BACK].threshold_value_low = -s_gyro_value_struct.acc_earth;

  /* 5 for screen up */
  s_codey_gyro_board_cb[SCREEN_UP].event_occured_flag = false;
  s_codey_gyro_board_cb[SCREEN_UP].threshold_value_high = 0; // no max
  s_codey_gyro_board_cb[SCREEN_UP].threshold_value_low = 5000;

  /* 6 for screen down */
  s_codey_gyro_board_cb[SCREEN_DOWN].event_occured_flag = false;
  s_codey_gyro_board_cb[SCREEN_DOWN].threshold_value_high = -5000;
  s_codey_gyro_board_cb[SCREEN_DOWN].threshold_value_low = 0; // no max

  /* 7 for free fall */
  s_codey_gyro_board_cb[FREE_FALL].event_occured_flag = false;
  s_codey_gyro_board_cb[FREE_FALL].threshold_value_high = 2000;
  s_codey_gyro_board_cb[FREE_FALL].threshold_value_low = -2000;
}

STATIC void codey_gyro_board_event_listening_t(int16_t ax, int16_t ay, int16_t az)
{
  // static uint8_t tilt_triggered[5] = {0, 0, 0, 0, 0}; // shake: 0  left: 1 right: 2  forward: 3  back: 4 

  /* 0 for shake */
  if(s_is_shaked == true)
  {
    s_codey_gyro_board_cb[SHAKE].event_occured_flag = true;
  }
  else
  {
    s_codey_gyro_board_cb[SHAKE].event_occured_flag = false;
    codey_eve_set_triggerd_flag_t(SHAKE + EVE_BOARD_SHAKE, false);
  }
  /* 1 for tilt left   2 for tilt right */ 
  if(ay <= s_codey_gyro_board_cb[TILT_LEFT].threshold_value_high && ay >= s_codey_gyro_board_cb[TILT_LEFT].threshold_value_low)
  {
    s_codey_gyro_board_cb[TILT_LEFT].event_occured_flag = true;
    s_codey_gyro_board_cb[TILT_RIGHT].event_occured_flag = false;
    codey_eve_set_triggerd_flag_t(TILT_RIGHT + EVE_BOARD_SHAKE, false);
  }
  else if(ay <= s_codey_gyro_board_cb[TILT_RIGHT].threshold_value_high && ay >= s_codey_gyro_board_cb[TILT_RIGHT].threshold_value_low)
  {
    s_codey_gyro_board_cb[TILT_LEFT].event_occured_flag = false;
    s_codey_gyro_board_cb[TILT_RIGHT].event_occured_flag = true;
    codey_eve_set_triggerd_flag_t(TILT_LEFT + EVE_BOARD_SHAKE, false);
  }
  else
  {
    s_codey_gyro_board_cb[TILT_LEFT].event_occured_flag = false;
    s_codey_gyro_board_cb[TILT_RIGHT].event_occured_flag = false;
    codey_eve_set_triggerd_flag_t(TILT_LEFT + EVE_BOARD_SHAKE, false);
    codey_eve_set_triggerd_flag_t(TILT_RIGHT + EVE_BOARD_SHAKE, false);
  }
  
  /* 3 for tilt forward  4 for tilt back */
  if(ax <= s_codey_gyro_board_cb[TILT_FORWARD].threshold_value_high && ax >= s_codey_gyro_board_cb[TILT_FORWARD].threshold_value_low)
  {
    if(fabs(ax) >= fabs(ay))
    {
      s_codey_gyro_board_cb[TILT_FORWARD].event_occured_flag = true;
      s_codey_gyro_board_cb[TILT_BACK].event_occured_flag = false;
      s_codey_gyro_board_cb[TILT_LEFT].event_occured_flag = false;
      s_codey_gyro_board_cb[TILT_RIGHT].event_occured_flag = false;
      codey_eve_set_triggerd_flag_t(TILT_BACK + EVE_BOARD_SHAKE, false);
      codey_eve_set_triggerd_flag_t(TILT_LEFT + EVE_BOARD_SHAKE, false);
      codey_eve_set_triggerd_flag_t(TILT_RIGHT + EVE_BOARD_SHAKE, false);
    }
    else
    {
      codey_eve_set_triggerd_flag_t(TILT_FORWARD + EVE_BOARD_SHAKE, false);
      codey_eve_set_triggerd_flag_t(TILT_BACK + EVE_BOARD_SHAKE, false);
    }
  }
  else if(ax <= s_codey_gyro_board_cb[TILT_BACK].threshold_value_high && ax >= s_codey_gyro_board_cb[TILT_BACK].threshold_value_low)
  {
   if(fabs(ax) >= fabs(ay))
   {
     s_codey_gyro_board_cb[TILT_FORWARD].event_occured_flag = false;
     s_codey_gyro_board_cb[TILT_BACK].event_occured_flag = true;
     s_codey_gyro_board_cb[TILT_LEFT].event_occured_flag = false;
     s_codey_gyro_board_cb[TILT_RIGHT].event_occured_flag = false;
     codey_eve_set_triggerd_flag_t(TILT_FORWARD + EVE_BOARD_SHAKE, false);
     codey_eve_set_triggerd_flag_t(TILT_LEFT + EVE_BOARD_SHAKE, false);
     codey_eve_set_triggerd_flag_t(TILT_RIGHT + EVE_BOARD_SHAKE, false);
   }
   else
   {
     codey_eve_set_triggerd_flag_t(TILT_FORWARD + EVE_BOARD_SHAKE, false);
     codey_eve_set_triggerd_flag_t(TILT_BACK + EVE_BOARD_SHAKE, false);
   }
  }
  else
  {
    s_codey_gyro_board_cb[TILT_FORWARD].event_occured_flag = false;
    s_codey_gyro_board_cb[TILT_BACK].event_occured_flag = false;    
    codey_eve_set_triggerd_flag_t(TILT_FORWARD + EVE_BOARD_SHAKE, false);
    codey_eve_set_triggerd_flag_t(TILT_BACK + EVE_BOARD_SHAKE, false);
  }

  /* 5 for screen up  6 for screen down*/
  if(az >= s_codey_gyro_board_cb[SCREEN_UP].threshold_value_low)
  {
    s_codey_gyro_board_cb[SCREEN_UP].event_occured_flag = true;
    s_codey_gyro_board_cb[SCREEN_DOWN].event_occured_flag = false;
  }
  else if(az <= s_codey_gyro_board_cb[SCREEN_DOWN].threshold_value_high)
  {
    s_codey_gyro_board_cb[SCREEN_UP].event_occured_flag = false;
    s_codey_gyro_board_cb[SCREEN_DOWN].event_occured_flag = true;
  }
  else
  {
    s_codey_gyro_board_cb[SCREEN_UP].event_occured_flag = false;
    s_codey_gyro_board_cb[SCREEN_DOWN].event_occured_flag = false;    
  }

  /* 7 for free fall */
  uint16_t acc_total = sqrt(ax * ax + ay * ay + az * az);
  if(acc_total >= s_codey_gyro_board_cb[FREE_FALL].threshold_value_low && acc_total <= s_codey_gyro_board_cb[FREE_FALL].threshold_value_high)
  {
    s_codey_gyro_board_cb[FREE_FALL].event_occured_flag = true;
  }
  else
  {
    s_codey_gyro_board_cb[FREE_FALL].event_occured_flag = false;    
  }

  if(!codey_eve_get_start_flag_t())
  {
    return;
  }
  /* give the semaphore */
  for(uint8_t i = 0; i < EVENT_MAX; i++)
  {
    if(s_codey_gyro_board_cb[i].event_occured_flag == true)
    {
      if(i > 0 && i < 5)
      {
        if(!codey_eve_get_triggerd_flag_t(i + EVE_BOARD_SHAKE))
        {
          codey_eve_trigger_t(i + EVE_BOARD_SHAKE, NULL);
          codey_eve_set_triggerd_flag_t(i + EVE_BOARD_SHAKE, true);
        }
      }
      else if(i == 0)
      {
        codey_eve_trigger_t(i + EVE_BOARD_SHAKE, NULL);
      }
      else // other event about gyro is not used
      {
        continue;
      }
    }
  }
}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
STATIC mp_obj_t codey_gyro_board_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  // setup the object
  codey_gyro_board_obj_t *self = &s_codey_gyro_board_obj;
  self->base.type = &codey_gyro_board_type;
  return self;
}

STATIC mp_obj_t codey_gyro_board_init(mp_obj_t self_in)
{ 
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_gyro_board_init_obj, codey_gyro_board_init);

STATIC mp_obj_t codey_gyro_board_deinit(mp_obj_t self_in)
{  
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_gyro_board_deinit_obj, codey_gyro_board_deinit);

STATIC mp_obj_t codey_gyro_board_value(mp_obj_t self_in,  mp_obj_t arg1)
{
  float value = 0;  
  uint8_t axis;

  axis = mp_obj_get_int(arg1);
  if(axis == 0)
  {
    return mp_const_none; 
  }

  else if(axis == 1)
  {
    value = s_gyro_value_struct.angle_pitch;
  }
  else if(axis == 2)
  {
    value = s_gyro_value_struct.angle_yaw;
  }
  else if(axis == 3)
  {
    value = s_gyro_value_struct.angle_roll;
  }
  else
  {
    value = s_gyro_value_struct.temperature;
  }

  return mp_obj_new_float(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_gyro_board_value_obj, codey_gyro_board_value);

STATIC mp_obj_t codey_gyro_board_raw_data(mp_obj_t self_in, mp_obj_t arg1)
{
  uint8_t sensor = 0;
  sensor = mp_obj_get_int(arg1);
  int16_t temp = 0;
  uint8_t list_len = 0;
  float const_gain = 100.0 / s_gyro_value_struct.acc_earth;
  if(sensor == 1 || sensor == 2)
  {
    list_len = 3;
  }
  else if(sensor == 3)
  {
    list_len = 6;
  }
  else
  {
    return mp_const_none;
  }
  
  mp_obj_list_t *newlist = mp_obj_new_list(list_len, NULL); 
  if(sensor == 1 || sensor == 3)
  {
    temp = const_gain * s_gyro_value_struct.acc_x;
    temp = (temp > 100)? 100 : temp;
    temp = (temp < -100)? -100 : temp;
    newlist->items[0] = MP_OBJ_NEW_SMALL_INT(temp);
    temp = const_gain * s_gyro_value_struct.acc_y;
    temp = (temp > 100)? 100 : temp;
    temp = (temp < -100)? -100 : temp;
    newlist->items[1] = MP_OBJ_NEW_SMALL_INT(temp);
    temp = const_gain * s_gyro_value_struct.acc_z;
    temp = (temp > 100)? 100 : temp;
    temp = (temp < -100)? -100 : temp;
    newlist->items[2] = MP_OBJ_NEW_SMALL_INT(temp);
  }
  else if(sensor == 2)
  {
    newlist->items[0] = mp_obj_new_float(s_gyro_value_struct.angle_pitch);
    newlist->items[1] = mp_obj_new_float(s_gyro_value_struct.angle_yaw);
    newlist->items[2] = mp_obj_new_float(s_gyro_value_struct.angle_roll);
  }
  if(sensor == 3)
  { 
    newlist->items[3] = mp_obj_new_float(s_gyro_value_struct.angle_pitch);
    newlist->items[4] = mp_obj_new_float(s_gyro_value_struct.angle_yaw);
    newlist->items[5] = mp_obj_new_float(s_gyro_value_struct.angle_roll);
  }
  return (mp_obj_t)(newlist);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_gyro_board_raw_data_obj, codey_gyro_board_raw_data);

STATIC mp_obj_t codey_gyro_board_event_init(mp_obj_t self_in)
{
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_gyro_board_event_init_obj, codey_gyro_board_event_init);

STATIC mp_obj_t codey_gyro_board_is_shaked(mp_obj_t self_in)
{
  return mp_obj_new_bool(s_is_shaked);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_gyro_board_is_shaked_obj, codey_gyro_board_is_shaked);

STATIC mp_obj_t codey_gyro_board_get_tilt_status(mp_obj_t self_in)
{
  uint8_t ret_status = 0;
  for(uint8_t i = 0; i < EVENT_MAX; i++)
  {
    if(s_codey_gyro_board_cb[i].event_occured_flag == true)
    {
      ret_status |= (1 << i); 
    }
    else
    {
      ret_status &= (~(1 << i));
    }
  }
  return mp_obj_new_int(ret_status);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_gyro_board_get_tilt_status_obj, codey_gyro_board_get_tilt_status);

STATIC void codey_gyro_board_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
  
}

STATIC mp_obj_t codey_gyro_board_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  return mp_const_none;
}

STATIC const mp_map_elem_t codey_gyro_board_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_value),               (mp_obj_t)&codey_gyro_board_value_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_raw_data),            (mp_obj_t)&codey_gyro_board_raw_data_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_init),                (mp_obj_t)&codey_gyro_board_init_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),              (mp_obj_t)&codey_gyro_board_deinit_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_is_shaked),           (mp_obj_t)&codey_gyro_board_is_shaked_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_tilt),                (mp_obj_t)&codey_gyro_board_get_tilt_status_obj },  
 
  { MP_OBJ_NEW_QSTR(MP_QSTR_event_init),          (mp_obj_t)&codey_gyro_board_event_init_obj },
};

STATIC MP_DEFINE_CONST_DICT(codey_gyro_board_locals_dict, codey_gyro_board_locals_dict_table);

const mp_obj_type_t codey_gyro_board_type =
{
  { &mp_type_type },
  .name = MP_QSTR_gyro_board,
  .print = codey_gyro_board_print,
  .call = codey_gyro_board_call,
  .make_new = codey_gyro_board_make_new,
  .locals_dict = (mp_obj_t)&codey_gyro_board_locals_dict,
};


