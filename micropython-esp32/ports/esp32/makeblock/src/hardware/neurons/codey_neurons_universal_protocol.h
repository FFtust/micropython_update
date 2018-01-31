/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_neurons_universal_protocol.c.
 * @file    codey_neurons_universal_protocol.h
 * @author  fftust
 * @version V1.0.0
 * @date    2017/08/14
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
 * This file is a drive neurons_protocol module.
 *
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
#ifndef _CODEY_NEURONS_UNIVERSAL_PROCOTOL_H_
#define _CODEY_NEURONS_UNIVERSAL_PROCOTOL_H_
#include "codey_ringbuf.h"

// the version.
#define DEVICE_TYPE      (21) // neurons device.
#define PROTOCOL_TYPE    (01) // firmata protocol.

// device mode.
#define OFF_LINE (0)
#define ON_LINE  (1)
#define ONLINE_ESP32 (2)

// protocol, start index, end index.
#define START_SYSEX             (0xF0) // online start
#define END_SYSEX               (0xF7) // online end
#define START_SYSEX_OFFLINE     (0xF1) // offline start
#define END_SYSEX_OFFLINE       (0xF6) // offline end

#define START_ESP32_ONLINE      (0xF0) // this new protocol is for esp32
#define END_ESP32_ONLINE        (0xF7) // on offline mode for esp32 

#define START_ESP32_MBFTP       (0xF3)
#define END_ESP32_MBFTP         (0xF4)

/* general command for system. */
#define CTL_SYSTEM_COMMAND_MIN           (0x10)
#define CTL_ASSIGN_DEV_ID                (0x10) // Assignment device ID.
#define CTL_SYSTEM_RESET                 (0x11) // reset from host.
#define CTL_QUERY_FIRMWARE_VERSION       (0x12)
#define CTL_SET_BAUDRATE                 (0x13)
#define CTL_COMMUNICATION_TEST           (0x14)

/* general response */
#define GENERAL_RESPONSE                 (0x15) // general response to host.

#define PROCESS_SUC             (0x0F)
#define PROCESS_BUSY            (0x10)
#define EXECUTE_ERROR           (0x11)
#define WRONG_TYPE              (0x12)
#define CHECK_ERROR             (0x13)

#define CTL_SYSTEM_COMMAND_MAX           (0x15)

/* general command for module */
#define CTL_GENERAL                      (0x61)

#define CTL_SET_FEEDBACK                 (0x01) // set feedback.
#define CTL_SET_RGB_LED                  (0x02)
#define CTL_FIND_BLOCK                   (0x03)
#define CTL_UPDATE_FIRMWARE              (0x05)
#define CTL_READ_HARDWARE_ID             (0x06)

/* Block type. */
#define NO_SUBTYPE_BOUND                 (0x60) // type <=0x60, no subtype.

/* MOTOR */
#define CLASS_MOTOR                     (0x62)

#define BLOCK_SINGLE_DC_MOTOR           (0x01)
#define BLOCK_DOUBLE_DC_MOTOR           (0x02)
#define BLOCK_9G_SERVO                  (0x03)
#define BLOCK_ENCODER_MOTOR             (0x04)

/* SENSOR */
#define CLASS_SENSOR                    (0x63)

#define BLOCK_TEMPERATURE               (0x01)
#define BLOCK_LIGHT_SENSOR              (0x02)
#define BLOCK_ULTRASONIC_SENSOR         (0x03)
#define BLOCK_LINE_FOLLOWER             (0x04)
#define BLOCK_COLOUR_SENSOR             (0x05)
#define BLOCK_ACCELEROMETER_GYRO        (0x06)
#define BLOCK_TEMPERATURE_HUMIDITY      (0x07)
#define BLOCK_SOIL_MOISTURE             (0x08)
#define BLOCK_RAINING_SENSOR            (0x09)
#define BLOCK_WIND_SPEED_SENSOR         (0x0a)
#define BLOCK_PM_SENSOR                 (0x0b)
#define BLOCK_PIR_SENSOR                (0x0c)
#define BLOCK_SOUND_SENSOR              (0x0d)
#define BLOCK_CODEY_CAR                 (0x10) // codey car id

