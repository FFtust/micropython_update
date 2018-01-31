/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   codey sys operation file
 * @file    codey_sys_operation.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/10/23
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
 * This file is a config for codey module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftsut            2017/10/23      1.0.0              build the new.
 * </pre>
 *
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
#include "esp_wifi.h"

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
#include "neurons_engine_list_maintain.h"
#include "neurons_engine_port.h"
#include "codey_wlan.h"
#include "codey_firmware.h"
#include "codey_config.h"
#include "codey_sys_operation.h"
#include "codey_event_mechanism.h"
#include "codey_voice.h"
#include "codey_ready_notify.h"
#include "codey_super_var.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG                   ("SYS_OPERATION")
#define ERASE_CHECKT_TIEM     (10000)  // ms
#define DTR_REQUIRE_WAIT_TIME (200) // ms
#define DTR_SIGNEL_WAIT_TIME  (3000) // ms

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
 
/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
static codey_main_py_status_t s_main_py_exec_status = CODEY_MAIN_PY_UPDATE;
static uint8_t s_codey_sensor_cur_status[CODEY_SENSORS_ID_MAX] = { 0 };
static codey_sys_task_manager_t codey_sys_task_manager[CODEY_TASK_ID_MAX];

/******************************************************************************
 DEFINE PUBLIC DATAS
 ******************************************************************************/
/* define mutexs */
SemaphoreHandle_t s_sys_fatfs_mutex = NULL;
SemaphoreHandle_t s_sys_wifi_ope_mutex = NULL;
SemaphoreHandle_t s_sys_dtr_cmd_require_sema = NULL;
SemaphoreHandle_t s_sys_dtr_cmd_respond_sema = NULL;

/* define binary semaphores */

/* define queue */

/******************************************************************************
DECLARE PRIVATE FUNCTIONS
******************************************************************************/
STATIC bool codey_sys_queue_init_t(void);

/******************************************************************************
DEFINE FFUNCTIONS
******************************************************************************/
uint8_t codey_get_main_py_exec_status_t(void)
{
  return s_main_py_exec_status;
}

void codey_set_main_py_exec_status_t(codey_main_py_status_t sta)
{
  s_main_py_exec_status = sta;
}

void codey_sensor_set_init_status_t(codey_sensors_id id, codey_sensor_init_sta sta)
{
  if(id >= CODEY_SENSORS_ID_MAX)
  {
    return;
  }
  s_codey_sensor_cur_status[id] = sta;
}

codey_sensor_init_sta codey_sensor_get_init_status_t(codey_sensors_id id)
{
  if(id >= CODEY_SENSORS_ID_MAX)
  {
    return CODEY_SENSOR_NOT_EXIT;
  }
  return s_codey_sensor_cur_status[id];
}

