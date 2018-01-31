/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for ledmatrix module
 * @file    codey_ledmatrix_board.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/10/03
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
 * This file is a drive ledmatrix_board module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/10/03      1.0.0              build the new.
 * </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/mphal.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_heap_caps.h"

#include "codey_ledmatrix.h"
#include "codey_config.h"
#include "codey_sys_operation.h"
#include "codey_sys.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG  ("codey_ledmatrix")
#define LED_DATA_WRITE_ENTER xSemaphoreTake(led_data_write_sema, portMAX_DELAY);
#define LED_DATA_WRITE_EXIT  xSemaphoreGive(led_data_write_sema);

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/ 
typedef struct
{
  mp_obj_base_t base; 
} codey_ledmatrix_obj_t;
 
const led_matrix_charfont_typedef codey_char_font[] =
{
  {{' '},  {0x00,0x00,0x00,0x00,0x00,0x00}},

  {{'0'},  {0x00,0x7C,0x82,0x82,0x7C,0x00}}, 
  {{'1'},  {0x00,0x42,0xFE,0x02,0x00,0x00}},
  {{'2'},  {0x00,0x46,0x8A,0x92,0x62,0x00}},
  {{'3'},  {0x00,0x44,0x92,0x92,0x6C,0x00}},  
  {{'4'},  {0x00,0x1C,0x64,0xFE,0x04,0x00}},
  {{'5'},  {0x00,0xF2,0x92,0x92,0x8C,0x00}},
  {{'6'},  {0x00,0x7C,0x92,0x92,0x4C,0x00}},
  {{'7'},  {0x00,0xC0,0x8E,0x90,0xE0,0x00}},
  {{'8'},  {0x00,0x6C,0x92,0x92,0x6C,0x00}},
  {{'9'},  {0x00,0x64,0x92,0x92,0x7C,0x00}},

  {{'a'},  {0x00,0x04,0x2A,0x2A,0x1E,0x00}},
  {{'b'},  {0x00,0xFE,0x12,0x12,0x0C,0x00}},
  {{'c'},  {0x00,0x0C,0x12,0x12,0x12,0x00}},
  {{'d'},  {0x00,0x0C,0x12,0x12,0xFE,0x00}},
  {{'e'},  {0x00,0x1C,0x2A,0x2A,0x18,0x00}},
  {{'f'},  {0x00,0x10,0x3E,0x50,0x50,0x00}},
  {{'g'},  {0x00,0x08,0x15,0x15,0x1E,0x00}},
  {{'h'},  {0x00,0xFE,0x10,0x1E,0x00,0x00}},
  {{'i'},  {0x00,0x00,0x2E,0x00,0x00,0x00}},
  {{'j'},  {0x00,0x02,0x01,0x2E,0x00,0x00}},
  {{'k'},  {0x00,0xFE,0x08,0x14,0x12,0x00}},
  {{'l'},  {0x00,0x00,0xFE,0x02,0x00,0x00}},
  {{'m'},  {0x00,0x0E,0x10,0x0E,0x10,0x0E}},
  {{'n'},  {0x00,0x1E,0x10,0x10,0x0E,0x00}},
  {{'o'},  {0x00,0x0C,0x12,0x12,0x0C,0x00}},
  {{'p'},  {0x00,0x1F,0x12,0x12,0x0C,0x00}},
  {{'q'},  {0x00,0x0C,0x12,0x12,0x1F,0x00}},
  {{'r'},  {0x00,0x1E,0x08,0x10,0x10,0x00}},
  {{'s'},  {0x00,0x12,0x29,0x25,0x12,0x00}},
  {{'t'},  {0x00,0x10,0x3E,0x12,0x00,0x00}},
  {{'u'},  {0x00,0x1C,0x02,0x02,0x1E,0x00}},
  {{'v'},  {0x18,0x04,0x02,0x04,0x18,0x00}},
  {{'w'},  {0x18,0x06,0x1C,0x06,0x18,0x00}},
  {{'x'},  {0x00,0x12,0x0C,0x0C,0x12,0x00}},
  {{'y'},  {0x00,0x18,0x05,0x05,0x1E,0x00}},
  {{'z'},  {0x00,0x12,0x16,0x1A,0x12,0x00}},

  {{'A'},  {0x00,0x7E,0x88,0x88,0x7E,0x00}},
  {{'B'},  {0x00,0xFE,0x92,0x92,0x6C,0x00}},
  {{'C'},  {0x00,0x7C,0x82,0x82,0x44,0x00}},
  {{'D'},  {0x00,0xFE,0x82,0x82,0x7C,0x00}},
  {{'E'},  {0x00,0xFE,0x92,0x92,0x82,0x00}},
  {{'F'},  {0x00,0xFE,0x90,0x90,0x80,0x00}},
  {{'G'},  {0x00,0x7C,0x82,0x92,0x5C,0x00}},
  {{'H'},  {0x00,0xFE,0x10,0x10,0xFE,0x00}},
  {{'I'},  {0x00,0x82,0xFE,0x82,0x00,0x00}},
  {{'J'},  {0x00,0x0C,0x02,0x02,0xFC,0x00}},
  {{'K'},  {0x00,0xFE,0x10,0x28,0xC6,0x00}},
  {{'L'},  {0x00,0xFE,0x02,0x02,0x02,0x00}},
  {{'M'},  {0x00,0xFE,0x40,0x30,0x40,0xFE}},
  {{'N'},  {0x00,0xFE,0x40,0x30,0x08,0xFE}},
  {{'O'},  {0x00,0x7C,0x82,0x82,0x82,0x7C}},
  {{'P'},  {0x00,0xFE,0x90,0x90,0x60,0x00}},
  {{'Q'},  {0x00,0x7C,0x82,0x8A,0x84,0x7A}},
  {{'R'},  {0x00,0xFE,0x98,0x94,0x62,0x00}},
  {{'S'},  {0x00,0x64,0x92,0x92,0x4C,0x00}},
  {{'T'},  {0x00,0x80,0xFE,0x80,0x80,0x00}},
  {{'U'},  {0x00,0xFC,0x02,0x02,0xFC,0x00}},
  {{'V'},  {0x00,0xF0,0x0C,0x02,0x0C,0xF0}},
  {{'W'},  {0x00,0xFE,0x04,0x38,0x04,0xFE}},
  {{'X'},  {0x00,0xC6,0x38,0x38,0xC6,0x00}},
  {{'Y'},  {0xC0,0x20,0x1E,0x20,0xC0,0x00}},
  {{'Z'},  {0x00,0x86,0x9A,0xB2,0xC2,0x00}},
  {{','},  {0x00,0x01,0x0e,0x0c,0x00,0x00}},
  {{'.'},  {0x00,0x00,0x06,0x06,0x00,0x00}},
  {{'%'},  {0x72,0x54,0x78,0x1e,0x2a,0x4e}},
  {{'!'},  {0x00,0x00,0x7a,0x00,0x00,0x00}},
  {{'?'},  {0x00,0x20,0x4a,0x30,0x00,0x00}},
  {{'-'},  {0x00,0x10,0x10,0x10,0x10,0x00}},
  {{'+'},  {0x08,0x08,0x3e,0x08,0x08,0x00}},
  {{'/'},  {0x00,0x02,0x0c,0x30,0x40,0x00}},
  {{'*'},  {0x22,0x14,0x08,0x14,0x22,0x00}},
  {{':'},  {0x00,0x00,0x14,0x00,0x00,0x00}},
  {{'"'},  {0x00,0xC0,0x00,0xC0,0x00,0x00}},
  {{'#'},  {0x28,0xFE,0x28,0xFE,0x28,0x00}},
  {{'('},  {0x00,0x00,0x7C,0x82,0x00,0x00}},
  {{')'},  {0x00,0x00,0x82,0x7C,0x00,0x00}},
  {{';'},  {0x00,0x02,0x24,0x00,0x00,0x00}},
  {{'~'},  {0x00,0x40,0x80,0x40,0x80,0x00}},
  {{';'},  {0x00,0x02,0x24,0x00,0x00,0x00}},
  {{'='},  {0x00,0x28,0x28,0x28,0x28,0x00}},
  {{'|'},  {0x00,0x00,0xFE,0x00,0x00,0x00}},
  {{'>'},  {0x00,0x82,0x44,0x28,0x10,0x00}},
  {{'<'},  {0x00,0x10,0x28,0x44,0x82,0x00}},  
  {{'@'},  {0x00,0xff,0xff,0xff,0xff,0x00}},
 
};  //85