/* CONTROL */
#define CLASS_CONTROL                   (0x64)

#define BLOCK_POTENTIOMETER             (0x01)
#define BLOCK_BUTTON                    (0x02)
#define BLOCK_MAKEY_MAKEY               (0x04)
#define BLOCK_TOUCH_KEY                 (0x05)
#define BLOCK_SELF_LOCKING_SWITCH       (0x06)
#define BLOCK_JOYSTICK                  (0x07)

/* DISPLAY */
#define CLASS_DISPLAY                   (0x65)

#define BLOCK_NUMERIC_DISPLAY           (0x01)
#define BLOCK_SINGLE_COLOUR_LED         (0x02)
#define BLOCK_LIGHT_BAR                 (0x03)
#define BLOCK_LED_COLOUR_MATRIX_8_8     (0x04)
#define BLOCK_OLED_DISPLAY              (0x05)
#define BLOCK_COOL_LIGHT                (0x06)

/* AUDIO */
#define CLASS_AUDIO                     (0x66)

#define BLOCK_MP3                       (0x01)
#define BLOCK_BUZZER                    (0x02)
#define BLOCK_VOICE_CONTROL             (0x03)

/*OHTER*/
#define CLASS_OTHER                     (0x67)

#define BLOCK_POWER_BANK                (0x01)

/* ESP32 BOARD SENSOR*/
#define CLASS_ESP32_SENSORS             (0x5E)

#define BOARD_DETECTORS_MIN             (0x01)

#define BOARD_BUTTON                    (0x01)
#define BOARD_SOUND_SENSOR              (0x02)
#define BOARD_LIGHT_SENSOR              (0x03)
#define BOARD_POTENTIONMETER            (0x04)
#define BOARD_GYRO                      (0x05)
#define BOARD_INFRARED_RX               (0x06)

#define BOARD_DETECTORS_MAX             (0x06)

#define BOARD_CONTROLLERS_MIN           (0x41)

#define BOARD_RGB_LED                   (0x41)
#define BOARD_LED_MATRIX                (0x42)
#define BOARD_SPEAKER                   (0x43)
#define BOARD_INFRARED_TX               (0x44)

#define BOARD_CONTROLLERS_MAX           (0x44)

/* report mode */
#define REPORT_MODE_REQ         (0x00) // report when host request current value.
#define REPORT_MODE_DIFF        (0x01) // report when current vale is differnt from previous value.
#define REPORT_MODE_CYCLE       (0x02) // report cycle.
#define MIN_REPORT_PERIOD_ON_LINE (10)
#define DEFAULT_REPORT_PERIOD_ON_LINE   (40)
#define OFF_LINE_REPORT_PERIOD    (40)

#define ALL_DEVICE                (0xff)    // pin included in I2C setup

#define CODEY_UART0_ID (1)
#define CODEY_UART1_ID (2)
#define CODEY_BLUE_TOOTH_ID (3)

#define  BYTE_8_LEN      (1)
#define  BYTE_16_LEN     (2)
#define  SHORT_16_LEN    (2)
#define  SHORT_24_LEN    (3)
#define  LONG_40_LEN     (5)
#define  FLOAT_40_LEN    (5)
#define  DOUBLE_72_LEN   (9)

typedef struct
{
  uint8_t head;
  uint8_t device_id;
  uint8_t service_id;
  uint8_t sub_service_id;             
  uint8_t *cmd_data;   // reserve for cmd_data ,never use          
  uint8_t end;
}neurons_command_frame_t;

typedef struct
{
  union
  {
    uint8_t data[64]; // a cmd package must less than 64 byte
    struct cmd_info_t
    {
      uint8_t head;
      uint8_t device_id;
      uint8_t service_id;
      uint8_t sub_service_id;  // this id may not exist in some devices
      uint8_t cmd_id;
    }cmd_info;
  }cmd_package_info;
  uint8_t data_in_index;
}neurons_commmd_package_t;

typedef enum
{
  NEURONS_FRAME_HEAD,
  NEURONS_FRAME_DEV_ID,
  NEURONS_FRAME_SER_ID,
  NEURONS_FRAME_SUB_SER_ID,  // this id may not exist in some devices
  NEURONS_FRAME_CMD_DATA,
  NEURONS_FRAME_END
}neurons_frame_data_index_t;

