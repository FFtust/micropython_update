/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief       Codey ready notify to app
 * @file        codey_ready_notify.c
 * @author      Leo lu
 * @version V1.0.0
 * @date        2017/11/17
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
 * This file include some system function.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * Leo lu             2017/11/17      1.0.0              Build the new.
 * </pre>
 *
 */
 
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "codey_utils.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "codey_comm_protocol.h"
#include "codey_ready_notify.h"
#include "driver/uart.h"
#include "codey_utils.h"
#include "codey_ble_sys_dev_func.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/ 
#undef    TAG
#define   TAG                           ("READY")
#define   UART                          (UART_NUM_0)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/  
enum
{
  READY_CMD = 0x00,
  READY_RSP = 0x80
};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
 
/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_ready(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  if(!data || !len || !output_len || !output_buf)
  {
    return;
  }

  if(1 == len && READY_CMD == data[0])
  {
    output_buf[0] = READY_RSP;
    *output_len = 1;
  }
  else
  {
    *output_len = 0;
  }
}

void codey_ready_notify(void)
{
  uint8_t data[64];
  uint32_t len;

  len = 1;
  data[0] = READY_RSP;
  codey_comm_build_frame(READY_NOTIFY, data, &len);

  uart_write_bytes(UART, (const char *)data, len);
  codey_ble_dev_put_data(data, len);
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/



