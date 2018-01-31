/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock neurons engine module
 * @file    neurons_engine_lib_struct.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/09/06
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
 * This file is a drive for neurons engine module.
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

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/mphal.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "esp_log.h"

#include "codey_esp32_resouce_manager.h"
#include "esp_heap_caps.h"
#include "codey_utils.h"

#include "extmod/vfs_fat.h"
#include "codey_sys.h"
#include "codey_esp32_resouce_manager.h"
#include "codey_neurons_universal_protocol.h"
#include "codey_config.h"

#include "neurons_engine_lib_struct.h"
#include "neurons_engine_list_maintain.h"
#include "codey_firmware.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG  ("NEURONS_FLASH_LIB")
 
/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
 const char NEURONS_FILE_HEAD_MAGIC[16] = "codey_neu";  

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/

/******************************************************************************
  DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
  DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
/* if return 0, means not found */ 
uint16_t neurons_engine_check_block_from_flash_lib_t(FIL *fp, neurons_block_type_inf_t *block, neurons_engine_flash_lib_file_head_t *map,
                                                                          neurons_engine_special_block_t *special_block)
{
  uint8_t type_num = map->head_frame.type_num;
  uint8_t special_block_num = map->head_frame.special_block_num;
  uint16_t offset = 0xffff;
  UINT n1;
  if(special_block_num > SPECIAL_BLOCK_NUM)
  {
    special_block_num = SPECIAL_BLOCK_NUM;
  }

  for(uint8_t i = 0; i < special_block_num; i++)
  {
    // ESP_LOGD(TAG, "bytes is %d", map->head_frame.type_info[i].type_id);
    if(block->block_type == map->head_frame.special_block_info[i].type_id
       && block->block_sub_type == map->head_frame.special_block_info[i].sub_type_id)
    {
      /* special block has found in flash */   
      offset = map->head_frame.special_block_info[i].offset;
      memcpy(special_block, &map->head_frame.special_block_info[i], sizeof(neurons_engine_special_block_t));
      // ESP_LOGD(TAG, "a special block have been found");
      return offset;
    }
  }
  
  if(type_num > TYPE_NUM_MAX)
  {
    type_num = TYPE_NUM_MAX;
  }
  
  for(uint8_t i = 0; i < type_num; i++)
  {
    ESP_LOGD(TAG, "the first stage index is %d", map->head_frame.type_info[i].type_id);
    if(block->block_type == map->head_frame.type_info[i].type_id)
    {
      /* have found the block in flash */   
      uint8_t data_out[2];
      offset = map->head_frame.type_info[i].offset;
      ESP_LOGD(TAG, "offset is %d", offset);
      for(uint j = 0; j < map->head_frame.type_info[i].blocks_num; j++)
      {
        f_lseek(fp, offset);
        f_read(fp, (uint8_t *)data_out, 2, &n1); // read the first two bytes(block_id & sub_id)
        ESP_LOGD(TAG, "first two bytes are %d, %d", data_out[0], data_out[1]);
        if(!memcmp(block, data_out, 2))
        {
          ESP_LOGE(TAG, "found the block in flash %d, %d", data_out[0], data_out[1]);
          special_block->offset = 0xffff; 
          return offset;
        }
        offset += BLOCK_LIB_PER_LENGtH;
      }
    }
  }
  
  special_block->offset = 0xffff;
  return 0;  
}

/* read a block information from flash by offset */
/* before call this function, the lib file must have been opened */  
void neurons_engine_get_blcok_info_t(FIL *fp, neurons_block_type_inf_t *block, uint16_t offset, 
                                                    void* data_out, neurons_engine_special_block_t *special_block)
{
  UINT n1;
  ESP_LOGD(TAG, "block is %d, %d, offset is %d, is special block: %d", block->block_type, block->block_sub_type, offset, special_block->offset);

  f_lseek(fp, offset);
  /* it is a special block */
  if(special_block->offset == offset)
  {
    ESP_LOGD(TAG, "special blcok info read")
    uint16_t size = (special_block->command_num + special_block->respond_num) * REC_CMD_ITEM_PER_LEN + 2;
    f_read(fp, (uint8_t *)data_out, size, &n1);
#if 0
    for(uint8_t i = 0; i < size; i++)
    {
      ESP_LOGD(TAG, "special lib data %d is %d\n", i,  *(uint8_t *) (data_out + i));
    }
#endif
  }
  /* it is a general block */ 
  else
  { 
    // ESP_LOGD(TAG, "general blcok info read")
    f_read(fp, (uint8_t *)data_out, sizeof(neurons_block_type_t), &n1);
#if 0
    for(uint8_t i = 0; i < sizeof(neurons_block_type_t); i++)
    {
      ESP_LOGD(TAG, "general lib data %d is %d\n", i,  *(uint8_t *) (data_out + i));
    }
#endif
  }
} 