typedef enum
{
  BYTE_8_T    = 0x01,
  BYTE_16_T   = 0x02,
  SHORT_16_T  = 0x03,
  SHORT_24_T  = 0x04,
  LONG_40_T   = 0x05,
  FLOAT_40_T  = 0x06,
  DOUBLE_72_T = 0x07,
  /* special type, use internaly only */
  ALTERABLE_NUM_BYTE = 0xb0
}neurons_data_type_t;

typedef union
{
  uint8_t byte_val[8];
  double  double_val;
}val_8_byte;

typedef union
{
  uint8_t byte_val[4];
  float   float_val;
  long    long_val;
}val_4_byte;

typedef union
{
  uint8_t byte_val[2];
  int16_t short_val;
}val_2_byte;

typedef union
{
  uint8_t byte_val[1];
  int8_t  char_val;
}val_1_byte;

typedef void (* neurons_command_parse_cb)(neurons_commmd_package_t *, uint8_t);

extern void    value_to_neurons_byte_8(void *val, uint8_t *byte_out, uint16_t *byte_out_size);
extern void    value_to_neurons_byte_16(void *val, uint8_t *byte_out, uint16_t *byte_out_size);
extern void    value_to_neurons_short_16(void *val, uint8_t *byte_out, uint16_t *byte_out_size);
extern void    value_to_neurons_short_16(void *val, uint8_t *byte_out, uint16_t *byte_out_size);
extern void    value_to_neurons_long_40(void *val, uint8_t *byte_out, uint16_t *byte_out_size) ;
extern void    value_to_neurons_float_40(void *val, uint8_t *byte_out, uint16_t *byte_out_size);
extern uint8_t byte_8_to_value(uint8_t *value_bytes);
extern int8_t  byte_16_to_value(uint8_t *value_bytes);
extern uint16_t  short_16_to_value(uint8_t *value_bytes);
extern short   short_24_to_value(uint8_t *value_bytes);
extern float   float_40_to_value(uint8_t *value_bytes);
extern long    long_40_to_value(uint8_t *value_bytes);

extern void neurons_frame_struct_init_t(neurons_command_frame_t *cmd_frame, uint8_t head, uint8_t dev_id,
                                        uint8_t ser_id, uint8_t s_ser_id, uint8_t end);
extern void neurons_frame_struct_set_by_type_t(neurons_command_frame_t *cmd_frame, neurons_frame_data_index_t index,
                                               uint8_t value);
extern void neurons_buffer_add_data_t(neurons_commmd_package_t *package, neurons_data_type_t cmd_data_type, void *cmd_data);
extern void neurons_buffer_read_value_t(neurons_commmd_package_t *package, neurons_frame_data_index_t index,
                                        uint8_t cmd_data_index, neurons_data_type_t cmd_data_type, void *value);
extern void neurons_buffer_add_head_frame_t(neurons_command_frame_t *cmd_frame, neurons_commmd_package_t *package, bool add_sub_ser_id);
extern void neurons_buffer_add_end_frame_t(neurons_command_frame_t *cmd_frame, neurons_commmd_package_t *package);
extern void neurons_command_package_add_bytes_t(neurons_commmd_package_t *package, uint8_t *data, uint16_t size);
extern void neurons_command_package_reset(neurons_commmd_package_t *package);
extern void neurons_command_package_put_t(neurons_commmd_package_t *package, uint8_t *c, uint8_t head, uint8_t end,
                                          neurons_command_parse_cb nc_cb, uint8_t peripheral, uint8_t protocol_index);
extern void neurons_command_packege_send_all(neurons_commmd_package_t *package, uint8_t peripheral);
extern uint8_t neurons_engine_parse_respond_type_t(neurons_commmd_package_t *package);
extern uint8_t firmata_get_data_type_lenth_t(uint8_t data_type);
extern void neurons_get_data_region_from_package_t(neurons_commmd_package_t *package, uint8_t *start_index, uint8_t *length);
extern void neurons_show_package_t(neurons_commmd_package_t *package);

#endif /* _CODEY_NEURONS_UNIVERSAL_PROCOTOL_H_ */
