/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     Heard for ff 55 protocol
 * @file      codey_ff_55_com_protocol.h
 * @author    Leo
 * @version V1.0.0
 * @date      2017/11/15
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
 *   Leo              2017/11/15      1.0.0            build the new.
 * </pre>
 *
 */

#ifndef _CODEY_FF_55_COMM_PROTOCOL_H_
#define _CODEY_FF_55_COMM_PROTOCOL_H_
  
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "codey_comm_protocol.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES 
 ******************************************************************************/ 
enum
{
  FW_DATA_TYPE_BYTE     = 0x01,
  FW_DATA_TYPE_FLOAT    = 0x02,
  FW_DATA_TYPE_SHORT    = 0x03,
  FW_DATA_TYPE_STR      = 0x04,
  FW_DATA_TYPE_DOUBLE   = 0x05,
  FW_DATA_TYPE_LONG     = 0x06,
};

/******************************************************************************
 DECLARE OF PUBLIC FUNCTIONS
 ******************************************************************************/
extern void codey_ff_55_protocol_pump_char(channel_data_tag_t chn_tag, uint8_t c, uint8_t *output_buf, uint32_t *output_len);

#endif /* _CODEY_FF_55_COMM_PROTOCOL_H_ */


