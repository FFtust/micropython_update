/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_task.h"

#include "soc/cpu.h"

#include "ff.h"
#include "diskio.h"
#include "ffconf.h"
#include "mb_fatfs/pybflash.h"
#include "mb_fatfs/drivers/sflash_diskio.h"
#include "extmod/vfs_fat.h"

#include "py/stackctrl.h"
#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "lib/mp-readline/readline.h"
#include "lib/utils/pyexec.h"
#include "uart.h"

#include "mb_ftp/codey_ftp_task.h"
#include "mb_ftp/codey_ftp.h"
#include "modmachine.h"
#include "codey_h_class.h"
#include "codey_sys.h"

#include "mpthreadport.h"
#include "codey_ble.h"
#include "codey_utils.h"
#include "codey_ble_sys_dev_func.h"
#include "esp_log.h"
#include "codey_neurons_ftp.h"
#include "codey_voice.h"
#include "codey_neurons_deal.h"

#include "codey_config.h"
#include "codey_sys_operation.h"

#define TAG "MAIN_ESP32"
// MicroPython runs as a task under FreeRTOS
#define MP_TASK_PRIORITY        (ESP_TASK_PRIO_MIN + 1)

#define MP_TASK_STACK_SIZE      (8 * 1024)
#define MP_TASK_STACK_LEN       (MP_TASK_STACK_SIZE / sizeof(StackType_t))
#define MP_TASK_HEAP_SIZE       (70 * 1024)

STATIC StaticTask_t mp_task_tcb;
STATIC StackType_t mp_task_stack[MP_TASK_STACK_LEN] __attribute__((aligned (8)));
#if CODEY_MP_TASK_HEAP_FROM_BSS
STATIC uint8_t mp_task_heap[MP_TASK_HEAP_SIZE];
#endif
/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
fs_user_mount_t *codey_sflash_vfs_fat;

static char fresh_main_py[] = "# main.py -- put your code here!\r\n"
                              "import codey\r\n"
                              "import time\r\n"
                              "while True:\r\n"
                              "    codey.show(\"write new script\")\r\n"
                              "    time.sleep(1)";
static char fresh_boot_py[] = "# boot.py -- run on boot-up\r\n"
                              "import codey\r\n"
                              "import time\r\n"
                              "while True:\r\n"
                              "    codey.show(\"write new script\")\r\n"
                              "    time.sleep(1)";

                                          
STATIC void mptask_create_main_py (void)
{
  // create empty main.py
  FIL fp;
  f_chdir(&codey_sflash_vfs_fat->fatfs,"/flash");
  f_open(&codey_sflash_vfs_fat->fatfs, &fp, "main.py", FA_WRITE | FA_CREATE_ALWAYS);
  UINT n;
  f_write(&fp, fresh_main_py, sizeof(fresh_main_py) - 1 /* don't count null terminator */, &n);
  f_close(&fp);
}

#if CODEY_FILESYSTEM_TEST

#else
static void mp_task_restart_init_filesystem(void)
{
  if(xSemaphoreTake(g_fatfs_sema, 10000 / portTICK_PERIOD_MS) == pdTRUE)
  {
    fs_user_mount_t *vfs_fat = codey_sflash_vfs_fat;
    vfs_fat->flags = 0;
    pyb_flash_init_vfs(vfs_fat);

    mp_vfs_mount_t *vfs = m_new_obj_maybe(mp_vfs_mount_t);
   
    if (vfs == NULL) 
    {
      ESP_LOGE(TAG, "file system fatal error");
    }

    vfs->str = "/flash";
    vfs->len = 6;
    vfs->obj = MP_OBJ_FROM_PTR(vfs_fat);
    vfs->next = NULL;
    MP_STATE_VM(vfs_mount_table) = vfs;
    MP_STATE_PORT(vfs_cur) = vfs;  //   It is set to the internal flash filesystem by default.
    xSemaphoreGive(g_fatfs_sema);
  }
}
#endif