void codey_task_init_t(void)
{
  for(uint8_t i = 0; i < CODEY_TASK_ID_MAX; i++)
  {
    codey_sys_task_manager[i].codey_task_status = CODEY_TASK_NOT_CREATE;
    codey_sys_task_manager[i].codey_task_execute_enable = false;
    codey_sys_task_manager[i].codey_task_auto_break_flag = false;
    codey_sys_task_manager[i].run_time = 0;
    codey_sys_task_manager[i].task_handle = NULL;
  }
  
  /* config CODEY_UART_DEAL_TASK */
  codey_sys_task_manager[CODEY_UART_DEAL_TASK_ID].task_fun = codey_neurons_deal;
  codey_sys_task_manager[CODEY_UART_DEAL_TASK_ID].stack_len = CODEY_UART_DEAL_TASK_STACK_SIZE;
  codey_sys_task_manager[CODEY_UART_DEAL_TASK_ID].task_priority =  CODEY_UART_DEAL_TASK_PRIORITY;
  strcpy(codey_sys_task_manager[CODEY_UART_DEAL_TASK_ID].fun_name, "CODEY_UART_DEAL_TASK");
  codey_task_set_status_t(CODEY_UART_DEAL_TASK_ID, CODEY_TASK_NOT_CREATE);
   
  /* config CODEY_SENSORS_GET_VALUE_TASK */
  codey_sys_task_manager[CODEY_SENSORS_GET_VALUE_TASK_ID].task_fun = codey_sensors_update_task_t;
  codey_sys_task_manager[CODEY_SENSORS_GET_VALUE_TASK_ID].stack_len = CODEY_SENSORS_GET_VALUE_TASK_STACK_SIZE;
  codey_sys_task_manager[CODEY_SENSORS_GET_VALUE_TASK_ID].task_priority =  CODEY_SENSORS_GET_VALUE_TASK_PRIORITY;
  strcpy(codey_sys_task_manager[CODEY_SENSORS_GET_VALUE_TASK_ID].fun_name, "CODEY_SENSORS_GET_VALUE_TASK"); 
  codey_task_set_status_t(CODEY_SENSORS_GET_VALUE_TASK_ID, CODEY_TASK_NOT_CREATE);
  
  /* config CODEY_FTP_TASK */
  codey_sys_task_manager[CODEY_FTP_TASK_ID].task_fun = codey_ftp_task;
  codey_sys_task_manager[CODEY_FTP_TASK_ID].stack_len = CODEY_FTP_TASK_STACK_SIZE;
  codey_sys_task_manager[CODEY_FTP_TASK_ID].task_priority =  CODEY_FTP_TASK_PRIORITY;
  strcpy(codey_sys_task_manager[CODEY_FTP_TASK_ID].fun_name, "CODEY_FTP_TASK");  
  codey_task_set_status_t(CODEY_FTP_TASK_ID, CODEY_TASK_NOT_CREATE);
  
  /* config CODEY_LEDMATRIX_SHOW_TASK */
  codey_sys_task_manager[CODEY_LEDMATRIX_SHOW_TASK_ID].task_fun = codey_ledmatrix_board_show_task_t;
  codey_sys_task_manager[CODEY_LEDMATRIX_SHOW_TASK_ID].stack_len = CODEY_LEDMATRIX_SHOW_TASK_STACK_SIZE;
  codey_sys_task_manager[CODEY_LEDMATRIX_SHOW_TASK_ID].task_priority =  CODEY_LEDMATRIX_SHOW_TASK_PRIORITY;
  strcpy(codey_sys_task_manager[CODEY_LEDMATRIX_SHOW_TASK_ID].fun_name, "CODEY_LEDMATRIX_SHOW_TASK");  
  codey_task_set_status_t(CODEY_LEDMATRIX_SHOW_TASK_ID, CODEY_TASK_NOT_CREATE);
    
  /* config CODEY_MUSIC_PLAY_TASK */
  codey_sys_task_manager[CODEY_MUSIC_PLAY_TASK_ID].task_fun = NULL;
  codey_sys_task_manager[CODEY_MUSIC_PLAY_TASK_ID].stack_len = 3 * 1024;
  codey_sys_task_manager[CODEY_MUSIC_PLAY_TASK_ID].task_priority =  1;
  strcpy(codey_sys_task_manager[CODEY_MUSIC_PLAY_TASK_ID].fun_name, "CODEY_MUSIC_PLAY_TASK");  
  codey_task_set_status_t(CODEY_MUSIC_PLAY_TASK_ID, CODEY_TASK_NOT_CREATE);
}

void codey_task_set_status_t(codey_sys_task_id id, codey_task_status_t sta)
{ 
  codey_sys_task_manager[id].codey_task_status = sta;
}

codey_task_status_t codey_task_get_status_t(codey_sys_task_id id)
{
  return codey_sys_task_manager[id].codey_task_status;
}

void codey_task_set_break_flag_t(codey_sys_task_id id, bool sta)
{ 
  if(id >= CODEY_TASK_ID_MAX)
  {
    return;
  }
  codey_sys_task_manager[id].codey_task_auto_break_flag = sta;
}

bool codey_task_get_break_flag_t(codey_sys_task_id id)
{ 
  if(id >= CODEY_TASK_ID_MAX)
  {
    return false;
  }
  return codey_sys_task_manager[id].codey_task_auto_break_flag;
}

void codey_task_set_enable_flag_t(codey_sys_task_id id, bool sta)
{ 
  if(id >= CODEY_TASK_ID_MAX)
  {
    return;
  }
  codey_sys_task_manager[id].codey_task_execute_enable = sta;
}

bool codey_task_get_enable_flag_t(codey_sys_task_id id)
{ 
  if(id >= CODEY_TASK_ID_MAX)
  {
    return false;
  }
  return codey_sys_task_manager[id].codey_task_execute_enable;
}

void codey_task_create_t(codey_sys_task_id id)
{
  xTaskCreatePinnedToCore(codey_sys_task_manager[id].task_fun, codey_sys_task_manager[id].fun_name, 
                          codey_sys_task_manager[id].stack_len, NULL, codey_sys_task_manager[id].task_priority,
                          codey_sys_task_manager[id].task_handle, 0);
}

