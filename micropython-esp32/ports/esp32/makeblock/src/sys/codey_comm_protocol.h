/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief      Heard for all common protocol module
 * @file       codey_comm_protocol.h
 * @author     Leo lu
 * @version    V1.0.0
 * @date       2017/09/29
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
 *   Leo lu           2017/09/29      1.0.0              Build the new.
 * </pre>
 *
 */

#ifndef _CODEY_COMM_PROTOCOL_H_
#define _CODEY_COMM_PROTOCOL_H_
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/******************************************************************************
 DEFINE MACRO 
 ******************************************************************************/ 

/******************************************************************************
 DEFINE TYPES 
 ******************************************************************************/  
typedef enum
{
  BLE_DATA_TAG = 0,
  UART0_DATA_TAG,
  UART1_DATA_TAG,
  COMM_CHN_NUM,
} channel_data_tag_t;

typedef enum
{
  NEU_FTP_ID = 0x01,
  SUPER_VAR_ID,
  MSG_BCAST_ID,
  SENSOR_REPORT_ID,
  GET_MAC_ID,
  GET_FIRMWARE,
  DTR_REQ,
  READY_NOTIFY,
  GET_CHN,
  PRODUCT_TEST,
  BAUDRATE_SETTING,
  GET_FILESYS,
  PROTOCOL_NUM,
} protocol_id_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DECLARE PUBLIC FUNCTIONS
 ******************************************************************************/
extern void codey_comm_protocol_pump_char(channel_data_tag_t chn_tag, uint8_t c, uint8_t *output_buf, uint32_t *output_len);
extern void codey_comm_build_frame(uint8_t protocol_id, uint8_t *inoutput_data, uint32_t *inoutput_len);

#endif /* _CODEY_COMM_PROTOCOL_H_ */
