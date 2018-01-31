/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief       Get communication channel
 * @file        codey_get_chn.c
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
#include "codey_ff_55_comm_protocol.h"
#include "codey_get_chn.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/ 
#undef    TAG
#define   TAG                           ("GET_CHN")
#define   EXPECT_MODE                   (0x01)                                            

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/  

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
const static char * s_channel_str[COMM_CHN_NUM] = 
{
  "BLE",
  "UART0"
};

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_get_chn(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  const char *chn_str;
  size_t chn_str_len;
  uint8_t index;
  uint8_t mode;

  if(!data || !len || !output_len || !output_buf)
  {
    return;
  }

  index = data[0];
  mode = data[1];

  if(EXPECT_MODE != mode)
  {
    *output_len = 0;
    return;
  }
  
  chn_str = codey_get_chn_tag(chn);
  if(!chn_str)
  {
    *output_len = 0;
    return;
  }
  chn_str_len = strlen(chn_str);

  output_buf[0] = index;
  output_buf[1] = FW_DATA_TYPE_STR;
  output_buf[2] = chn_str_len;
  memcpy(output_buf + 3, chn_str, chn_str_len);
  *output_len = chn_str_len + 3;
}

const char *codey_get_chn_tag(channel_data_tag_t chn)
{
  return s_channel_str[chn];
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/