extern void main_create_mp_task();
void codey_sys_create_t(void)
{
  /* only create the task , not execute to the main loop */
#if CODEY_MP_TASK_ENABLE
  /* mp_task config is special */
  /* we config it in main.c */
  main_create_mp_task();
#endif
  
#if CODEY_LEDMATRIX_SHOW_TASK_ENABLE
  codey_task_set_status_t(CODEY_LEDMATRIX_SHOW_TASK_ID, CODEY_TASK_NOT_CREATE);
  codey_task_create_t(CODEY_LEDMATRIX_SHOW_TASK_ID);
#endif

#if CODEY_SENSORS_GET_VALUE_TASK_ENABLE
  codey_task_set_status_t(CODEY_SENSORS_GET_VALUE_TASK_ID, CODEY_TASK_NOT_CREATE);
  codey_task_create_t(CODEY_SENSORS_GET_VALUE_TASK_ID);
#endif
  
#if CODEY_UART_DEAL_TASK_ENABLE
  codey_task_set_status_t(CODEY_UART_DEAL_TASK_ID, CODEY_TASK_NOT_CREATE);
  codey_task_create_t(CODEY_UART_DEAL_TASK_ID);
#endif

#if CODEY_FTP_TASK_ENABLE
  codey_task_set_status_t(CODEY_FTP_TASK_ID, CODEY_TASK_NOT_CREATE);
  codey_task_create_t(CODEY_FTP_TASK_ID);
#endif

#if CODEY_MUSIC_PLAY_TASK_ENABLE
  codey_task_set_status_t(CODEY_MUSIC_PLAY_TASK_ID, CODEY_TASK_NOT_CREATE);
  /* music task create when needed */ 
#endif
}

void codey_start_task_t(void)
{
  /* we control the order to start task here */
#if CODEY_LEDMATRIX_SHOW_TASK_ENABLE
  codey_task_set_enable_flag_t(CODEY_LEDMATRIX_SHOW_TASK_ID, true);
  vTaskDelay(100);
#endif

#if CODEY_MP_TASK_ENABLE
  codey_task_set_enable_flag_t(CODEY_MP_TASK_ID, true);
  vTaskDelay(100);
#endif

#if CODEY_SENSORS_GET_VALUE_TASK_ENABLE
  codey_task_set_enable_flag_t(CODEY_SENSORS_GET_VALUE_TASK_ID, true);  
#endif
  
#if CODEY_UART_DEAL_TASK_ENABLE
  codey_task_set_enable_flag_t(CODEY_UART_DEAL_TASK_ID, true);
#endif
 
#if CODEY_FTP_TASK_ENABLE
  codey_task_set_enable_flag_t(CODEY_FTP_TASK_ID, true);
#endif

#if CODEY_MUSIC_PLAY_TASK_ENABLE
  codey_task_set_enable_flag_t(CODEY_MUSIC_PLAY_TASK_ID, true);
#endif 
}

void codey_script_update_pre_operation(void)
{
  uint8_t count = 0;
#if CODEY_MP_TASK_ENABLE
  /* wait ledmatrix task clear */ 
  codey_ledmatrix_show_disable();
  codey_task_set_enable_flag_t(CODEY_LEDMATRIX_SHOW_TASK_ID, false);
  codey_ledmatrix_screen_clean_t();
  ESP_LOGD(TAG, "wiat led task out start");
  count = 0;
  while(codey_task_get_status_t(CODEY_LEDMATRIX_SHOW_TASK_ID) !=  CODEY_TASK_WAIT_TO_EXECUTE)
  {
    vTaskDelay(5);
    if(count++ > 20)
    {
      count = 0;
      break;
    }
  } 
  ESP_LOGD(TAG, "wiat led task out end");
#endif

#if CODEY_SENSORS_GET_VALUE_TASK_ENABLE
  /* wait sensor_get_value clear*/
  #if 0
  codey_task_set_enable_flag_t(CODEY_SENSORS_GET_VALUE_TASK_ID, false);
  ESP_LOGD(TAG, "wiat sensor out start");
  while(codey_task_get_status_t(CODEY_SENSORS_GET_VALUE_TASK_ID) !=  CODEY_TASK_WAIT_TO_EXECUTE)
  {
    vTaskDelay(5);
  } 
  ESP_LOGD(TAG, "wiat sensor out end")
  #endif
#endif

#if CODEY_MUSIC_PLAY_TASK_ENABLE
  /* wait music task clear */
  codey_task_set_enable_flag_t(CODEY_MUSIC_PLAY_TASK_ID, false);
  codey_voice_stop_play();
  vTaskDelay(50);
#endif
  /* wait ftp task clear */

}

