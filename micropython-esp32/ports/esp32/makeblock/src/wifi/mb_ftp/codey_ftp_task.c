/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock codey_ftp_task module
 * @file    codey_ftp_task.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/04/07
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
 * This file is a drive ftp_task module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/04/07      1.0.0            build the new.
 * </pre>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "mphalport.h"

#include "nvs_flash.h"

#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/nlr.h"
#include "py/objexcept.h"
#include "py/obj.h"

#include "mb_ftp/codey_ftp.h"
#include "mb_ftp/codey_ftp_task.h"
#include "codey_sys_operation.h"
#include "codey_h_class.h"
/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/

/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
typedef struct
{
  uint32_t timeout;
  bool     enabled;
  bool     do_disable;
  bool     do_enable;
  bool     do_reset;
  bool     do_wlan_cycle_power;
} codey_ftp_data_t;

codey_ftp_data_t s_codey_ftp_data;
static volatile bool s_codey_sleep_sockets = false;
 
/******************************************************************************/
void codey_ftp_task(void *pvParameters) 
{  
  while(!codey_task_get_enable_flag_t(CODEY_FTP_TASK_ID))
  {
    vTaskDelay(10);
  }

  codey_ftp_init();
  codey_ftp_task_start();
  
  for(; ;) 
  {
    if(s_codey_ftp_data.do_enable)
    {
      codey_ftp_enable();
      s_codey_ftp_data.enabled = true;
      s_codey_ftp_data.do_enable = false;
    }
    else if(s_codey_ftp_data.do_disable)
    {
      // ftp_disable();
      s_codey_ftp_data.do_disable = false;
      s_codey_ftp_data.enabled = false;
    }
    else if(s_codey_ftp_data.do_reset)
    {
      s_codey_ftp_data.do_reset = false;
      if(s_codey_ftp_data.enabled) 
      {
        // codey_ftp_reset();
      }

    }
    if(codey_task_get_enable_flag_t(CODEY_FTP_TASK_ID))
    {
      codey_ftp_run();
    }
    vTaskDelay(FTP_TASK_CYCLE_TIME_MS / portTICK_PERIOD_MS);
  }
}

void codey_ftp_task_start(void) 
{
  s_codey_ftp_data.do_enable = true;
  vTaskDelay((FTP_TASK_CYCLE_TIME_MS * 3) / portTICK_PERIOD_MS);
}

void codey_ftp_task_stop(void) 
{
  s_codey_ftp_data.do_disable = true;
  do
  {
    vTaskDelay(FTP_TASK_CYCLE_TIME_MS / portTICK_PERIOD_MS);
  }while(codey_ftp_task_are_enabled());
  vTaskDelay((FTP_TASK_CYCLE_TIME_MS * 3) / portTICK_PERIOD_MS);
}

void codey_ftp_task_reset(void) 
{
  s_codey_ftp_data.do_reset = true;
}

void codey_ftp_wlan_cycle_power(void) 
{
  s_codey_ftp_data.do_wlan_cycle_power = true;
}

bool codey_ftp_task_are_enabled(void) 
{
  return s_codey_ftp_data.enabled;
}

void codey_ftp_task_sleep_sockets(void) 
{
  s_codey_sleep_sockets = true;
  vTaskDelay((FTP_TASK_CYCLE_TIME_MS + 1) / portTICK_PERIOD_MS);
}

void codey_ftp_task_set_login(char *user, char *pass)
{
  if(strlen(user) > FTP_TASK_USER_PASS_LEN_MAX || strlen(pass) > FTP_TASK_USER_PASS_LEN_MAX)
  {
    // nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, mpexception_value_invalid_arguments));
  }
  memcpy(codey_ftp_user, user, FTP_TASK_USER_PASS_LEN_MAX);
  memcpy(codey_ftp_pass, pass, FTP_TASK_USER_PASS_LEN_MAX);
}

void codey_ftp_task_set_timeout(uint32_t timeout) 
{
  if(timeout < FTP_TASK_MIN_TIMEOUT_MS)
  {
    // timeout is too low
    // nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, mpexception_value_invalid_arguments));
  }
  s_codey_ftp_data.timeout = timeout;
}

uint32_t codey_ftp_task_get_timeout (void)
{
  return s_codey_ftp_data.timeout;
}