STATIC void mptask_init_sflash_filesystem (void)
{
  if(xSemaphoreTake(g_fatfs_sema, portMAX_DELAY) == pdTRUE)
  {
    FILINFO fno;
#if CODEY_FILESYSTEM_TEST
  /* the python level has mount the filesystem, check this filesystem from vfs_mount_table */
  /* */
  mp_vfs_mount_t **vfsp = &MP_STATE_VM(vfs_mount_table);
  while (*vfsp != NULL) 
  {
    if(!strcmp((*vfsp)->str, "/flash")) 
    {
      ESP_LOGI(TAG, "find the mouted fatfs /flash");
      MP_STATE_PORT(vfs_cur) = (*vfsp);
      codey_sflash_vfs_fat = (*vfsp)->obj;       
      break;
    }
    vfsp = &(*vfsp)->next;
  }
  /* must execute the codes below, but the reason is not clear */
  fs_user_mount_t *vfs_fat = codey_sflash_vfs_fat;
  vfs_fat->flags = 0;
  pyb_flash_init_vfs(vfs_fat);
  f_mount(&vfs_fat->fatfs);

#else
    codey_sflash_vfs_fat = (fs_user_mount_t *)heap_caps_realloc(codey_sflash_vfs_fat, sizeof(*codey_sflash_vfs_fat), MALLOC_CAP_8BIT);
    
    fs_user_mount_t *vfs_fat = codey_sflash_vfs_fat;
    vfs_fat->flags = 0;
    pyb_flash_init_vfs(vfs_fat);
    FRESULT res = f_mount(&vfs_fat->fatfs);
    if (res == FR_NO_FILESYSTEM)
    {
      ESP_LOGI(TAG, "no file syster\n");
      uint8_t working_buf[_MAX_SS];
      res = f_mkfs(&vfs_fat->fatfs, FM_FAT | FM_SFD, 0, working_buf, sizeof(working_buf));
      f_mount(&codey_sflash_vfs_fat->fatfs);
      if (FR_OK != f_chdir (&vfs_fat->fatfs,"/flash")) 
      {
        f_mkdir(&vfs_fat->fatfs,"/flash");
      }
      else
      {
        // ESP_LOGI(TAG, "flash find!");
      }

      if (res != FR_OK)
      {
        // ESP_LOGI(TAG, "no file sys,failed to create flash");
      }
      mptask_create_main_py();
      
    }
    else if (res == FR_OK)
    {
      f_chdir(&codey_sflash_vfs_fat->fatfs, "/flash");
      if (FR_OK != f_stat(&codey_sflash_vfs_fat->fatfs, "main.py", &fno))
      {
        ESP_LOGI(TAG, "no main.py and create a new one\n");    
        mptask_create_main_py();
      }
#if CODEY_UNUSUAL_REBOOT_CHECK
      f_chdir(&codey_sflash_vfs_fat->fatfs, "/sys");
      if (FR_OK != f_stat(&codey_sflash_vfs_fat->fatfs, "script_manager.bin", &fno))
      {
        ESP_LOGI(TAG, "no script_manager.bin and create a new one\n");    
        sys_create_erase_script_count_file_t(false);
      }
      else
      {
        uint8_t erase_count = sys_clear_erase_script_count_get_t(false);
        if(erase_count >= SCRIPT_ERASE_COUNT_MAX) // erase the main.py
        {
          /* close this for conference */
          mptask_create_main_py();
          sys_clear_erase_script_count_t(false);
        }
        else
        {
          sys_clear_erase_script_count_add_t(false);
        }
      }
#endif
    }
    else
    {
      //printf("failed to create flash,res:%d\n",res);
    }
    
    mp_vfs_mount_t *vfs = m_new_obj_maybe(mp_vfs_mount_t);

    if (vfs == NULL) 
    {
      ESP_LOGE(TAG, "file system fatal error");
    }


    vfs->str = "/flash";
    vfs->len = 6;
    vfs->obj = MP_OBJ_FROM_PTR(vfs_fat);
    vfs->next = NULL;
    MP_STATE_VM(vfs_mount_table) = vfs;
    MP_STATE_PORT(vfs_cur) = vfs;  //   It is set to the internal flash filesystem by default.
#endif
    
    if (FR_OK != f_chdir (&vfs_fat->fatfs, "/sys")) 
    {
      f_mkdir(&vfs_fat->fatfs, "/sys");
    }
    if (FR_OK != f_chdir (&vfs_fat->fatfs, "/lib")) 
    {
      f_mkdir(&vfs_fat->fatfs, "/lib");
    }
    if (FR_OK != f_chdir (&vfs_fat->fatfs, "/cert"))
    {
      f_mkdir(&vfs_fat->fatfs, "/cert");
    }
    if (FR_OK != f_chdir (&vfs_fat->fatfs, "/music"))
    {
      f_mkdir(&vfs_fat->fatfs, "/music");
    }

    f_chdir(&codey_sflash_vfs_fat->fatfs,"/flash");
#if CODEY_USER_SCRIPT_SHOW_FLAG
    char *codey_read_buffer;
#endif
    if (FR_OK != f_stat(&codey_sflash_vfs_fat->fatfs, "boot.py", &fno)) 
    {
      FIL fp1;
      UINT n1;

      f_chdir (&codey_sflash_vfs_fat->fatfs, "/flash");
      f_open(&codey_sflash_vfs_fat->fatfs,&fp1, "boot.py", FA_WRITE | FA_CREATE_ALWAYS);
      f_write(&fp1, fresh_boot_py, sizeof(fresh_boot_py) - 1 /* don't count null terminator */, &n1);
      f_close(&fp1);
    }
    else
    {   
#if CODEY_USER_SCRIPT_SHOW_FLAG
      FIL fp2;
      UINT n2;
      uint32_t file_char_num = 0;
      
      f_chdir(&codey_sflash_vfs_fat->fatfs,"/flash");
      f_open(&codey_sflash_vfs_fat->fatfs,&fp2, "boot.py", FA_READ); //fftust: can not add FA_CREATE_ALWAYS
      file_char_num = f_size(&fp2);
      codey_read_buffer = (char *)malloc(file_char_num+2);
      f_read(&fp2, codey_read_buffer, file_char_num, &n2);
      f_close(&fp2);
      codey_read_buffer[file_char_num] = '\n';
      codey_read_buffer[file_char_num + 1] = '\0';
      printf(codey_read_buffer);
      free(codey_read_buffer);
#endif
    }

    f_chdir (&codey_sflash_vfs_fat->fatfs,"/flash");
    if (FR_OK != f_stat(&codey_sflash_vfs_fat->fatfs,"main.py", &fno)) 
    {
      mptask_create_main_py();
      ESP_LOGI(TAG, "main.py can't find,created a new one!");
    }
    else
    {   
#if CODEY_USER_SCRIPT_SHOW_FLAG
      FIL fp3;
      UINT n3;
      uint32_t file_char_num = 0;
      
      f_chdir(&codey_sflash_vfs_fat->fatfs,"/flash");
      f_open(&codey_sflash_vfs_fat->fatfs,&fp3, "main.py", FA_READ); //fftust: can not add FA_CREATE_ALWAYS
      file_char_num = f_size(&fp3);
      codey_read_buffer = (char *)malloc(file_char_num + 2);
      f_read(&fp3, codey_read_buffer, file_char_num, &n3);
      f_close(&fp3);
      codey_read_buffer[file_char_num] = '\n';
      codey_read_buffer[file_char_num+1] = '\0';
      printf(codey_read_buffer);
      free(codey_read_buffer);
#endif
    }
  }
  xSemaphoreGive(g_fatfs_sema); 
  codey_resource_manager_set_file_system_status_t(true);
  codey_filesystem_init_hook_t();
}

