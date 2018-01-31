/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   The basis of the function for makeblock.
 * @file    codey_sys.c
 * @author  Mark Yan
 * @version V1.0.0
 * @date    2017/03/30
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
 *   Mark Yan        2017/03/30     1.0.0            build the new.
 * </pre>
 *
 */
 
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "codey_sys.h"
#include "mphalport.h"

#include "py/stackctrl.h"
#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "extmod/vfs_fat.h"
#include "codey_esp32_resouce_manager.h"
#include "esp_log.h"
#include "mb_fatfs/drivers/sflash_diskio.h"
#include "esp_heap_caps.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG                           ("codey_sys") 
#define SCRIPT_ERASE_COUNT_FILE_NAME  ("script_manager.bin")
#define MUSIC_FILE_MAX                (32)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
typedef struct
{
  char music_name[13];
}music_name_t;

const music_name_t music_name[MUSIC_FILE_MAX] = {{"cat.wav"}};

/********************************************************************
 DEFINE PRIVATE DATAS
 ********************************************************************/

/********************************************************************
 DEFINE PUBLIC DATAS
 ********************************************************************/
/* this global var is used to control the rebooting of user script */
uint8_t g_user_script_interrupt_reason = 0;
/* this global var is used to choose which factory script to execute */
/* 0 for main.py */
uint8_t g_factory_script_exec_index = 0;
/* to control the executing of user script */
uint8_t g_user_script_execute_status_flag = 0; // 1:execute 2:stop 

uint8_t g_erase_script_count = 0;
/********************************************************************
 DEFINE PUBLIC FUNCTION
 ********************************************************************/
uint32_t micros(void)
{
  return mp_hal_ticks_us();
}

