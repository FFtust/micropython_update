/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_ledmatrix_board.c
 * @file    codey_ledmatrix_board.h
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
 * fftust              2017/10/03     1.0.0              build the new.
 * </pre>
 *
 */

#ifndef _CODEY_LEDMATRIX_H_
#define _CODEY_LEDMATRIX_H_

#define WIRE_DIGITAL_SCL_IO    (23)
#define WIRE_DIGITAL_SDA_IO    (22)
//*************************************************
#define LED_BUFFER_SIZE               (16)
#define STRING_DISPLAY_BUFFER_SIZE    (20)

/* Define Data Command Parameters */
#define Mode_Address_Auto_Add_1       (0x40)     
#define Mode_Permanent_Address        (0x44)     

/* Define Address Command Parameters */
#define ADDRESS(addr)                 (0xC0 | addr)

#define LED_MATRIX_CHARFONT_ROW       (8)
#define LED_MATRIX_CHARFONT_COLUMN    (6)

#define LED_MATRIX_NUMBERFONT_ROW     (8)
#define LED_MATRIX_NUMBERFONT_COLUMN  (3)
 
#define LED_MATRIX_CHAR_TABLE_LEN     (85)
#define LED_MATRIX_NUMBER_TABLE_LEN   (14)

#define LED_MATRIX_STRING_SHOW_INTERVAL (100)
#define LED_MATRIX_STRING_SHOW_SPEED    (1)

#define ANIMATION_FACE_NUM_MAX          (50)
#define STRING_CHARS_NUM_MAX            (128)

#define LEDMATRIX_COLUMN_NUM            (16)
#define LEDMATRIX_ROW_NUM               (8)

typedef enum    
{
  WIRE_DIGITAL_LOW = 0,
  WIRE_DIGITAL_HIGH = 1,
} wire_digital_status;

typedef enum
{
  BRIGHTNESS_0 = 0,
  BRIGHTNESS_1,
  BRIGHTNESS_2,
  BRIGHTNESS_3,
  BRIGHTNESS_4,
  BRIGHTNESS_5,
  BRIGHTNESS_6,
  BRIGHTNESS_7,
  BRIGHTNESS_8
}led_matrix_brightness_typedef;

typedef struct 
{ 
  uint8_t character[1];
  uint8_t data[3];
}led_matrix_numberfont_typedef;

typedef struct 
{
  uint8_t character[1];
  uint8_t data[6];
}led_matrix_charfont_typedef;

typedef struct
{
  uint16_t index;
  uint16_t len;
  uint16_t interval;
  uint16_t loop_times;
  uint8_t animation_data[ANIMATION_FACE_NUM_MAX][LED_BUFFER_SIZE]; 
}animation_struct_in_flash_t;

extern const led_matrix_charfont_typedef codey_char_font[];
extern const led_matrix_numberfont_typedef codey_number_font[];

extern const mp_obj_type_t codey_ledmatrix_type;

extern void codey_ledmatrix_board_config_t(void); 
extern void codey_ledmatrix_initialize_t(void);
extern void codey_ledmatrix_screen_clean_t(void);
extern void codey_Ledmatrix_setbrightness_t(uint8_t bright);
extern void codey_ledmatrix_show_faceplate_t(const uint8_t *data);
extern void codey_ledmatrix_show_pixel_t(int16_t pos_x, int16_t pos_y, bool status);
extern void codey_ledmatrix_board_show_task_t(void *parameter);
extern void codey_ledmatrix_show_enable(void);
extern void codey_ledmatrix_show_disable(void);
extern bool codey_ledmatrix_show_is_enabled(void);
extern void codey_ledmatrix_reload_t(void);
extern void codey_ledmatrix_show_faceplate_with_time_t(const uint8_t *data, uint32_t time_ms);

#endif /* _CODEY_LEDMATRIX_H_ */



