/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_ftp_task.c
 * @file    codey_ftp_task.h
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
 * fftust              2017/04/07      1.0.0            build the new.
 * </pre>
 *
 */

#ifndef _CODEY_FTP_TASK_H_
#define _CODEY_FTP_TASK_H_

/******************************************************************************
 DEFINE CONSTANTS
 ******************************************************************************/
#define FTP_TASK_PRIORITY                        (2)
#define FTP_TASK_STACK_SIZE                      (1024) // in bytes
#define FTP_TASK_STACK_LEN                       (FTP_TASK_STACK_SIZE / sizeof(StackType_t))

#define FTP_TASK_SSID_LEN_MAX                    (16)
#define FTP_TASK_KEY_LEN_MAX                     (16)

#define FTP_TASK_USER_PASS_LEN_MAX               (32)

#define FTP_TASK_CYCLE_TIME_MS                   (20)

#define FTP_TASK_DEF_USER                        ("makeblock")
#define FTP_TASK_DEF_PASS                        ("12345678")
#define FTP_TASK_DEF_TIMEOUT_MS                  (300000)        // 5 minutes
#define FTP_TASK_MIN_TIMEOUT_MS                  (5000)         // 5 seconds

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
 extern char codey_ftp_user[];
 extern char codey_ftp_pass[];

/******************************************************************************
 EXPORTED DATA
 ******************************************************************************/
extern StaticTask_t ftpTaskTCB;
extern StackType_t ftpTaskStack[];
extern char ftp_task_user[];
extern char ftp_task_pass[];

/******************************************************************************
 DECLARE PUBLIC FUNCTIONS
 ******************************************************************************/
 extern void codey_ftp_task(void *pvParameters);
 extern void codey_ftp_task_start(void);
 extern void codey_ftp_task_stop(void);
 extern void codey_ftp_task_reset(void);
 extern void codey_ftp_wlan_cycle_power(void);
 extern bool codey_ftp_task_are_enabled(void);
 extern void codey_ftp_task_sleep_sockets(void);

#endif /* _CODEY_FTP_TASK_H_ */


