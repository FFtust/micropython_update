/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   resouce manager 
 * @file    codey_esp32_resouce_manager.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/06/28
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
 * This file is a drive resouce_manager module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/06/28      1.0.0              build the new.
 * </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_task.h"

#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/nlr.h"
#include "py/obj.h"

#include "codey_h_class.h"
#include "codey_sys.h"

#include "codey_utils.h"
#include "codey_esp32_resouce_manager.h"

/************************************************************************************
 DEFINE MACROS
 ************************************************************************************/

/************************************************************************************
 DEFINE TYPES & CONSTANTS
 ************************************************************************************/
typedef enum
{
  USER_TASK,
  COMMUNICATION_TASK,
  SENSOR_VALUE_TASK,
}codey_task_type_t;

/************************************************************************************
 DEFINE PRIVATE DATAS
************************************************************************************/

/************************************************************************************
 DEFINE PUBLIC DATAS
************************************************************************************/
TaskHandle_t g_mp_task_handle = NULL;
SemaphoreHandle_t g_fatfs_sema = NULL;  // the music player & the ftp & neurons file transmission have the possibily to  operate the fatfs in the same time 

uint32_t g_codey_system_status = 0x00000000;
uint8_t g_neurons_engine_status = 0;
bool g_esp_restart_flag = false;
uint8_t g_file_system_status = 0; // 0 for not exist  1 for existed

/************************************************************************************
 DEFINE PUBLIC FUNCTIONS
************************************************************************************/
void sys_set_restart_flag(bool sta)
{
  g_esp_restart_flag = (bool)(sta);
}

bool sys_get_restart_flag(void)
{
  return g_esp_restart_flag;
}

uint8_t neurons_engine_get_status_t(void)
{
  return g_neurons_engine_status;
}

void neurons_engine_set_status_t(uint8_t sta)
{
  g_neurons_engine_status = sta;
}

uint32_t codey_get_system_status_t(void)
{
  return g_codey_system_status;
}

void codey_set_system_status_t(uint32_t sta)
{
  g_codey_system_status |= sta;
}

void codey_clear_system_status_t(uint32_t sta)
{
  g_codey_system_status &= (~sta);
}

void codey_clear_all_system_status_t(void)
{
  g_codey_system_status = 0x00000000;
}

void codey_resource_manager_init(void)
{
  g_fatfs_sema = xSemaphoreCreateMutex();;
  xSemaphoreGive(g_fatfs_sema);
}

uint8_t codey_resource_manager_get_file_system_status_t(void)
{
  return g_file_system_status;
}

void codey_resource_manager_set_file_system_status_t(uint8_t now_status)
{
  g_file_system_status = now_status;
}