const led_matrix_numberfont_typedef codey_number_font[] =
{
  {{'0'}, {0x3e, 0x22, 0x3e}},      // 0
  {{'1'}, {0x00, 0x3e, 0x00}},       
  {{'2'}, {0x3a, 0x2a, 0x2e}},
  {{'3'}, {0x2a, 0x2a, 0x3e}},
  {{'4'}, {0x0e, 0x08, 0x3e}},
  {{'5'}, {0x2e, 0x2a, 0x3a}},
  {{'6'}, {0x3e, 0x2a, 0x3a}},
  {{'7'}, {0x02, 0x02, 0x3e}},
  {{'8'}, {0x3e, 0x2a, 0x3e}},
  {{'9'}, {0x2e, 0x2a, 0x3e}},      // 9
  {{':'}, {0x00, 0x22, 0x00}},      // :
  {{'.'}, {0x00, 0x20, 0x00}},      // .
  {{'-'}, {0x08, 0x08, 0x08}},      // -
  {{'+'}, {0x00, 0x00, 0x00}},      // 
};

const animation_struct_in_flash_t animation_test = 
{
  .len = 22,
  .index = 1,
  .interval = 40,
  .loop_times = 1,
  .animation_data = 
  {
    {255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {219, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {85, 219, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {74, 165, 219, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 146, 165, 219, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 74, 165, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 69, 165, 219, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 74, 165, 219, 255, 255, 255, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 74, 165, 219, 255, 255, 255, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 41, 165, 219, 255, 255, 255, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 74, 165, 219, 255, 255, 255, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 84, 165, 219, 255, 255, 255, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 145, 165, 203, 255, 255, 255, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 74, 165, 219, 255, 255, 255},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 164, 165, 219, 255, 255},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 82, 165, 219, 255},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 73, 165, 219},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 82, 165},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  },
};

const animation_struct_in_flash_t animation_happy = 
{
  .len = 3,
  .index = 2,
  .interval = 200,
  .loop_times = 10,
  .animation_data = 
  {
    {0x00, 0x00, 0x18, 0x30, 0x30, 0x38, 0x18, 0x00, 0x00, 0x18, 0x38, 0x30, 0x30, 0x18, 0x00, 0x00},
    {0x00, 0x00, 0x30, 0x60, 0x60, 0x70, 0x30, 0x00, 0x00, 0x30, 0x70, 0x60, 0x60, 0x30, 0x00, 0x00},
    {0x00, 0x00, 0x18, 0x30, 0x30, 0x38, 0x18, 0x00, 0x00, 0x18, 0x38, 0x30, 0x30, 0x18, 0x00, 0x00}, 
  },
};

const animation_struct_in_flash_t animation_cry = 
{
  .len = 3,
  .index = 2,
  .interval = 200,
  .loop_times = 10,
  .animation_data = 
  {
    {0x00, 0x10, 0x2b, 0x2b, 0x28, 0x28, 0x10, 0x00, 0x00, 0x10, 0x28, 0x2d, 0x2d, 0x28, 0x10, 0x00},
    {0x00, 0x10, 0x2d, 0x2d, 0x28, 0x28, 0x10, 0x00, 0x00, 0x10, 0x28, 0x2e, 0x2e, 0x28, 0x10, 0x00},
    {0x00, 0x10, 0x2e, 0x2e, 0x28, 0x28, 0x10, 0x00, 0x00, 0x10, 0x28, 0x2b, 0x2b, 0x28, 0x10, 0x00}, 
  },
};


const animation_struct_in_flash_t animation_dispirited = 
{
  .len = 3,
  .index = 2,
  .interval = 200,
  .loop_times = 10,
  .animation_data =
  {
    {0x00, 0x02, 0x07, 0x07, 0x0f, 0x0e, 0x08, 0x00, 0x00, 0x08, 0x0e, 0x0f, 0x07, 0x07, 0x02, 0x00},
    {0x00, 0x04, 0x0e, 0x0e, 0x1e, 0x1c, 0x10, 0x00, 0x00, 0x10, 0x1c, 0x1e, 0x0e, 0x0e, 0x04, 0x00},
    {0x00, 0x04, 0x0e, 0x0e, 0x1e, 0x1c, 0x10, 0x00, 0x00, 0x10, 0x1c, 0x1e, 0x0e, 0x0e, 0x04, 0x00}, 
  },
};