void codey_script_update_after_operation(void)
{
  uint8_t count = 0;
  neurons_engine_read_control_deinit_t();
  codey_eve_deinit_t();
  codey_set_main_py_exec_status_t(CODEY_MAIN_PY_EXECUTED);
 
  codey_task_set_enable_flag_t(CODEY_LEDMATRIX_SHOW_TASK_ID, true);
  codey_ledmatrix_show_enable();
  codey_ledmatrix_screen_clean_t();

  codey_task_set_enable_flag_t(CODEY_SENSORS_GET_VALUE_TASK_ID, true);  
  codey_voice_set_volume_t(100.0);
  codey_task_set_enable_flag_t(CODEY_MUSIC_PLAY_TASK_ID, true);
  codey_restart_indicate();
  codey_ready_notify();
  codey_super_var_clear();
  /* wait the neurons modules list online update */
  while(!neurons_engine_get_status_t())
  {
    vTaskDelay(50 / portTICK_PERIOD_MS);
    if(count++ > 20)
    {
      break;  
    }
  }
}

void codey_restart_indicate(void)
{
  codey_voice_stop_play();

  for(uint8_t i = 0; i < 2 ; i++)
  {
    codey_rgbled_board_set_color_t(0, 0, 100);
    vTaskDelay(180 / portTICK_PERIOD_MS);
    codey_rgbled_board_set_color_t(0, 0, 0);
    vTaskDelay(180 / portTICK_PERIOD_MS);
  }
}

void codey_ble_connected_indicate(void)
{
  codey_voice_stop_play();
  codey_rgbled_board_set_color_t(0, 0, 100);
  vTaskDelay(300 / portTICK_PERIOD_MS);
  vTaskSuspendAll();
  codey_8bit_voice_board_play_note_directly_t(1200, 200, 100);
  xTaskResumeAll();
  codey_rgbled_board_set_color_t(0, 0, 0);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  
  codey_ledmatrix_reload_t();
  codey_rgbled_board_reload_t();
}

extern void mp_thread_suspend(void);
void codey_main_task_hook_t(void)
{
#if CODEY_UNUSUAL_REBOOT_CHECK
  static bool script_erase_count_flag = false;
#endif
  uint8_t count = 0;
  while(1)
  {
    if(sys_get_restart_flag())
    {
      codey_script_update_pre_operation();
      ESP_LOGD(TAG, "codey_script_update_pre_operation done");
      if(xSemaphoreTake(g_fatfs_sema, 0) == pdTRUE)
      { 
        if(xSemaphoreTake(s_sys_dtr_cmd_require_sema, DTR_REQUIRE_WAIT_TIME / portTICK_PERIOD_MS) == pdTRUE)
        {
          // there is a DTR require
          xSemaphoreGive(s_sys_dtr_cmd_respond_sema);
          vTaskDelay(DTR_SIGNEL_WAIT_TIME / portTICK_PERIOD_MS);
        }
        vTaskPrioritySet(g_mp_task_handle, CODEY_MP_TASK_PRIORITY + 3);
        mp_thread_suspend();
        vTaskDelay(10);
        ESP_LOGD(TAG, "son thread suspend succeed");
#if CODEY_SOFT_RESTART_AFTER_SCRIPT_UPDATE
        /* must add GIT_EXIT here, because the mp_task may be blocked by GIL */
        MP_THREAD_GIL_EXIT(); 
        vTaskDelay(50 / portTICK_PERIOD_MS);
        user_script_interrupt();
        xSemaphoreGive(g_fatfs_sema);
        count = 1;
        while(codey_get_main_py_exec_status_t() != CODEY_MAIN_PY_EXECUTED)
        {
          vTaskDelay(50 / portTICK_PERIOD_MS);
          if(count++ % 20 == 0)
          {
            ESP_LOGD(TAG, "user_script_interrupt again");
            MP_THREAD_GIL_EXIT(); 
            vTaskDelay(10);
            user_script_interrupt();
          }
          if(count > 100)
          {
            ESP_LOGE(TAG, "python restart failed, please power off");
            break;
          }
        }
        sys_set_restart_flag(false);
        ESP_LOGD(TAG, "python restart succeed");
        vTaskPrioritySet(g_mp_task_handle, CODEY_MP_TASK_PRIORITY);
        codey_voice_set_volume_t(100);
        codey_voice_stop_play();
#else
        esp_restart();
#endif        
      }
    } 
#if CODEY_SOFT_POWER_SWITCH 
    if(count >= 20)
    {
      count = 0;
    }
#if CODEY_UNUSUAL_REBOOT_CHECK
    if(!script_erase_count_flag)
    {
      if(millis() > ERASE_CHECKT_TIEM)
      {
        sys_clear_erase_script_count_t(true);
        script_erase_count_flag = true;
      }
    }
#endif
    codey_board_power_status_check_t();
#endif
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP_LOGD(TAG, "main task running");
    count++;
  }
}

