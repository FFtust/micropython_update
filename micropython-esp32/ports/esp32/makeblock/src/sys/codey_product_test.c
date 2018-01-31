/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief       product test
 * @file        codey_product_test.c
 * @author      fftust
 * @version     V1.0.0
 * @date        2017/11/21
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
 * fftust             2017/11/21       1.0.0              Build the new.
 * </pre>
 *
 */

/* this file wil be deleted later  
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

#include "codey_comm_protocol.h"
#include "codey_sensor_value_update.h"
#include "codey_product_test.h"
#include "codey_h_class.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/ 
#undef    TAG
#define   TAG                           ("product_test")
#define   AD_VALUE_TO_100               (100.0 / 4095)
#define   TIME_TO_MINUTE                (1.0 / 1000)
#define   ACC_OF_GRAVITY                (9.8)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/  
typedef enum
{
  PT_SENSOR_BUTTON = 1,
  PT_SENSOR_DAIL,
  PT_SENSOR_LIGHT,
  PT_SENSOR_SOUND,
  PT_BATTERY_VOLTAGE,
  PT_SENSOE_IR_REC,
  PT_SENSOR_GYRO,
  PT_SENSOR_SPEAKER,
  PT_SENSOR_RGB,
  PT_SENSOR_LEDMATRIX,
  PT_SENSOR_IR_SEND,
  PT_SENSOR_MAX // 0x0c
}codey_pt_sensor_id;

typedef enum
{
  BUTTON_A = 1,
  BUTTON_B,
  BUTTON_C,
  BUTTON_POWER,
  BUTTON_MAX
}codey_pt_sensor_keys_id;

typedef enum
{
  GYRO_ACC_X = 1,
  GYRO_ACC_Y,
  GYRO_ACC_Z,
  GYRO_PITCH,
  GYRO_ROLL,
  GYRO_YAW,
  GYRO_DATA_MAX
}codey_pt_sensor_gyro_value_id;

typedef void (*p_pt_command_ope_handle_t)(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);

/******************************************************************************
 DECLARE PRIVATE DATAS
 ******************************************************************************/
