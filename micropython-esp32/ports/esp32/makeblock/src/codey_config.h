/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   codey config file
 * @file    codey_config.h
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
  
#ifndef _CODEY_CONFIG_H_
#define _CODEY_CONFIG_H_
/* system control */
#define CODEY_MICROPYTHON_REPL_ENABLE 0 // if set this macro to 1, neurons, BT communication, update script will be disabled

#define CODEY_SOFT_POWER_SWITCH 1
#define CODEY_UNUSUAL_REBOOT_CHECK 0

#define CODEY_BT_COMMUNICATION_ENABLE 1
#define CODEY_UART_COMMUNICATION_ENABLE 1

#define CODEY_LOW_ENERGY_POWER_OFF 1

#define ROCKY_READ_REPORT_MODE 0
/* Version manager */

/* RAM configer */

/* log */
#if 0
#ifdef ESP_LOGE  
  #undef ESP_LOGE
  #define ESP_LOGE( tag, format, ... )  // do nothing
#else
  #define ESP_LOGE( tag, format, ... )  // do nothing
#endif

#ifdef ESP_LOGW 
  #undef ESP_LOGW
  #define ESP_LOGW( tag, format, ... )  // do nothing
#else
  #define ESP_LOGW( tag, format, ... )  // do nothing
#endif

#ifdef ESP_LOGI 
  #undef ESP_LOGI
  #define ESP_LOGI( tag, format, ... )  // do nothing
#else
  #define ESP_LOGI( tag, format, ... )  // do nothing
#endif  

#ifdef ESP_LOGD
  #undef ESP_LOGD
  #define ESP_LOGD( tag, format, ... )  // do nothing
#else
  #define ESP_LOGD( tag, format, ... )  // do nothing  
#endif

#ifdef ESP_LOGV
  #undef ESP_LOGV
  #define ESP_LOGV( tag, format, ... )  // do nothing
#else
  #define ESP_LOGV( tag, format, ... )  // do nothing  
#endif
#endif

#define CODEY_USER_SCRIPT_SHOW_FLAG 1
#define CODEY_PERIPHERAL_LOG_CLOSE 0

/* sensors on board */
#define CODEY_SENSOR_BOARD_LIGHT_ENABLE 1
#define CODEY_SENSOR_BOARD_SOUND_ENABLE 1
#define CODEY_SENSOR_BOARD_DAIL_ENABLE 1
#define CODEY_SENSOR_BOARD_IR_SEND_ENABLE 1 
#define CODEY_SENSOR_BOARD_IR_REC_ENABLE 1 

#define CODEY_SENSOR_GYRO_ZERO_COMPENSATION_ENABLE 1
/* home control test */ 
#define CODEY_SENSOR_BOARD_IR_HOME_CONTROL_TEST 1 
#define CODEY_SENSOR_BOARD_GYRO_ENABLE 1
#define CODEY_SENSOR_BOARD_SPEAKER_ENABLE 1  
#define CODEY_SENSOR_BOARD_LEDMATRIX_ENABLE 1 
/* task priority */

/* mp_task */
#define CODEY_MP_TASK_HEAP_FROM_BSS 0
#define CODEY_MP_TASK_STACK_SIZE 10 * 1024
#define CODEY_MP_TASK_HEAP_SIZE 70 * 1024

#define CODEY_MP_TASK_ENABLE 1
/* mp task and thread priority should be the same */
/* it seems that set to 1 will cause many problems */
#define CODEY_MP_TASK_PRIORITY 1
/* config python thread piority by set MP_THREAD_PRIORITY to */
/* CODEY_MP_THREAD_PRIORITY in mpthreadport.c */
#define CODEY_MP_THREAD_PRIORITY 1

#define CODEY_SOFT_RESTART_AFTER_SCRIPT_UPDATE 1
/* system tasks config*/ 
#define CODEY_UART_DEAL_TASK_ENABLE 1
#define CODEY_UART_DEAL_TASK_PRIORITY 4
#define CODEY_UART_DEAL_TASK_STACK_SIZE 4 * 1024

#define CODEY_SENSORS_GET_VALUE_TASK_ENABLE 1
#define CODEY_SENSORS_GET_VALUE_TASK_PRIORITY 2
#define CODEY_SENSORS_GET_VALUE_TASK_STACK_SIZE 3 * 1024

#define CODEY_FTP_TASK_ENABLE 0
#define CODEY_FTP_TASK_PRIORITY 3
#define CODEY_FTP_TASK_STACK_SIZE 3 * 1024
 
#define CODEY_LEDMATRIX_SHOW_TASK_ENABLE 1
#define CODEY_LEDMATRIX_SHOW_TASK_PRIORITY 1
#define CODEY_LEDMATRIX_SHOW_TASK_STACK_SIZE 3 * 1024

#define CODEY_MUSIC_PLAY_TASK_ENABLE 1
#define CODEY_MUSIC_PLAY_TASK_PRIORITY 1
#define CODEY_MUSIC_PLAY_TASK_STACK_SIZE 3 * 1024

/* test function, will be delete*/
#define CODEY_FILESYSTEM_TEST 0
#endif /* _CODEY_CONFIG_H_ */