uint32_t millis(void)
{
  return (xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/* this function will interrupt the main.py */
void user_script_interrupt(void)  
{
  MP_STATE_VM(mp_pending_exception) = MP_OBJ_FROM_PTR(&MP_STATE_VM(mp_kbd_exception));
#if MICROPY_ENABLE_SCHEDULER
  if(MP_STATE_VM(sched_state) == MP_SCHED_IDLE) 
  {
    MP_STATE_VM(sched_state) = MP_SCHED_PENDING;
  }
#endif
}

uint8_t get_user_script_interrupt_reason(void)
{
  return g_user_script_interrupt_reason;
}

void set_user_script_interrupt_reason(uint8_t value)
{
  g_user_script_interrupt_reason = value;
}

/* */
uint8_t get_factory_script_exec_index(void)
{
  return g_factory_script_exec_index;
}

void set_factory_script_exec_index(uint8_t value)
{
  g_factory_script_exec_index = value;
}

void set_factory_script_exec_index_next(void)
{
  g_factory_script_exec_index++;
  g_factory_script_exec_index %= 4;
}

/* */
uint8_t get_user_script_exec_flag()
{
  return g_user_script_execute_status_flag;
}

void set_user_script_exec_flag(uint8_t value)
{
  g_user_script_execute_status_flag = value;
}

void sys_create_erase_script_count_file_t(bool wait_sema)
{
  bool give_flag = false;
  if(wait_sema)
  {
    if(xSemaphoreTake(g_fatfs_sema, 5000 / portTICK_PERIOD_MS) == pdTRUE)
    {
      give_flag = true;
    }
    else
    {
       return;
    }
  }
  g_erase_script_count = 0;
  /* write to flash sys/script_manager.bin*/
  FIL fp;
  f_chdir(&codey_sflash_vfs_fat->fatfs, "/sys");
  f_open(&codey_sflash_vfs_fat->fatfs, &fp, SCRIPT_ERASE_COUNT_FILE_NAME, FA_WRITE | FA_CREATE_ALWAYS);
  UINT n;
  f_write(&fp, &g_erase_script_count, 1 /* only one byte*/, &n);
  f_close(&fp);
  if(give_flag)
  {
    xSemaphoreGive(g_fatfs_sema);
  }
}

void sys_clear_erase_script_count_t(bool wait_sema)
{
  bool give_flag = false;
  if(wait_sema)
  {
    if(xSemaphoreTake(g_fatfs_sema, 5000 / portTICK_PERIOD_MS) == pdTRUE)
    {
      give_flag = true;
    }
    else
    {
       return;
    }
  }
    
  g_erase_script_count = 0;
  /* write to flash sys/script_manager.bin*/
  FIL fp;
  f_chdir(&codey_sflash_vfs_fat->fatfs, "/sys");
  f_open(&codey_sflash_vfs_fat->fatfs, &fp, SCRIPT_ERASE_COUNT_FILE_NAME, FA_WRITE | FA_CREATE_ALWAYS);
  UINT n;
  f_write(&fp, &g_erase_script_count, 1 /* only one byte*/, &n);
  f_close(&fp);

  if(give_flag)
  {
    xSemaphoreGive(g_fatfs_sema);
  }
}

uint8_t sys_clear_erase_script_count_get_t(bool wait_sema)
{
  bool give_flag = false;
  if(wait_sema)
  {
    if(xSemaphoreTake(g_fatfs_sema, 5000 / portTICK_PERIOD_MS) == pdTRUE)
    {
      give_flag = true;
    }
    else
    {
       return 0 ;
    }
  }

  g_erase_script_count++;
  /* write to flash sys/script_manager.bin*/
  FIL fp;
  f_chdir(&codey_sflash_vfs_fat->fatfs, "/sys");
  f_open(&codey_sflash_vfs_fat->fatfs, &fp, SCRIPT_ERASE_COUNT_FILE_NAME, FA_READ);
  UINT n;
  f_read(&fp, &g_erase_script_count, 1, &n);
  f_close(&fp);
  if(give_flag)
  {
    xSemaphoreGive(g_fatfs_sema);
  }
  return g_erase_script_count;
}

void sys_clear_erase_script_count_add_t(bool wait_sema)
{
  bool give_flag = false;
  if(wait_sema)
  {
    if(xSemaphoreTake(g_fatfs_sema, 5000 / portTICK_PERIOD_MS) == pdTRUE)
    {
      give_flag = true;
    }
    else
    {
       return;
    }
  }

  g_erase_script_count++;
  /* write to flash sys/script_manager.bin*/
  
  FIL fp;
  f_chdir(&codey_sflash_vfs_fat->fatfs, "/sys");
  f_open(&codey_sflash_vfs_fat->fatfs, &fp, SCRIPT_ERASE_COUNT_FILE_NAME, FA_WRITE | FA_CREATE_ALWAYS);
  UINT n;
  f_write(&fp, &g_erase_script_count, 1 /* only one byte*/, &n);
  f_close(&fp);
  
  if(give_flag)
  {
    xSemaphoreGive(g_fatfs_sema);
  }
}

uint8_t sys_music_file_check_t(bool wait_sema)
{
  bool give_flag = false;
  FILINFO fno;
  uint8_t ret = 0;
  
  if(wait_sema)
  {
    if(xSemaphoreTake(g_fatfs_sema, 5000 / portTICK_PERIOD_MS) == pdTRUE)
    {
      give_flag = true;
    }
    else
    {
      return 0;
    }
  }

  f_chdir(&codey_sflash_vfs_fat->fatfs, "/music");
  if(FR_OK != f_stat(&codey_sflash_vfs_fat->fatfs, "cat.wav", &fno))
  {
    ESP_LOGD("music check", "music file is not exit");    
    ret = 0;
  }
  else
  {
    ret = 1;
  }
  if(give_flag)
  {
    xSemaphoreGive(g_fatfs_sema);
  }
  return ret;
}

void read_all_filesystem_test_t()
{
  uint8_t *buffer;
  buffer = heap_caps_malloc(4096, MALLOC_CAP_8BIT);
  uint32_t addr = 0;
  
  for(uint16_t j = 0; j < 256; j++)
  {  
    uint8_t ret = 0;
    ret = sflash_disk_read(buffer, addr++, 1);
    if(ret != RES_OK)
    {
      printf("read error ret is %d\n", ret);
      continue;
    }
    for(uint8_t i = 0; i < 16; i++)
    {
      uart_write_bytes(UART_NUM_0, (const char *)buffer + i * 256, 256);
    }
    // printf("the addr is %d\n", addr);
  }
  
  heap_caps_free(buffer);
}

void esp32_heap_info_show_t()
{
  size_t free_8, free_32, free_heap;
  free_8 = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  free_32 = heap_caps_get_free_size(MALLOC_CAP_32BIT);
  free_heap = heap_caps_get_free_size(MALLOC_CAP_MP_HEAP);
  printf("free heap 8bit size is %d\n, 32bit size is %d\n, heap size is %d\n", free_8, free_32, free_heap);
}
