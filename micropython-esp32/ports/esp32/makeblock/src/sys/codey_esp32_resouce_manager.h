/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_esp32_resouce_manager.c
 * @file    codey_esp32_resouce_manager.h
 * @author  fftust
 * @version V1.0.0
 * @date    2017/066/28
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
 * This file is a drive  button_board module.
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

#ifndef _CODEY_ESP32_RESOUCE_MANAGER_H_
#define _CODEY_ESP32_RESOUCE_MANAGER_H_

/* for system status manager */
#define SYS_STA_NORMAL               (0x00000000)
#define SYS_STA_UPDATE_USER_SCRIPT   (0x000000001)
#define SYS_STA_UPDATE_MUSIC_LIB     (0x00000002)
#define SYS_STA_UPDATE_NEURONS_LIB   (0x00000004)
#define SYS_STA_UPDATE_ANIMATION_LIB (0x00000008)
/* add more if needed */
extern TaskHandle_t g_mp_task_handle;
extern SemaphoreHandle_t g_fatfs_sema;

extern void codey_resource_manager_init(void);
extern uint8_t codey_resource_manager_get_file_system_status_t(void);
extern void codey_resource_manager_set_file_system_status_t(uint8_t now_status);

extern uint32_t codey_get_system_status_t(void);
extern void codey_set_system_status_t(uint32_t sta);
extern void codey_clear_system_status_t(uint32_t sta);
extern void codey_clear_all_system_status_t(void);

extern uint8_t neurons_engine_get_status_t(void);
extern void neurons_engine_set_status_t(uint8_t sta);

extern void sys_set_restart_flag(bool sta);
extern bool sys_get_restart_flag(void);

#endif /* _CODEY_ESP32_RESOUCE_MANAGER_H_ */