const animation_struct_in_flash_t animation_angry = 
{
  .len = 3,
  .index = 2,
  .interval = 200,
  .loop_times = 10,
  .animation_data = 
  {
    {0x00, 0x00, 0x3c, 0x1e, 0x0e, 0x04, 0x00, 0x00, 0x00, 0x00, 0x04, 0x0e, 0x1e, 0x3c, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x3c, 0x1e, 0x0e, 0x04, 0x00, 0x00, 0x04, 0x0e, 0x1e, 0x3c, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x3c, 0x1e, 0x0e, 0x04, 0x00, 0x00, 0x00, 0x00, 0x04, 0x0e, 0x1e, 0x3c, 0x00, 0x00}, 
  },
};
 
const animation_struct_in_flash_t animation_fear = 
{
  .len = 3,
  .index = 2,
  .interval = 200,
  .loop_times = 10,
  .animation_data = 
  {
    {0x00, 0x00, 0x18, 0x3c, 0x3c, 0x7c, 0x78, 0x00, 0x00, 0x78, 0x7c, 0x3c, 0x3c, 0x18, 0x00, 0x00},
    {0x00, 0x00, 0x0c, 0x1e, 0x1e, 0x3e, 0x3c, 0x00, 0x00, 0x3c, 0x3e, 0x1e, 0x1e, 0x0c, 0x00, 0x00},
    {0x00, 0x00, 0x18, 0x3c, 0x3c, 0x7c, 0x78, 0x00, 0x00, 0x78, 0x7c, 0x3c, 0x3c, 0x18, 0x00, 0x00}, 
  },
};

const animation_struct_in_flash_t *s_animation_lib_list[10] = 
{
  &animation_test, &animation_happy, &animation_cry, 
  &animation_dispirited, &animation_angry, &animation_fear, 
  NULL, NULL, NULL, NULL
};

typedef void(*ledmatrix_sequence_show_cb_t)(uint16_t, uint8_t *);

typedef struct
{
  uint8_t cur_matrix_data[LED_BUFFER_SIZE];
  uint8_t show_count;
  uint8_t show_type;
  uint8_t show_out_flag;
  
  uint8_t animation_index;
  animation_struct_in_flash_t animation_struct;
  
  char    string_data[STRING_CHARS_NUM_MAX];
  uint8_t string_len;
  
  SemaphoreHandle_t led_seq_sema;
  ledmatrix_sequence_show_cb_t ledmatrix_sequence_show_cb;
}codey_ledmatrix_manager_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
/* this buffer store the current faceplate */
static codey_ledmatrix_obj_t s_codey_ledmatrix_obj = {.base = {&codey_ledmatrix_type}};
static bool s_codey_ledmatrix_init_flag = false;
static bool s_codey_ledmatrix_seq_show_out = false;
static bool s_codey_ledmatrix_show_enable = false;
static SemaphoreHandle_t led_data_write_sema = NULL;

codey_ledmatrix_manager_t s_codey_ledmatrix_manager;

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/ 
STATIC  void    codey_digital_write_delay(void);
STATIC  void    codey_digital_write(gpio_num_t io_num , wire_digital_status sta);
STATIC  void    codey_digital_write_byte(uint8_t data);
STATIC  void    codey_digital_writebytestoaddress(uint8_t Address, const uint8_t *P_data, uint8_t count_of_data);
STATIC  uint8_t codey_char_font_invert(uint8_t data);
STATIC void     codey_lematrix_data_buffer_clear_t(void);
STATIC uint8_t  codey_lematrix_data_buffer_add_t(int16_t colume_index, uint8_t *data, uint16_t len);
STATIC uint8_t  codey_lematrix_data_buffer_add_from_tail_t(int16_t colume_index, uint8_t *data, uint16_t len);
STATIC void     codey_lematrix_data_buffer_show_t(void);
STATIC void     codey_ledmatrix_screen_invert_t(void);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_ledmatrix_show_enable(void)
{
  s_codey_ledmatrix_show_enable = true;
}

void codey_ledmatrix_show_disable(void)
{
  s_codey_ledmatrix_show_enable = false;
}

bool codey_ledmatrix_show_is_enabled(void)
{
  return s_codey_ledmatrix_show_enable;
}

void codey_ledmatrix_initialize_t(void)
{
  if(s_codey_ledmatrix_init_flag == false)
  {
    memset(&s_codey_ledmatrix_manager, 0 , sizeof(codey_ledmatrix_manager_t)); 
    s_codey_ledmatrix_manager.led_seq_sema = xSemaphoreCreateBinary();
    xSemaphoreTake(s_codey_ledmatrix_manager.led_seq_sema, 0);
    led_data_write_sema = xSemaphoreCreateBinary();
    xSemaphoreGive(led_data_write_sema);
    codey_ledmatrix_board_config_t();
    codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_HIGH);
    codey_digital_write(WIRE_DIGITAL_SDA_IO, WIRE_DIGITAL_HIGH);
    codey_digital_write_byte(Mode_Address_Auto_Add_1);
    codey_Ledmatrix_setbrightness_t(BRIGHTNESS_8);
    codey_lematrix_data_buffer_clear_t();
    codey_lematrix_data_buffer_show_t();
    codey_ledmatrix_show_enable();
    ESP_LOGD(TAG, "ledmatrix init succeed");
    s_codey_ledmatrix_init_flag = true;
  }
}

void codey_ledmatrix_board_config_t(void)  
{
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;      
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1 << WIRE_DIGITAL_SCL_IO) ;//GPIO_OUTPUT_PIN_SEL;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf);
  
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;       
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1 << WIRE_DIGITAL_SDA_IO) ;//GPIO_OUTPUT_PIN_SEL;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf); 
}

void codey_ledmatrix_screen_clean_t(void)
{
  s_codey_ledmatrix_manager.show_out_flag = true;
  
  while(s_codey_ledmatrix_manager.show_count)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  codey_lematrix_data_buffer_clear_t();
  codey_lematrix_data_buffer_show_t();
}