void codey_filesystem_init_hook_t(void)
{ 
  s_codey_sensor_cur_status[CODEY_NEURONS_ENGINE] = neurons_engine_init_t();  
  ESP_LOGI(TAG, "neurons engine init status is %d", s_codey_sensor_cur_status[CODEY_NEURONS_ENGINE]);
}

bool codey_mutex_lock(SemaphoreHandle_t mut, int wait)
{
  return (pdTRUE == xSemaphoreTake(mut, wait ? portMAX_DELAY : 0));
}

void codey_mutex_unlock(SemaphoreHandle_t mut)
{
  xSemaphoreGive(mut);
}

bool codey_filesystem_mutex_lock(void)
{
  bool ret = false;
  ret = codey_mutex_lock(s_sys_fatfs_mutex, 1);
  return ret;
}
void codey_filesystem_mutex_unlock(void)
{
  codey_mutex_unlock(s_sys_fatfs_mutex);
}

void codey_system_information_show_t(void)
{
  uint8_t wifi_ap_mac[13] = { 0 };
  codey_wifi_mac_to_string_t(1, wifi_ap_mac);
  printf("***system information***\n");
  printf("the version of codey is %s\n", codey_get_firmware());
  printf("the address of wifi(ap) is %s\n", wifi_ap_mac);
  printf("the address of ble is %s\n", " ");
  if(sys_music_file_check_t(true))
  {
    printf("music files is existed\n");
  }
  else
  {
    printf("music files is not existed\n");
  }
  printf("\n");
}

void codey_system_init_t(void)
{
#if CODEY_PERIPHERAL_LOG_CLOSE
  esp_log_level_set("*", ESP_LOG_NONE); // this function close all the logs about all the peripherals
#endif
  codey_resource_manager_init();
  codey_eve_init_t();
  codey_sys_queue_init_t();
  codey_ble_register_connected_func(codey_ble_connected_indicate);
  codey_task_init_t();    
  /* try to init filesystem here, but now init in the mp_task */
  /* init senors on codey */
  codey_sensors_init_t();
#if CODEY_UART_COMMUNICATION_ENABLE
  extern void codey_neurons_uart_rec_start(void);
  codey_neurons_uart_rec_start();
#endif

#if CODEY_BT_COMMUNICATION_ENABLE
  extern int codey_ble_enter_sys_dev_mode(void);
  codey_ble_enter_sys_dev_mode();
#endif
#if CODEY_LOW_ENERGY_POWER_OFF
  codey_battery_check_get_value_t();
#endif
  /* system task create */
  codey_sys_create_t();
  codey_start_task_t();
  while(!codey_resource_manager_get_file_system_status_t())
  {
    vTaskDelay(10);
  }
  vTaskDelay(100);
  codey_system_information_show_t();
  vTaskDelay(200);
#if CODEY_BT_COMMUNICATION_ENABLE
    extern int codey_ble_enter_sys_dev_mode(void);
    printf("BLE begine");
    codey_ble_enter_sys_dev_mode();
    printf("BLE end");
#endif

}

/******************************************************************************
DEFINE PRIVATE FUNCTIONS
******************************************************************************/
STATIC bool codey_sys_queue_init_t(void)
{
  /* protect the file sytem */
  s_sys_fatfs_mutex = xSemaphoreCreateMutex();
  if(s_sys_fatfs_mutex == NULL) return false;

  /* because the IOT(python) operate the wifi resource directly, set a mutex to protect system */
  s_sys_wifi_ope_mutex = xSemaphoreCreateMutex();
  if(s_sys_wifi_ope_mutex == NULL) return false;
  
  /* use this sema to sync DTR reset */
  s_sys_dtr_cmd_require_sema = xSemaphoreCreateBinary();
  if(s_sys_dtr_cmd_require_sema == NULL) return false;
  
  s_sys_dtr_cmd_respond_sema = xSemaphoreCreateBinary();
  if(s_sys_dtr_cmd_respond_sema == NULL) return false;
  return true;
}