void mp_task(void *pvParameter)
{   
    static bool first_exec=true;
    volatile uint32_t sp = (uint32_t)get_sp();
#if MICROPY_PY_THREAD
    mp_thread_init(&mp_task_stack[0], MP_TASK_STACK_SIZE);
#endif
    uart_init();

    size_t free_8, free_32, free_heap;
#if CODEY_MP_TASK_HEAP_FROM_BSS
#else
    void *heap = heap_caps_malloc(MP_TASK_HEAP_SIZE, MALLOC_CAP_MP_HEAP);
#endif
    while(!codey_task_get_enable_flag_t(CODEY_MP_TASK_ID))
    {
      vTaskDelay(10);
    }
soft_reset:

    free_8 = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    free_32 = heap_caps_get_free_size(MALLOC_CAP_32BIT);
    free_heap = heap_caps_get_free_size(MALLOC_CAP_MP_HEAP);
    ESP_LOGD(TAG, "free heap 8bit size is %d, 32bit size is %d, heap size is %d\n", free_8, free_32, free_heap);
    // initialise the stack pointer for the main thread
    mp_stack_set_top((void *)sp);
    mp_stack_set_limit(MP_TASK_STACK_SIZE - 1024);
    
#if CODEY_MP_TASK_HEAP_FROM_BSS
    gc_init(mp_task_heap, mp_task_heap + sizeof(mp_task_heap));
#else
    memset(heap, 0, MP_TASK_HEAP_SIZE);
    gc_init(heap, (char*)heap + MP_TASK_HEAP_SIZE);
#endif
   
    mp_init();
    mp_obj_list_init(mp_sys_path, 0);
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_lib));

    mp_obj_list_init(mp_sys_argv, 0);
    // readline_init0();
    machine_pins_init();
    gc_info_t info;
    gc_info(&info);
    ESP_LOGD(TAG, "_boot start, the mp heap free is %d", info.free);
    pyexec_frozen_module("_boot.py");
    gc_info(&info);
    ESP_LOGD(TAG, "_boot end, the mp heap free is %d", info.free);
    if(first_exec == true)
    {
      /* this must be called befor init filesyster */
      /* but the reason is not clear */
#if CODEY_FTP_TASK_ENABLE
      pyexec_frozen_module("esp32_init_ftp.py");
      vTaskDelay(10);
#endif
      mptask_init_sflash_filesystem();

      first_exec = false;
    }  
    else
    {
#if CODEY_FILESYSTEM_TEST
      mptask_init_sflash_filesystem();
#else
      mp_task_restart_init_filesystem();
#endif
    }
   
    /*run boot-up scripts*/
    for (;;)
    {
#if CODEY_MICROPYTHON_REPL_ENABLE
      for(;;)
      {
        if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL)
        {
          if (pyexec_raw_repl() != 0)
          {
            break;
          }
        }
        else if(pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL)
        {
          if (pyexec_friendly_repl() != 0)
          { 
            break;
          }
        }
        else
        {
          vTaskDelay(100/portTICK_PERIOD_MS);
        }
      }