bool neurons_engine_get_respond_info_from_lib_t(uint8_t *block_lib_t, void **respond_type_out, 
                                                respond_data_t *respond_data_out, neurons_engine_special_block_t *special_block)
{
  if(block_lib_t == NULL)
  {
    return false;
  }

  uint8_t respond_data_type_num = 0;
  uint8_t respond_data_store_num = 0;

  uint8_t respond_num = 0;
  if(special_block != NULL)
  {
    respond_num = special_block->respond_num;
    /* add one byte to store the offset of received buffer */
    respond_data_type_num = respond_num * (REC_CMD_ITEM_PER_LEN + 1) + 1;
    ESP_LOGD(TAG, "special block respond data bytes num is %d", respond_data_type_num);
  }
  else
  {
    for(uint8_t i = 0; i < RESPOND_ITEM_MAX; i++)
    {
      if(*(block_lib_t + REC_CMD_ITEM_PER_LEN * i + 2) == RES_CMD_OVER_FLAG_DATA)
      {       
        respond_num = i;
        break;
      }
    }
    /* add one byte to store the offset of received buffer */
    respond_data_type_num = respond_num * (REC_CMD_ITEM_PER_LEN + 1) + 1;
    ESP_LOGD(TAG, "general block respond data bytes num is %d", respond_data_type_num);
  }
  
  uint8_t *respond_data_type = NULL;
  uint8_t *respond_data_store = NULL;
  respond_data_type = heap_caps_malloc(respond_data_type_num, MALLOC_CAP_8BIT);
  
  if(respond_data_type == NULL)
  {
    *respond_type_out = NULL;
    respond_data_out->respond_data_buffer = NULL;
    respond_data_out->respond_data_buffer_size = 0;
    ESP_LOGE(TAG, "respond type heap over");
    return false;
  }

  uint8_t off = 0;
  *respond_data_type = respond_num; // the first byte store the respond number
  for(uint k = 0; k < respond_num; k++)
  {
    memcpy(respond_data_type + k * MODULE_ONLINE_INFO_SINGLE_RESPOND_LEN + 1, block_lib_t + k * REC_CMD_ITEM_PER_LEN + 2, REC_CMD_ITEM_PER_LEN);
    memcpy(1 + respond_data_type + (k + 1) * MODULE_ONLINE_INFO_SINGLE_RESPOND_LEN - 1, &off, 1);

    /* the first byte is the respond id, skip */
    uint8_t respond_data_index = 1;
    while((respond_data_type[respond_data_index] != 0) && (respond_data_index <= 5))
    { 
      uint16_t temp_index = k * MODULE_ONLINE_INFO_SINGLE_RESPOND_LEN + 1;
      uint8_t temp = respond_data_type[temp_index + respond_data_index];
      /* a special type */
      /* the next data byte show the number */
      if((temp & 0xf0) == ALTERABLE_NUM_BYTE)
      {
        /* wrong format */
        if(respond_data_index == 5)
        {
          heap_caps_free(respond_data_type);
          respond_data_type = NULL;
          *respond_type_out = NULL;
          respond_data_out->respond_data_buffer = NULL;
          respond_data_out->respond_data_buffer_size = 0;
          ESP_LOGE(TAG, "wrong format of respond dtype group");          
          return false;
        }
        ESP_LOGD(TAG, "respond buffer max is %d", respond_data_type[temp_index + respond_data_index + 1]);
        off += respond_data_type[temp_index + respond_data_index + 1] *  firmata_get_data_type_lenth_t(temp & 0x0f);
      }
      else if((temp & 0xf0) == 0)
      {
        off += firmata_get_data_type_lenth_t(temp & 0x0f);
      }
      else
      {
        off += ((temp & 0xf0) >> 4) * firmata_get_data_type_lenth_t(temp & 0x0f);

      }
      respond_data_store_num = off;
      ESP_LOGD(TAG, "respond_data_store_num is%d", respond_data_store_num);
      respond_data_index++;
    }
    ESP_LOGD(TAG, "respond data read from flash");
  }
  *respond_type_out = respond_data_type;

  respond_data_store = heap_caps_malloc(respond_data_store_num, MALLOC_CAP_8BIT);
  if(respond_data_store_num != 0 && respond_data_store == NULL)
  {
    heap_caps_free(*respond_type_out);
    *respond_type_out = NULL;
    respond_data_out->respond_data_buffer = NULL;
    respond_data_out->respond_data_buffer_size = 0;
    ESP_LOGE(TAG, "respond data heap over\n");
    return false;
  }
  respond_data_out->respond_data_buffer = respond_data_store;
  respond_data_out->respond_data_buffer_size = respond_data_store_num;

  return true;
}