void codey_ledmatrix_reload_t(void)
{
  s_codey_ledmatrix_manager.show_out_flag = true;
 
  while(s_codey_ledmatrix_manager.show_count)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  codey_lematrix_data_buffer_show_t();

}

void codey_Ledmatrix_setbrightness_t(uint8_t brightness)
{
  if((uint8_t)brightness > 8)
  {
    brightness = BRIGHTNESS_8;
  }

  if((uint8_t)brightness != 0)
  {
    brightness = (led_matrix_brightness_typedef)((uint8_t)(brightness - 1) | 0x08);  
  }
  codey_digital_write_byte(0x80 | (uint8_t)brightness);
}

void codey_ledmatrix_show_faceplate_t(const uint8_t *data)
{
  uint8_t char_data_inver[LED_BUFFER_SIZE];
  
  for(uint8_t k = 0; k < LED_BUFFER_SIZE; k++)
  {
    char_data_inver[k] = codey_char_font_invert(data[k]); 
  }
  
  if(!memcmp(char_data_inver, s_codey_ledmatrix_manager.cur_matrix_data, LED_BUFFER_SIZE))
  {
    return;
  }
  
  s_codey_ledmatrix_manager.show_out_flag = true;
  while(s_codey_ledmatrix_manager.show_count)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  
  memcpy(s_codey_ledmatrix_manager.cur_matrix_data, char_data_inver, LED_BUFFER_SIZE);
  codey_digital_writebytestoaddress(0, char_data_inver, LED_BUFFER_SIZE);
}

/* can be called only in python apis */
void codey_ledmatrix_show_faceplate_with_time_t(const uint8_t *data, uint32_t time_ms)
{
  uint8_t char_data_inver[LED_BUFFER_SIZE];
  uint32_t start_time = 0;
  for(uint8_t k = 0; k < LED_BUFFER_SIZE; k++)
  {
    char_data_inver[k] = codey_char_font_invert(data[k]); 
  }
  
  if(!memcmp(char_data_inver, s_codey_ledmatrix_manager.cur_matrix_data, LED_BUFFER_SIZE))
  {
    return;
  }
  
  s_codey_ledmatrix_manager.show_out_flag = true;
  while(s_codey_ledmatrix_manager.show_count)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  s_codey_ledmatrix_manager.show_out_flag = false;
  memcpy(s_codey_ledmatrix_manager.cur_matrix_data, char_data_inver, LED_BUFFER_SIZE);
  codey_digital_writebytestoaddress(0, char_data_inver, LED_BUFFER_SIZE);
  start_time = millis();
  while((millis() - start_time) < time_ms)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
    if(s_codey_ledmatrix_manager.show_out_flag == true)
    {
      break;
    }
  }
  codey_lematrix_data_buffer_clear_t();
  codey_lematrix_data_buffer_show_t();
}

/* status should be 0(off), or 1(on) */
void codey_ledmatrix_show_pixel_t(int16_t pos_x, int16_t pos_y, bool status)
{
  if(pos_x > 15 || pos_y > 7 || pos_x < 0 || pos_y < 0)
  {
    return;
  }

  s_codey_ledmatrix_manager.show_out_flag = true;
  while(s_codey_ledmatrix_manager.show_count)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  
  if(status == 1)
  {
    s_codey_ledmatrix_manager.cur_matrix_data[pos_x] |= (uint8_t)(pow(2, pos_y)); 
  }
  else
  {
    s_codey_ledmatrix_manager.cur_matrix_data[pos_x] &= 255 - (uint8_t)pow(2, pos_y);
  }        
  codey_lematrix_data_buffer_show_t();
}

void codey_ledmatrix_show_animation_t(uint16_t animation_index, uint8_t *animation_data)
{
  /* read the animation data from flash rely on animation index firstly*/

  /* then send the data to buffer */
  uint8_t char_data_inver[LED_BUFFER_SIZE] = {0};
  bool break_flag = false;
  s_codey_ledmatrix_manager.show_out_flag = true;
  while(s_codey_ledmatrix_manager.show_count)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  s_codey_ledmatrix_manager.show_out_flag = false;
  s_codey_ledmatrix_manager.show_count = 1;

  for(uint8_t t = 0; t < s_animation_lib_list[animation_index]->loop_times; t++)
  {
    for(uint8_t i = 0; i < s_animation_lib_list[animation_index]->len; i++)
    {
      for(uint8_t k = 0; k < LED_BUFFER_SIZE; k++)
      {
        char_data_inver[k] = codey_char_font_invert(s_animation_lib_list[animation_index]->animation_data[i][k]); 
      }
      memcpy(s_codey_ledmatrix_manager.cur_matrix_data, char_data_inver, LED_BUFFER_SIZE);
      codey_lematrix_data_buffer_show_t();
      if(s_codey_ledmatrix_manager.show_out_flag == true)
      {
        break_flag = true;
        break;
      }
      vTaskDelay(s_animation_lib_list[animation_index]->interval / portTICK_PERIOD_MS);
    }
    if(break_flag == true)
    {
      goto animation_out;
    }
  }
  codey_lematrix_data_buffer_clear_t();
  codey_lematrix_data_buffer_show_t();
animation_out:
  s_codey_ledmatrix_manager.show_count = 0;
}

void codey_ledmatrix_draw_chars_t(int16_t p_x, int16_t p_y, uint8_t charnum, const char *chardata)
{
  uint16_t index = 0;
  uint8_t char_data_inver[LED_MATRIX_CHARFONT_COLUMN];
  
  codey_lematrix_data_buffer_clear_t();
  for(uint8_t n = 0; n < charnum; n++)
  {
    for(index = 0; index < LED_MATRIX_CHAR_TABLE_LEN; index++)
    {    
      if(chardata[n] == codey_char_font[index].character[0])
      {
        break;
      }
    }
    
    if(index >= LED_MATRIX_CHAR_TABLE_LEN)
    { 
      index = LED_MATRIX_CHAR_TABLE_LEN - 1;
    }

    for(uint8_t k = 0; k < LED_MATRIX_CHARFONT_COLUMN; k++)
    {
      char_data_inver[k] = (char_data_inver[k] & (0xff << (8 - p_y))) | (p_y > 0 ? (codey_char_font[index].data[k] >> p_y) : (codey_char_font[index].data[k] << (-p_y)));
      char_data_inver[k] = codey_char_font_invert(codey_char_font[index].data[k]); 
    }
    
    const uint8_t *tempdata = char_data_inver;
    if(codey_lematrix_data_buffer_add_t(p_x + n * LED_MATRIX_CHARFONT_COLUMN, (uint8_t *)tempdata, LED_MATRIX_CHARFONT_COLUMN))
    { 
      codey_lematrix_data_buffer_show_t();
      return;
    }
  }  
  codey_lematrix_data_buffer_show_t();
}

