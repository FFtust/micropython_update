/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for neurons_engine_lib_struct.c.
 * @file    neurons_engine_lib_struct.h
 * @author  fftust
 * @version V1.0.0
 * @date    2017/9/6
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

#ifndef _NEURONS_ENGINE_LIB_H_
#define _NEURONS_ENGINE_LIB_H_

#include "neurons_engine_list_maintain.h"

#define NEURONS_ENGINE_LIB_FILE_NAME "neurons_engine_lib.bin"

typedef struct 
{
  uint8_t respond_id;
  uint8_t data_type[5];
}neurons_respond_type_t;

typedef struct 
{
  uint8_t command_id;
  uint8_t data_type[5];
}neurons_command_type_t;

/* this struct store the lib of neurons, get neurons blocks infomation from flash rely
   on this struct */
#define BLOCK_LIB_PER_LENGtH     (62)
#define RESPOND_DATA_ADDR_OFFSET (2)
#define COMMAND_DATA_ADDR_OFFSET (32)
#define REC_CMD_ITEM_PER_LEN     (6)
#define RESPOND_ITEM_MAX         (5)
#define COMMAND_ITEM_MAX         (5)
#define RES_CMD_TYPE_MAX         (5)
#define RES_RESPOND_TYPE_MAX     (5)
#define RES_CMD_OVER_FLAG_DATA   (0xff)

typedef struct
{
  uint8_t block_type;
  uint8_t block_sub_type;
  neurons_respond_type_t neurons_respond_type[5];
  neurons_command_type_t neurons_command_type[5];
}neurons_block_type_t;
/*****************************************************/
typedef struct
{
  uint8_t block_type;
  uint8_t block_sub_type;
}neurons_block_type_inf_t;

/*****************************************************/
/* flash lib format information */
typedef struct
{
  uint8_t type_id;
  uint8_t blocks_num;
  uint16_t offset;
}neurons_engine_every_type_info_t;

typedef struct
{
  uint8_t type_id;
  uint8_t sub_type_id;
  uint8_t respond_num;
  uint8_t command_num;
  uint16_t offset;
}neurons_engine_special_block_t;

#define TYPE_NUM_MAX 32
#define SPECIAL_BLOCK_NUM 8
typedef union
{
  uint8_t data[160];
  struct head_frame_t
  {
    uint8_t magic[16];
    uint8_t version_info[16];
    uint32_t single_lib_bytes_num; // sizeof(neurons_block_type_t) = 62
    uint32_t type_num;
    uint32_t special_block_num;
    neurons_engine_special_block_t special_block_info[8];
    neurons_engine_every_type_info_t type_info[17]; // 47 = (160 - 32 -4 - 4 - 4 - 6 * 8) / 4  
  }head_frame;

}neurons_engine_flash_lib_file_head_t; // reserve 128 byte for lib file

/*****************************************************/
extern bool neurons_engine_read_head_from_flash_lib_t(neurons_engine_flash_lib_file_head_t *head_frame);
extern uint16_t neurons_engine_check_block_from_flash_lib_t(FIL *fp, neurons_block_type_inf_t *block, neurons_engine_flash_lib_file_head_t *map,
                                                            neurons_engine_special_block_t *special_block);
extern void neurons_engine_get_blcok_info_t(FIL *fp, neurons_block_type_inf_t *block, uint16_t offset, 
                                            void* data_out, neurons_engine_special_block_t *special_block);
extern bool neurons_engine_get_respond_info_from_lib_t(uint8_t *block_lib, void **respond_type_out, 
                                                       respond_data_t *respond_data_out, neurons_engine_special_block_t *special_block);
extern void *neurons_engine_get_command_info_from_lib_t(uint8_t *block_lib_t, neurons_engine_special_block_t *special_block);
#endif /* _NEURONS_ENGINE_LIB_H_ */