static uint8_t s_product_test_enable = 0;

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC void codey_product_test_get_button(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
STATIC void codey_product_test_get_dail(channel_data_tag_t chn,uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
STATIC void codey_product_test_get_light(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
STATIC void codey_product_test_get_sound(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
STATIC void codey_product_test_get_battery(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
STATIC void codey_product_test_get_ir_rec(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
STATIC void codey_product_test_get_gyro(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
STATIC void codey_product_test_set_speaker(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
STATIC void codey_product_test_set_rgb(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
STATIC void codey_product_test_set_ledmatrix(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
STATIC void codey_product_test_ir_send(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
STATIC void codey_product_test_set_status(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);

p_pt_command_ope_handle_t p_pt_command_ope_handle_table[PT_SENSOR_MAX] = 
{
  [0] = codey_product_test_set_status,
  [1] = codey_product_test_get_button,
  [2] = codey_product_test_get_dail,
  [3] = codey_product_test_get_light,
  [4] = codey_product_test_get_sound,
  [5] = codey_product_test_get_battery,  
  [6] = codey_product_test_get_ir_rec,
  [7] = codey_product_test_get_gyro,
  [8] = codey_product_test_set_speaker,
  [9] = codey_product_test_set_rgb,
  [10] = codey_product_test_set_ledmatrix,
  [11] = codey_product_test_ir_send,
};

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_product_test_request(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  // ESP_LOGI(TAG, "****in product test***"); 
  if(!s_product_test_enable && data[0] != 0)
  {
    ESP_LOGE(TAG, "product test command is not enabled");
    *output_len = 0;
    return;
  }
  if(!output_len || !output_buf)
  {
    // ESP_LOGE(TAG, "output buffer or len address is NULL");
    *output_len = 0;
    return;
  }
  
  uint8_t sensor_id = 0;
  sensor_id = data[0];
  // ESP_LOGI(TAG, "****sensor id is %d", sensor_id);
  
  if(sensor_id >= PT_SENSOR_MAX)
  {
    // ESP_LOGE(TAG, "sensor is not existed");
    *output_len = 0;
    return;
  }
  
  if(!p_pt_command_ope_handle_table[sensor_id])
  {
    // ESP_LOGE(TAG, "sensor operation handle is NULL");
    *output_len = 0;
    return;
  }
  else
  {
    p_pt_command_ope_handle_table[sensor_id](chn, data, len, output_buf, output_len);
  }
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC void codey_product_test_set_status(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint8_t index = 0;
  /* check magic number */
  if(data[1] == 1)
  {
    s_product_test_enable = 1;
  }
  else if(data[1] == 0)
  {
    s_product_test_enable = 0;
  }
  output_buf[index++] = data[0];
  output_buf[index++] = data[1];
  output_buf[index++] = 1;

  *output_len = index;
}

STATIC void codey_product_test_get_button(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint8_t button_id = data[1];
  uint8_t index = 0;
  if(button_id > BUTTON_MAX)
  {
    ESP_LOGE(TAG, "button id is not existed");
    *output_len = 0;
    return;
  }
  
  output_buf[index++] = data[0];
  output_buf[index++] = data[1];
  output_buf[index++] = (uint8_t)codey_is_button_pressed_t( button_id);
  *output_len = index;
}

STATIC void codey_product_test_get_dail(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint8_t index = 0;
  
  output_buf[index++] = data[0];
  output_buf[index++] = (uint8_t)( codey_potentionmeter_board_read_value_t() * AD_VALUE_TO_100);
  *output_len = index;
}

STATIC void codey_product_test_get_light(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint8_t index = 0;
 
  output_buf[index++] = data[0];
  output_buf[index++] = (uint8_t)( codey_light_sensor_board_read_value_t() * AD_VALUE_TO_100);
  *output_len = index;

}

STATIC void codey_product_test_get_sound(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint8_t index = 0;
 
  output_buf[index++] = data[0];
  output_buf[index++] = (uint8_t)( codey_sound_sensor_board_read_value_t() * AD_VALUE_TO_100);
  *output_len = index;
}

STATIC void codey_product_test_get_battery(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint8_t index = 0;
  float val = 0.0;
  output_buf[index++] = data[0];
  val = codey_battery_check_get_voltage_t();
  // printf("battery vol is %f\n", val);
  memcpy(&output_buf[index], &val, sizeof( float));
  // printf("float bytes is %d, %d, %d, %d\n", output_buf[1], output_buf[2], output_buf[3], output_buf[4]);
  index += 4;
  *output_len = index;
}
 
STATIC void codey_product_test_get_ir_rec(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  extern uint8_t codey_rmt_board_get_single_char_t();
  
  uint8_t index = 0;
  uint8_t val = 0;
  
  output_buf[index++] = data[0];
  val = codey_rmt_board_get_single_char_t();
  output_buf[index++] = val;
  *output_len = index; 
}

STATIC void codey_product_test_get_gyro(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  extern float codey_gyro_sensor_value_t(uint8_t index);
  uint8_t index = 0;
  uint8_t val_index = data[1];
  
  // ESP_LOGI(TAG, "val index is %d", val_index);
  if(val_index >= GYRO_DATA_MAX)
  {
    ESP_LOGE(TAG, "GYRO data id is not existed");
    *output_len = 0;
    return;
  }
  float val = codey_gyro_sensor_value_t(val_index);
  
  output_buf[index++] = data[0];
  output_buf[index++] = data[1];
  memcpy(&output_buf[index], &val, sizeof( float));
  index += 4;
  *output_len = index;
}

STATIC void codey_product_test_set_speaker(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  extern void codey_voice_stop_play();
  extern bool codey_voice_play_file_to_stop(const char*);
  extern bool codey_voice_play_note(uint32_t, uint32_t, uint8_t, uint8_t);
  uint8_t ope_id = data[1];
  if(ope_id == 1)
  {
    codey_voice_stop_play();
    codey_voice_play_file_to_stop("/music/cat.wav");
  }
  else if(ope_id == 2)
  {
    codey_voice_stop_play();
    codey_voice_play_file_to_stop("/music/dog.wav");   
  }
  else if(ope_id == 3)
  {
    codey_voice_stop_play();
    codey_voice_play_note(523, 1000, 0, 30);
  }
}

STATIC void codey_product_test_set_rgb(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint8_t r = data[1];
  uint8_t g = data[2];
  uint8_t b = data[3];
  
  codey_rgbled_board_set_color_t(r, g, b);
  
  *output_len = 0;
}

STATIC void codey_product_test_set_ledmatrix(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint8_t led_all_on = data[1]; 
  if(led_all_on == 1)
  {
    uint8_t led_data[16];
    memset(led_data, 0xff, 16);
    codey_ledmatrix_show_faceplate_t(led_data);
  }
  else if(led_all_on == 2)
  {
    codey_ledmatrix_screen_clean_t();  
  }
  *output_len = 0;
}

STATIC void codey_product_test_ir_send(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint16_t val = (uint16_t)data[1];
  codey_rmt_board_send_t(0x0000, 1, &val);
  *output_len = 0;
}