void codey_ledmatrix_show_string_t(uint16_t len, uint8_t *data)
{ 
  int16_t pos_x = 0;
  int16_t pos_y = 0;
  if(len == 0)
  {
    return;
  }

  s_codey_ledmatrix_manager.show_out_flag = true;
  while(s_codey_ledmatrix_manager.show_count)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  
  if(len <= 2)
  {
    codey_ledmatrix_draw_chars_t(pos_x, pos_y, len, (const char *)data);
    // ESP_LOGI(TAG, "len < 2 out");
  }
  else
  {
    s_codey_ledmatrix_manager.show_out_flag = false;
    s_codey_ledmatrix_manager.show_count = 1;
    for(uint16_t i = 0; i < len * LED_MATRIX_CHARFONT_COLUMN + 1; i++)
    {
      codey_ledmatrix_draw_chars_t(pos_x, pos_y, len, (const char *)data);
      if(s_codey_ledmatrix_manager.show_out_flag == true)
      {
        break;
      }
      vTaskDelay(LED_MATRIX_STRING_SHOW_INTERVAL / portTICK_PERIOD_MS);
      pos_x -= LED_MATRIX_STRING_SHOW_SPEED;

    }
    s_codey_ledmatrix_manager.show_count = 0;
  }  
}

void codey_ledmatrix_show_number_t(uint16_t len, uint8_t *data)
{
  uint8_t index = 0;
  uint8_t column_len = 0;
  uint8_t column_index = 0;
  const uint8_t *tempdata = NULL;
  
  s_codey_ledmatrix_manager.show_out_flag = true;
  while(s_codey_ledmatrix_manager.show_count)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  
  codey_lematrix_data_buffer_clear_t();
  for(uint8_t n = 0; n < len; n++)
  {
    for(index = 0; index < LED_MATRIX_NUMBER_TABLE_LEN; index++)
    {    
      if(data[n] == codey_number_font[index].character[0])
      {
        break;
      }
    }
    
    if(index >= LED_MATRIX_NUMBER_TABLE_LEN)
    { 
      return;
    }
     
    if(*(data + n) == '.')
    {
      if(column_index >= LEDMATRIX_COLUMN_NUM - LED_MATRIX_NUMBERFONT_COLUMN)
      {
        break;
      }
      tempdata = &codey_number_font[index].data[1];
      column_len = 1;
      column_index -= 1;
    }
    else
    {
      tempdata = codey_number_font[index].data;
      column_len = LED_MATRIX_NUMBERFONT_COLUMN;
    }
    
    if(codey_lematrix_data_buffer_add_t(column_index, (uint8_t *)tempdata, column_len))
    {
      codey_lematrix_data_buffer_show_t();
      return;
    }
    column_index += column_len + 1; // add one space
  }  
  codey_lematrix_data_buffer_show_t();
}

void codey_ledmatrix_sequence_show_cb_register_t(uint8_t show_type, ledmatrix_sequence_show_cb_t cb)
{
  s_codey_ledmatrix_manager.show_out_flag = true;
  while(s_codey_ledmatrix_manager.show_count)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  s_codey_ledmatrix_manager.show_out_flag = false;
  
  s_codey_ledmatrix_manager.ledmatrix_sequence_show_cb = cb;
  s_codey_ledmatrix_manager.show_type = show_type;
}