#endif
      uint8_t pyexe_ret = 0;
      if(1 == get_user_script_interrupt_reason()) 
      {
        ESP_LOGI(TAG, "user script out");
        pyexec_frozen_module("esp32_pass.py"); // it's a forever loop,but donothing;it's necessary
        break;
      }
      else
      {
        ESP_LOGI(TAG, "in user script");
        set_user_script_interrupt_reason(1); // when the main.py is excecuting,this flag is set to be 1;
        if(1)
        {
          codey_script_update_after_operation();
          // xSemaphoreTake(g_fatfs_sema, portMAX_DELAY); 
          // xSemaphoreGive(g_fatfs_sema); 
          
          f_chdir(&codey_sflash_vfs_fat->fatfs,"/flash");
          /* this file just register the callback 
             don't execute any function */
          pyexe_ret = pyexec_file("main.py");
          if(pyexe_ret == 0)
          {
            ESP_LOGE(TAG, "main.py error occured");
          }
          /* executing user codes here is enabled */
          if(get_user_script_interrupt_reason() == 2)
          {
            break;
          }
          /* we schedule the task here */
          // xSemaphoreTake(g_fatfs_sema, portMAX_DELAY); 
          // xSemaphoreGive(g_fatfs_sema); 
          f_chdir(&codey_sflash_vfs_fat->fatfs,"/flash");
          pyexe_ret = pyexec_frozen_module("codey_main_loop.py");
          if(pyexe_ret == 0)
          {
            ESP_LOGE(TAG, "main_loop.py error occured");
          }

          ESP_LOGI(TAG, "main end, the mp heap free is %d", info.free);
        }
        
        if(get_user_script_interrupt_reason() == 2)
        {
          break;
        }
        else
        {
          ; // error or script has executed all 
        }
      } 
    }

    // resource deinit
#if MICROPY_PY_THREAD
    mp_thread_deinit();
#endif
    
    ESP_LOGI(TAG, "PYB: soft reboot\r\n");

    // deinitialise peripherals
    machine_pins_deinit();

    mp_deinit();
    fflush(stdout);
    goto soft_reset;
}


const char timestamp[] = __TIMESTAMP__;

void main_create_mp_task()
{
  g_mp_task_handle = xTaskCreateStaticPinnedToCore(mp_task, "mp_task", MP_TASK_STACK_LEN, NULL, CODEY_MP_TASK_PRIORITY + 1, &mp_task_stack[0], &mp_task_tcb, 0);
}

void app_main(void)
{
  vTaskPrioritySet(NULL, 4);
  nvs_flash_init();
  ESP_LOGE(TAG, "************before system start");
  esp32_heap_info_show_t();  
  codey_system_init_t();
  ESP_LOGE(TAG, "************after system start");
  esp32_heap_info_show_t();
  codey_main_task_hook_t();
}

void nlr_jump_fail(void *val) 
{
    printf("NLR jump failed, val=%p\n", val);
    esp_restart();
}

// modussl_mbedtls uses this function but it's not enabled in ESP IDF
void mbedtls_debug_set_threshold(int threshold) 
{
    (void)threshold;
}
