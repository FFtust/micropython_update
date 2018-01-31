/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     Heard for codey_neurons_ftp.c
 * @file      codey_neurons_ftp.h
 * @author    Leo lu
 * @version    V1.0.0
 * @date      2017/06/28
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
 * This file setup ble to device and start ble device with a makeblock profile.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * Leo lu             2017/06/28         1.0.0              Initial version
 * </pre>
 *
 */

#ifndef _CODEY_NEURONS_FTP_H_
#define _CODEY_NEURONS_FTP_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_heap_caps.h"
#include "codey_utils.h"
#include "codey_comm_protocol.h"

/******************************************************************************
 DEFIEN MACROS
 ******************************************************************************/
#define MAX_NEU_FTP_FILE_NAME_LEN         (64)

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
extern void codey_neurons_ftp_init(void);
extern uint8_t codey_neurons_ftp_pump_char(uint8_t c, uint8_t **out_buf, uint32_t *out_len);
extern void codey_neurons_ftp(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
extern void codey_neurons_get_recv_file_name(char *output_buf);

#endif /* _CODEY_NEURONS_FTP_H_ */