void codey_ledmatrix_board_show_task_t(void *parameter)
{
  while(!codey_task_get_enable_flag_t(CODEY_LEDMATRIX_SHOW_TASK_ID))
  {
    vTaskDelay(10);
  }
  
  while(1)
  { 
    codey_task_set_status_t(CODEY_LEDMATRIX_SHOW_TASK_ID, CODEY_TASK_WAIT_TO_EXECUTE);
    if(codey_task_get_enable_flag_t(CODEY_LEDMATRIX_SHOW_TASK_ID))
    {
      if(s_codey_ledmatrix_manager.led_seq_sema == NULL)
      {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        continue;
      }
      if(xSemaphoreTake(s_codey_ledmatrix_manager.led_seq_sema, 100) == true)
      {
        codey_task_set_status_t(CODEY_LEDMATRIX_SHOW_TASK_ID, CODEY_TASK_EXECUTING);
        if(s_codey_ledmatrix_manager.show_type == 2)
        {
          if(s_codey_ledmatrix_manager.ledmatrix_sequence_show_cb != NULL)
          {
            s_codey_ledmatrix_manager.ledmatrix_sequence_show_cb(s_codey_ledmatrix_manager.animation_index,
                                                               NULL);
          }
        }
        else if(s_codey_ledmatrix_manager.show_type == 1)
        {
          if(s_codey_ledmatrix_manager.ledmatrix_sequence_show_cb != NULL)
          {
            s_codey_ledmatrix_manager.ledmatrix_sequence_show_cb(s_codey_ledmatrix_manager.string_len, 
                                                               (uint8_t *)s_codey_ledmatrix_manager.string_data);
          }      
        }
        s_codey_ledmatrix_seq_show_out = true;
        vTaskDelay(50 / portTICK_PERIOD_MS);
      }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC void codey_lematrix_data_buffer_clear_t(void)
{
  memset(s_codey_ledmatrix_manager.cur_matrix_data, 0, LED_BUFFER_SIZE);
}

STATIC uint8_t codey_lematrix_data_buffer_add_t(int16_t colume_index, uint8_t *data, uint16_t len)
{
  for(uint8_t i = 0; i < len; i++)
  {
    if(colume_index + i < 0)
    {
      continue;
    }

    if((colume_index + i) >= LED_BUFFER_SIZE)
    {
      return 1; // indicate the buffer is over
    }
    s_codey_ledmatrix_manager.cur_matrix_data[colume_index + i] = data[i];
  }
  return 0; // indicate the buffer is not over
}

STATIC uint8_t codey_lematrix_data_buffer_add_from_tail_t(int16_t colume_index, uint8_t *data, uint16_t len)
{
  for(int8_t i = 0; i < len; i++)
  {
    if(colume_index - i > LED_BUFFER_SIZE - 1)
    {
      continue;
    }

    if((colume_index - i) < 0)
    {
      return 1; // indicate the buffer is over
    }
    s_codey_ledmatrix_manager.cur_matrix_data[colume_index - i] = data[len - 1 - i];
  }
  return 0; // indicate the buffer is not over
}

STATIC void codey_lematrix_data_buffer_show_t(void)
{
  codey_digital_writebytestoaddress(0, s_codey_ledmatrix_manager.cur_matrix_data, LED_BUFFER_SIZE);
}

STATIC void codey_ledmatrix_screen_invert_t(void)
{
  s_codey_ledmatrix_manager.show_out_flag = true;
  while(s_codey_ledmatrix_manager.show_count)
  {
    mp_hal_delay_ms(10);
  }

  for(uint8_t i = 0; i < LED_BUFFER_SIZE; i++)
  {
    s_codey_ledmatrix_manager.cur_matrix_data[i] = ~s_codey_ledmatrix_manager.cur_matrix_data[i];
  }
  codey_lematrix_data_buffer_show_t();
}

STATIC void codey_ledmatrix_pixel_invert_t(int16_t p_x, int16_t p_y)
{
  if(p_x > 15 || p_y > 7 || p_x < 0 || p_y < 0)
  {
    return;
  }
 
  s_codey_ledmatrix_manager.show_out_flag = true;
  while(s_codey_ledmatrix_manager.show_count)
  {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  
  s_codey_ledmatrix_manager.cur_matrix_data[p_x] ^= (1 << p_y);

  codey_lematrix_data_buffer_show_t();
}

STATIC uint8_t codey_ledmatrix_get_pixel_status_t(int16_t p_x, int16_t p_y)
{
  if(p_x > 15 || p_y > 7 || p_x < 0 || p_y < 0)
  {
    return 0;
  }

  if(s_codey_ledmatrix_manager.cur_matrix_data[p_x] & (1 << p_y))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

STATIC void codey_digital_write(gpio_num_t io_num, wire_digital_status sta)
{
  gpio_set_level(io_num, sta);
  codey_digital_write_delay();
}

STATIC void codey_digital_write_byte(uint8_t data)
{
  // Start
  codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_HIGH);
  codey_digital_write(WIRE_DIGITAL_SDA_IO, WIRE_DIGITAL_HIGH);
  codey_digital_write(WIRE_DIGITAL_SDA_IO, WIRE_DIGITAL_LOW);

  for(uint8_t i = 0; i < 8; i++)
  {
    codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_LOW);
    codey_digital_write(WIRE_DIGITAL_SDA_IO, (data & 0x01));
    codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_HIGH);
    data = data >> 1;
  }

  // End
  codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_LOW);
  codey_digital_write(WIRE_DIGITAL_SDA_IO, WIRE_DIGITAL_LOW);
  codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_HIGH);
  codey_digital_write(WIRE_DIGITAL_SDA_IO, WIRE_DIGITAL_HIGH);
}

STATIC void codey_digital_writebytestoaddress(uint8_t address, const uint8_t *p_data, uint8_t count_of_data)
{
  uint8_t t_data;

  if((address > 15) || (count_of_data == 0))
  {
    return;
  }
  address = ADDRESS(address);
  LED_DATA_WRITE_ENTER
  /* to avoid static interference*/
  codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_HIGH);
  codey_digital_write(WIRE_DIGITAL_SDA_IO, WIRE_DIGITAL_HIGH);
  codey_digital_write_byte(Mode_Address_Auto_Add_1);
  // Start
  codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_HIGH);
  codey_digital_write(WIRE_DIGITAL_SDA_IO, WIRE_DIGITAL_HIGH);
  codey_digital_write(WIRE_DIGITAL_SDA_IO, WIRE_DIGITAL_LOW);

  // write Address
  for(uint8_t i = 0; i < 8; i++)
  {
    codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_LOW);
    codey_digital_write(WIRE_DIGITAL_SDA_IO, (address & 0x01));
    codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_HIGH);
    address = address >> 1;
  }

  // write data
  for(uint8_t k = 0; k < count_of_data; k++)
  {
    t_data = *(p_data + k);

    for(char i = 0; i < 8; i++)
    {
      codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_LOW);
      codey_digital_write(WIRE_DIGITAL_SDA_IO, (t_data & 0x01));
      codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_HIGH);
      t_data = t_data >> 1;
    }
  }

  // End
  codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_LOW);
  codey_digital_write(WIRE_DIGITAL_SDA_IO, WIRE_DIGITAL_LOW);
  codey_digital_write(WIRE_DIGITAL_SCL_IO, WIRE_DIGITAL_HIGH);
  codey_digital_write(WIRE_DIGITAL_SDA_IO, WIRE_DIGITAL_HIGH);
  LED_DATA_WRITE_EXIT
}

STATIC void codey_digital_write_delay(void)
{
  ets_delay_us(5);
}

STATIC uint8_t codey_char_font_invert(uint8_t data) 
{
  volatile uint8_t tempdata;
   
  tempdata = 0;
  tempdata += (data & 0x80) >> 7;
  tempdata += (data & 0x40) >> 5;
  tempdata += (data & 0x20) >> 3; 
  tempdata += (data & 0x10) >> 1;
  tempdata += (data & 0x08) << 1;
  tempdata += (data & 0x04) << 3;
  tempdata += (data & 0x02) << 5;
  tempdata += (data & 0x01) << 7;
  
  return tempdata;
} 

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
STATIC mp_obj_t codey_ledmatrix_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
  
  // setup the object
  codey_ledmatrix_obj_t *self = &s_codey_ledmatrix_obj;
  self->base.type = &codey_ledmatrix_type;
  
  return self;
}

STATIC mp_obj_t codey_ledmatrix_init(mp_obj_t self_in)
{
  codey_ledmatrix_initialize_t(); 
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_ledmatrix_init_obj, codey_ledmatrix_init);

STATIC mp_obj_t codey_ledmatrix_deinit(mp_obj_t self_in)
{
  codey_Ledmatrix_setbrightness_t(BRIGHTNESS_0);
  codey_ledmatrix_screen_clean_t();

  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_ledmatrix_deinit_obj, codey_ledmatrix_deinit);