void *neurons_engine_get_command_info_from_lib_t(uint8_t *block_lib_t, neurons_engine_special_block_t *special_block)
{
  uint8_t cmd_data_num = 0;
  neurons_block_type_t *block_lib = NULL;
  
  if(special_block == NULL)
  {
    block_lib = (neurons_block_type_t *)block_lib_t;
    for(uint8_t i = 0; i < COMMAND_ITEM_MAX + 1; i++)
    {
      if(block_lib->neurons_command_type[i].data_type[0] == RES_CMD_OVER_FLAG_DATA || i == RESPOND_ITEM_MAX)
      {
        cmd_data_num = i * REC_CMD_ITEM_PER_LEN + 1;
        uint8_t *cmd_data = NULL;
        cmd_data = heap_caps_malloc(cmd_data_num, MALLOC_CAP_8BIT);
        *cmd_data = i;
        memcpy(cmd_data + 1, block_lib->neurons_command_type, cmd_data_num - 1);
#if 0
        for(uint8_t j = 0; j < cmd_data_num; j++)
        {
          ESP_LOGD(TAG, "command data type  is %d", block_lib->neurons_command_type[0].data_type[j]);
        }
#endif
        return cmd_data;
      }
    }
  }
  else
  {
    cmd_data_num = special_block->command_num * REC_CMD_ITEM_PER_LEN + 1;
    uint8_t *cmd_data = heap_caps_malloc(cmd_data_num, MALLOC_CAP_8BIT);
    (*cmd_data) = special_block->command_num; /* the first byte store the number of command */
    memcpy(cmd_data + 1, (block_lib_t + 2) + special_block->respond_num * REC_CMD_ITEM_PER_LEN, cmd_data_num - 1);
    for(uint8_t i = 0; i < cmd_data_num; i++)
    {
      ESP_LOGD(TAG, "special block command data %d is %d", i, *(cmd_data + i));
    }
    return cmd_data;
  }
  return NULL;
}

void neurons_engine_show_lib_file_head_t(neurons_engine_flash_lib_file_head_t *head_frame)
{
  ESP_LOGD(TAG, "library version is %s", (char *)head_frame->head_frame.version_info);
  ESP_LOGD(TAG, "special blocks num is %d", head_frame->head_frame.special_block_num);
  ESP_LOGD(TAG, "blcok type is %d", head_frame->head_frame.type_num);
  
}

STATIC uint8_t neurons_file_head_verify(neurons_engine_flash_lib_file_head_t *head_frame)
{
  if(!strcmp((const char *)head_frame->head_frame.magic, NEURONS_FILE_HEAD_MAGIC))
  {
    ESP_LOGD(TAG, "neurons file head verify succeed, magic is %s", (const char *)head_frame->head_frame.magic);
    return 1;
  }
  else
  {
    ESP_LOGD(TAG, "neurons file head verify failed, magic is %s", (const char *)head_frame->head_frame.magic);
    return 0;
  }
}

/* it's better to read this information and store in RAM when start neurons engine */
bool neurons_engine_read_head_from_flash_lib_t(neurons_engine_flash_lib_file_head_t *head_frame)
{
  uint32_t file_char_num = sizeof(neurons_engine_flash_lib_file_head_t);

  if(xSemaphoreTake(g_fatfs_sema, 5000 / portTICK_PERIOD_MS) == pdTRUE)
  {
    f_chdir(&codey_sflash_vfs_fat->fatfs, "/lib");
    if(FR_OK == f_stat(&codey_sflash_vfs_fat->fatfs, NEURONS_ENGINE_LIB_FILE_NAME, NULL)) 
    {
      FIL fp1;
      UINT n1;
      f_chdir(&codey_sflash_vfs_fat->fatfs, "/lib");
      f_open(&codey_sflash_vfs_fat->fatfs, &fp1, NEURONS_ENGINE_LIB_FILE_NAME, FA_READ);

      ESP_LOGD(TAG, "file head size is %d", file_char_num);
      
      f_read(&fp1, head_frame, file_char_num, &n1);
      f_close(&fp1);
      xSemaphoreGive(g_fatfs_sema);

      neurons_engine_show_lib_file_head_t(head_frame);
      if(neurons_file_head_verify(head_frame))
      {
        return true;
      }
      {
        return false;
      }
    }
    else
    {
      ESP_LOGE(TAG, "the lib file is not found");
      xSemaphoreGive(g_fatfs_sema);
      return false;
    }
  }
  else
  {  
    ESP_LOGD(TAG, "read head have not gotten thw sema within 5 seconds");
    return false;
  }
}


