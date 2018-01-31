/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for neurons_engine_list_maintain.c.
 * @file    neurons_engine_list_maintain.h
 * @author  fftust
 * @version V1.0.0
 * @date    2017/09/6
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
 * This file is a drive  neurons engine module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/09/06      1.0.0              build the new.
 * </pre>
 *
 */

#ifndef _NEURONS_ENGINE_LIST_MAINTAIN_H_
#define _NEURONS_ENGINE_LIST_MAINTAIN_H_
#include "codey_neurons_universal_protocol.h"

#define NEURONS_NODE_MAX (50)
#define NEURONS_NODE_TYPE_MAX (50)

#define ROCKY_SENSOR_REPORT_RGB (0x01)
#define ROCKY_SENSOR_REPORT_COLOR (0x02)
#define ROCkY_SENSOR_REPORT_LIGHTNESS (0x03)
#define ROCKY_SENSOR_REPORT_GREY (0x04)
#define ROCkY_SENSOR_REPORT_LIGHTNESS_REFLECT (0x05)
#define ROCkY_SENSOR_REPORT_IR_REFLECT (0x06)
#define ROCKY_SENSOR_REPORT_IS_BARRIER (0x08)
#define ROCKY_SENSOR_REPORT_MOTOR_CURRENT (0x0A)

#define ROCKY_SENSOR_REPORT_MAX (0x0A)
#define ROCKY_SENSOR_REPORT_MIN (0x01)

/* define the struct of module information that stored in RAM */
#define MODULE_ONLINE_INFO_SINGLE_COMMAND_LEN (6) // cmd_id(1) + para_type(5)
#define MODULE_ONLINE_INFO_SINGLE_RESPOND_LEN (7) // respond_id(1) + para_type(5) + data_store_offset(1)


/* for rocky data read, not all the neurons modules */
typedef enum
{
  ROCKY_RGB_BIT    =    BIT0,
  ROCKY_COLOR_BIT  =    BIT1,
  ROCKY_LIGHT_BIT  =    BIT2,
  ROCKY_GREY_BIT   =    BIT3,
  ROCKY_LIGHT_RE   =    BIT4,
  ROCKY_IR_RE      =    BIT5,
  ROCKY_BARRIER    =    BIT6,
  ROCKY_MOTOR_CURRENT = BIT7
}ROCKY_READ_BITS;

typedef union
{
  uint8_t respond_item[7]; // respond_id(1)  + data_type(5) + valueoffset(1)
  struct respond_struct_t
  {
    uint8_t respond_id;
    uint8_t data_type[5];
    uint8_t value_offset; 
  }respond_struct;
}neurons_engine_online_block_lib_respond_t;

typedef union
{
  uint8_t command_item[6]; // respond_id(1) + valueoffset(1) + data_type(5)
  struct command_struct_t
  {
    uint8_t command_id;
    uint8_t data_type[5];
  }command_struct;

}neurons_engine_online_block_lib_command_t;

typedef struct
{
  uint8_t block_type;
  uint8_t block_sub_type; 
  uint8_t block_index;
  uint8_t specila_block;
  uint8_t block_num;    /* if  block_num == 0 , then free this lib space */
  uint8_t *command_data_type;
  uint8_t *respond_data_type;  
}neurons_engine_online_block_lib_t;

/* this struct store the address offset in the library of every block information */
/* this information is stored in flash, it's better to read it and put it into a 
   RAM space when board reboot to improve efficence*/
typedef struct
{
  uint8_t type_inf[2]; // type and sub_type
  uint16_t offset;
}neurons_engine_lib_dict_t;

typedef struct
{
  uint8_t respond_data_buffer_size;
  uint8_t *respond_data_buffer;
}respond_data_t;
/* this struct store the new list that will update the main list */
typedef struct
{ 
  uint8_t block_type;
  uint8_t block_sub_type;
  uint8_t dev_id;
  uint8_t offline_time;
  neurons_engine_online_block_lib_t *block_ram_addr; // alloc a heap space when find the block information in flash
  respond_data_t respond_data_info;
}neurons_engine_online_block_info_t;

// #define CMD_BUFFER_MAX 64 /* must be 2^^n */
// Change to 2 for just one comand at the same time
#define CMD_BUFFER_MAX 2 /* must be 2^^n */
#define CMD_BUFFER_WAIT_MS (10)
#define CMD_DATA_BUFFER_LEN (128)
typedef struct
{
  uint8_t type_id;
  uint8_t sub_type_id;
  uint8_t index;
  uint8_t cmd_id;
  uint8_t cmd_data_len;
  float cmd_data[CMD_DATA_BUFFER_LEN];
}neurons_comnad_type_t;

extern void neurons_engine_parse_package_t(neurons_commmd_package_t *package);
extern bool neurons_engine_init_t(void);
extern void neurons_engine_send_heart_package_t(void);
extern neurons_engine_online_block_info_t *neurons_engine_get_main_list_block_info_by_index_t(uint8_t index, uint8_t type, uint8_t sub_type);
extern void neurons_engine_get_command_info_t(neurons_engine_online_block_info_t *info, uint8_t cmd_id, 
                                              uint8_t *data, uint8_t *len);
extern void neurons_engine_get_respond_info_t(neurons_engine_online_block_info_t *info, uint8_t respond_id, 
                                              uint8_t *respond_data_offset, uint8_t *data, uint8_t *len);
extern void neurons_engine_command_buffer_push(neurons_comnad_type_t *cmd_type);
extern uint8_t neurons_engine_command_buffer_pop(neurons_comnad_type_t *cmd_type);
extern uint8_t neurons_engine_send_single_command_t(neurons_comnad_type_t *cmd_type);
extern void neurons_engine_command_buffer_init(void);
extern void neurons_engine_set_rocky_report_mode_t(uint8_t block_id, uint8_t sensor_id, uint8_t mode,
                                                   long period);
extern void rocky_sensor_set_report_status_t(uint8_t index, uint8_t sta);
extern int8_t rocky_sensor_get_report_status_t(uint8_t index);
extern void neurons_engine_reset_rocky_report_mode_t(void);
extern void neurons_engine_rocky_stop(void);
extern bool neurons_engine_get_lib_found_flag_t(void);
extern void neurons_engine_get_version_t(char *dataout, uint8_t *len);

extern void neurons_engine_read_set_bits_t(uint32_t bits);
extern void neurons_engine_read_clear_bits_t(uint32_t bits);
extern void neurons_engine_wait_bits_t(uint32_t bits);
extern uint8_t neurons_engine_find_block_in_main_llist_by_index_t(uint8_t index, uint8_t type, uint8_t sub_type);

extern void neurons_engine_alloc_id_t(void);

#endif /* _NEURONS_ENGINE_LIST_MAINTAIN_H_ */