STATIC mp_obj_t codey_ledmatrix_clean(mp_obj_t self_in)
{
  codey_ledmatrix_screen_clean_t();
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_ledmatrix_clean_obj, codey_ledmatrix_clean);

STATIC mp_obj_t codey_ledmatrix_screen_invert(mp_obj_t self_in)
{
  codey_ledmatrix_screen_invert_t();
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_ledmatrix_screen_invert_obj, codey_ledmatrix_screen_invert);

STATIC mp_obj_t codey_ledmatrix_get_pixel_status(mp_obj_t self_in, mp_obj_t arg1, mp_obj_t arg2)
{
  int16_t pos_x = mp_obj_get_int(arg1);
  int16_t pos_y = mp_obj_get_int(arg2);

  if(pos_x > (LEDMATRIX_COLUMN_NUM - 1) || pos_y > (LEDMATRIX_ROW_NUM - 1) || pos_x < 0 || pos_y < 0)
  {
    return mp_const_false;
  }
  return mp_obj_new_bool(codey_ledmatrix_get_pixel_status_t(pos_x, pos_y));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(codey_ledmatrix_get_pixel_status_obj, codey_ledmatrix_get_pixel_status);

STATIC mp_obj_t codey_ledmatrix_set_brightness(mp_obj_t self_in, mp_obj_t arg1)
{
  uint8_t brightness = mp_obj_get_int(arg1);
  brightness = brightness > BRIGHTNESS_8 ? BRIGHTNESS_8 : brightness;
  brightness = brightness < BRIGHTNESS_0 ? BRIGHTNESS_0 : brightness;
  codey_Ledmatrix_setbrightness_t(brightness);
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_ledmatrix_set_brightness_obj, codey_ledmatrix_set_brightness);

STATIC mp_obj_t codey_ledmatrix_string_show(mp_obj_t self_in, mp_obj_t arg1, mp_obj_t arg2)
{ 
  if(!codey_ledmatrix_show_is_enabled())
  {
    return mp_const_none;
  }

  uint8_t to_stop = mp_obj_get_int(arg2);
  const char *data = NULL;
  size_t len = 0;
 
  if(MP_OBJ_IS_STR(arg1))
  {
    data = mp_obj_str_get_data(arg1, &len);
    len = (len > STRING_CHARS_NUM_MAX) ? STRING_CHARS_NUM_MAX : len;
    if(to_stop != 0)
    { 
      memcpy(s_codey_ledmatrix_manager.string_data, data, len);
      s_codey_ledmatrix_manager.string_len = len;
      
      s_codey_ledmatrix_seq_show_out = false;
      codey_ledmatrix_sequence_show_cb_register_t(1, codey_ledmatrix_show_string_t);
      xSemaphoreGive(s_codey_ledmatrix_manager.led_seq_sema);
      while(!s_codey_ledmatrix_seq_show_out)
      {
        mp_hal_delay_ms(10);
      }
    }
    else
    {
      memcpy(s_codey_ledmatrix_manager.string_data, data, len);
      s_codey_ledmatrix_manager.string_len = len;
      codey_ledmatrix_sequence_show_cb_register_t(1, codey_ledmatrix_show_string_t);
      xSemaphoreGive(s_codey_ledmatrix_manager.led_seq_sema);
    }
  }
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(codey_ledmatrix_string_show_obj, codey_ledmatrix_string_show);

STATIC mp_obj_t codey_ledmatrix_animation_show(mp_obj_t self_in, mp_obj_t arg1, mp_obj_t arg2)
{
  if(!codey_ledmatrix_show_is_enabled())
  {
    return mp_const_none;
  }

  uint8_t to_stop = mp_obj_get_int(arg2);
  uint8_t index = mp_obj_get_int(arg1);
  s_codey_ledmatrix_manager.animation_index = index;
  
  if(to_stop != 0)
  {
    s_codey_ledmatrix_seq_show_out = false;
    codey_ledmatrix_sequence_show_cb_register_t(2, codey_ledmatrix_show_animation_t);
    xSemaphoreGive(s_codey_ledmatrix_manager.led_seq_sema);
    while(!s_codey_ledmatrix_seq_show_out)
    {
      mp_hal_delay_ms(10);
    }
  }
  else
  {
    codey_ledmatrix_sequence_show_cb_register_t(2, codey_ledmatrix_show_animation_t);
    xSemaphoreGive(s_codey_ledmatrix_manager.led_seq_sema);
  }
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(codey_ledmatrix_animation_show_obj, codey_ledmatrix_animation_show);

STATIC mp_obj_t codey_ledmatrix_number_show(mp_obj_t self_in, mp_obj_t arg1)
{
  if(!codey_ledmatrix_show_is_enabled())
  {
    return mp_const_none;
  }
    
  const char *data = NULL;
  size_t len;
  
  data = mp_obj_str_get_data(arg1, &len);
  // ESP_LOGI(TAG, "number received is %s", data);
  codey_ledmatrix_show_number_t(len, (uint8_t *)data);  
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_ledmatrix_number_show_obj, codey_ledmatrix_number_show);

STATIC mp_obj_t codey_ledmatrix_pixel_control(mp_uint_t n_args, const mp_obj_t *args)
{
  if(!codey_ledmatrix_show_is_enabled())
  {
    return mp_const_none;
  }
    
  if(n_args < 3)
  {
    return mp_const_false;
  }
  int16_t pos_x = mp_obj_get_int(args[1]);
  int16_t pos_y = mp_obj_get_int(args[2]);
  uint8_t status = mp_obj_get_int(args[3]);
  if(pos_x > (LEDMATRIX_COLUMN_NUM - 1) || pos_y > (LEDMATRIX_ROW_NUM - 1) || pos_x < 0 || pos_y < 0)
  {
    return mp_const_false;
  }
  codey_ledmatrix_show_pixel_t(pos_x, pos_y, (bool)(status));
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(codey_ledmatrix_pixel_control_obj, 4, 4, codey_ledmatrix_pixel_control);

STATIC mp_obj_t codey_ledmatrix_pixel_invert(mp_obj_t self_in, mp_obj_t arg1, mp_obj_t arg2)
{
  int16_t pos_x = mp_obj_get_int(arg1);
  int16_t pos_y = mp_obj_get_int(arg2);

  if(pos_x > (LEDMATRIX_COLUMN_NUM - 1) || pos_y > (LEDMATRIX_ROW_NUM - 1) || pos_x < 0 || pos_y < 0)
  {
    return mp_const_false;
  }
  codey_ledmatrix_pixel_invert_t(pos_x, pos_y);
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(codey_ledmatrix_pixel_invert_obj, codey_ledmatrix_pixel_invert);

STATIC mp_obj_t codey_ledmatrix_faceplate_show(mp_uint_t n_args, const mp_obj_t *args)
{  
  if(!codey_ledmatrix_show_is_enabled())
  {
    return mp_const_none;
  }
  
  int16_t pos_x = 0;
  int16_t pos_y = 0;
  uint8_t data_buf[LED_BUFFER_SIZE];
  uint8_t face_data[LED_BUFFER_SIZE];
  memset(data_buf, 0 , LED_BUFFER_SIZE);
  memset(face_data, 0 , LED_BUFFER_SIZE);
  
  pos_x = mp_obj_get_int(args[1]);
  pos_y = mp_obj_get_int(args[2]);

  if(abs(pos_x) > (LEDMATRIX_COLUMN_NUM - 1) || abs(pos_y) > (LEDMATRIX_ROW_NUM - 1))
  {
    return mp_const_false;
  }

  for(uint8_t n = 0; n < LEDMATRIX_COLUMN_NUM; n++)
  {
    if(n < (n_args - 3))
    {
      face_data[n] = (uint8_t)(mp_obj_get_int(args[n + 3]));
      if(pos_y > 0)
      {
        face_data[n] >>= pos_y;
      }
      else if(pos_y < 0)
      {
        face_data[n] <<= (-pos_y);
      }
      
    }
    else
    {
      face_data[n] = 0;
    }
  }
  
  if(pos_x >= 0)
  {
    memcpy(data_buf + pos_x, face_data, LED_BUFFER_SIZE - pos_x);
  }
  else if(pos_x < 0)
  {
    memcpy(data_buf, face_data - pos_x, LED_BUFFER_SIZE + pos_x);
  }
  codey_ledmatrix_show_faceplate_t(data_buf);
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(codey_ledmatrix_faceplate_show_obj, 3, 19, codey_ledmatrix_faceplate_show);

STATIC mp_obj_t codey_ledmatrix_faceplate_show_with_time(mp_uint_t n_args, const mp_obj_t *args)
{  
  if(!codey_ledmatrix_show_is_enabled())
  {
    return mp_const_none;
  }
  
  int16_t pos_x = 0;
  int16_t pos_y = 0;
  uint32_t t_ms = 0;
  uint8_t data_buf[LED_BUFFER_SIZE];
  uint8_t face_data[LED_BUFFER_SIZE];
  memset(data_buf, 0 , LED_BUFFER_SIZE);
  memset(face_data, 0 , LED_BUFFER_SIZE);
  
  pos_x = mp_obj_get_int(args[1]);
  pos_y = mp_obj_get_int(args[2]);
  t_ms  = mp_obj_get_int(args[3]);

  if(abs(pos_x) > (LEDMATRIX_COLUMN_NUM - 1) || abs(pos_y) > (LEDMATRIX_ROW_NUM - 1))
  {
    return mp_const_false;
  }

  for(uint8_t n = 0; n < LEDMATRIX_COLUMN_NUM; n++)
  {
    if(n < (n_args - 4))
    {
      face_data[n] = (uint8_t)(mp_obj_get_int(args[n + 4]));
      if(pos_y > 0)
      {
        face_data[n] >>= pos_y;
      }
      else if(pos_y < 0)
      {
        face_data[n] <<= (-pos_y);
      }
      
    }
    else
    {
      face_data[n] = 0;
    }
  }
  
  if(pos_x >= 0)
  {
    memcpy(data_buf + pos_x, face_data, LED_BUFFER_SIZE - pos_x);
  }
  else if(pos_x < 0)
  {
    memcpy(data_buf, face_data - pos_x, LED_BUFFER_SIZE + pos_x);
  }
  codey_ledmatrix_show_faceplate_with_time_t(data_buf, t_ms);
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(codey_ledmatrix_faceplate_show_with_time_obj, 4, 20, codey_ledmatrix_faceplate_show_with_time);

STATIC void codey_ledmatrix_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC mp_obj_t codey_ledmatrix_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  
  return mp_const_none;
}

STATIC const mp_map_elem_t codey_ledmatrix_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_clean),                    (mp_obj_t)&codey_ledmatrix_clean_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_init),                     (mp_obj_t)&codey_ledmatrix_init_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),                   (mp_obj_t)&codey_ledmatrix_deinit_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_brightness),           (mp_obj_t)&codey_ledmatrix_set_brightness_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_string_show),              (mp_obj_t)&codey_ledmatrix_string_show_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_number_show),              (mp_obj_t)&codey_ledmatrix_number_show_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_animation_show),           (mp_obj_t)&codey_ledmatrix_animation_show_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_faceplate_show),           (mp_obj_t)&codey_ledmatrix_faceplate_show_obj },  
  { MP_OBJ_NEW_QSTR(MP_QSTR_faceplate_show_with_time), (mp_obj_t)&codey_ledmatrix_faceplate_show_with_time_obj }, 
  { MP_OBJ_NEW_QSTR(MP_QSTR_pixel_control),            (mp_obj_t)&codey_ledmatrix_pixel_control_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_screen_invert),            (mp_obj_t)&codey_ledmatrix_screen_invert_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_pixel_invert),             (mp_obj_t)&codey_ledmatrix_pixel_invert_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_pixel),                (mp_obj_t)&codey_ledmatrix_get_pixel_status_obj },

};

STATIC MP_DEFINE_CONST_DICT(codey_ledmatrix_locals_dict, codey_ledmatrix_locals_dict_table);

const mp_obj_type_t codey_ledmatrix_type =
{
  { &mp_type_type },
  .name = MP_QSTR_ledmatrix,
  .print = codey_ledmatrix_print,
  .call = codey_ledmatrix_call,
  .make_new = codey_ledmatrix_make_new,
  .locals_dict = (mp_obj_t)&codey_ledmatrix_locals_dict,
};
