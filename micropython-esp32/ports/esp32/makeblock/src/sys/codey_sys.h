/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for all makeblock codey_sys.c.
 * @file    codey_sys.h
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
 * This file is a header for system function.
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
  
#ifndef _CODEY_SYS_H_
#define _CODEY_SYS_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "ff.h"
#include "diskio.h"
#include "ffconf.h"
#include "mb_fatfs/pybflash.h"
#include "mb_fatfs/drivers/sflash_diskio.h"
#include "extmod/vfs_fat.h"

#define SCRIPT_ERASE_COUNT_MAX (5)

extern fs_user_mount_t *codey_sflash_vfs_fat;

extern uint32_t micros(void);
extern uint32_t millis(void);
/*this function will interrupt the main.py*/
extern void user_script_interrupt(void); 
extern uint8_t get_user_script_interrupt_reason(void);
extern void set_user_script_interrupt_reason(uint8_t value);
extern uint8_t get_factory_script_exec_index(void);
extern void set_factory_script_exec_index(uint8_t value);
extern void set_factory_script_exec_index_next(void);
extern uint8_t get_user_script_exec_flag(void);
extern void set_user_script_exec_flag(uint8_t value);

extern void sys_create_erase_script_count_file_t(bool);
extern void sys_clear_erase_script_count_t(bool);
extern uint8_t sys_clear_erase_script_count_get_t(bool);
extern void sys_clear_erase_script_count_add_t(bool);
extern uint8_t sys_music_file_check_t(bool wait_sema);
extern void esp32_heap_info_show_t();

/* comfiger to define different version board */
#define CODEY_V107 1

#endif /* _CODEY_SYS_H_ */
