/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   codey system ope file
 * @file    codey_sys_operation.h
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
  
#ifndef _CODEY_SYS_OPERATION_H_
#define _CODEY_SYS_OPERATION_H_

typedef enum
{
  CODEY_LIGHT,
  CODEY_SOUND,
  CODEY_DAIL,
  CODEY_SPEAKER,
  CODEY_BUTTON,
  CODEY_IR_SEND,
  CODEY_IR_REC,
  CODEY_GYRO,
  CODEY_LEDMATRIX,
  CODEY_RGBLED,
  CODEY_NEURONS_ENGINE,
  CODEY_SENSORS_ID_MAX
}codey_sensors_id;

typedef enum
{
  CODEY_SENSOR_INIT = 0,
  CODEY_SENSOR_NOT_INIT = 1,
  CODEY_SENSOR_NOT_EXIT = 2
}codey_sensor_init_sta;

typedef enum
{ 
  CODEY_MP_TASK_ID,
  CODEY_UART_DEAL_TASK_ID,
  CODEY_SENSORS_GET_VALUE_TASK_ID,
  CODEY_FTP_TASK_ID,
  CODEY_LEDMATRIX_SHOW_TASK_ID,
  CODEY_MUSIC_PLAY_TASK_ID,
  
  CODEY_TASK_ID_MAX
}codey_sys_task_id;

typedef enum
{
  CODEY_TASK_NOT_CREATE,
  CODEY_TASK_WAIT_TO_EXECUTE,
  CODEY_TASK_EXECUTING,
  CODEY_TASK_SUSPENDED
}codey_task_status_t;

typedef enum
{
  CODEY_MAIN_PY_NOT_READY = 0,
  CODEY_MAIN_PY_EXECUTED = 1,
  CODEY_MAIN_PY_UPDATE = 2
}codey_main_py_status_t;

typedef void(*task_fun_t)(void *);

typedef struct
{
  codey_task_status_t codey_task_status;
  bool codey_task_execute_enable;
  bool codey_task_auto_break_flag;  

  task_fun_t task_fun; 
  char fun_name[32]; 
  TaskHandle_t task_handle;      
  uint8_t task_priority;
  uint16_t stack_len;
  uint32_t run_time;
}codey_sys_task_manager_t;

extern void codey_task_set_status_t(codey_sys_task_id id, codey_task_status_t sta);
extern codey_task_status_t codey_task_get_status_t(codey_sys_task_id id);
extern void codey_task_set_break_flag_t(codey_sys_task_id id, bool sta);
extern bool codey_task_get_break_flag_t(codey_sys_task_id id);
extern void codey_task_set_enable_flag_t(codey_sys_task_id id, bool sta);
extern bool codey_task_get_enable_flag_t(codey_sys_task_id id);
extern void codey_task_create_t(codey_sys_task_id id);
extern void codey_system_init_t(void);
extern void codey_main_task_hook_t(void);
extern void codey_filesystem_init_hook_t(void);
extern uint8_t codey_get_main_py_exec_status_t(void);
extern void codey_set_main_py_exec_status_t(codey_main_py_status_t sta);
extern void codey_script_update_after_operation(void);
extern void codey_restart_indicate(void);
extern bool codey_mutex_lock(SemaphoreHandle_t mut, int wait);
extern void codey_mutex_unlock(SemaphoreHandle_t mut);
extern bool codey_filesystem_mutex_lock(void);
extern void codey_filesystem_mutex_unlock(void);

#endif /* _CODEY_SYS_OPERATION_H_ */


