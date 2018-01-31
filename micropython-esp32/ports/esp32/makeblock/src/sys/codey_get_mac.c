/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief       Get mac address
 * @file        codey_get_mac.c
 * @author      Leo lu
 * @version V1.0.0
 * @date        2017/10/17
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
 * Leo lu             2017/10/17      1.0.0              Build the new.
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
#include "codey_wlan.h"
#include "codey_get_mac.h"

/******************************************************************************
 DEFINE MACROS 
 ******************************************************************************/ 
#undef    TAG
#define   TAG                           ("GET_MAC")

#define   GET_MAC_CMD_LEN               (1)
#define   WIFI_MAC_ID                   (0x01)
#define   BLE_MAC_ID                    (0x02)
#define   MAC_ADDR_LEN                  (0x06)
                                            
/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/  

/******************************************************************************
 DECLARE PRIVATE DATAS
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
static bool codey_get_mac_check_len(uint32_t len);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
/*
This must been define
#define CONFIG_FOUR_MAC_ADDRESS_FROM_EFUSE 1
#define CONFIG_NUMBER_OF_MAC_ADDRESS_GENERATED_FROM_EFUSE 4

esp_efuse_mac_get_default() return the base mac, which is the station_wifi_mac
eth_mac = ap_wifi_mac = sta_wifi_mac + 1 
ble_mac = sta_wifi_mac + 2
*/
void codey_get_mac(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint8_t mac_addr_id;

  if(!data || !len || !output_len || !output_buf)
  {
    return;
  }

  if(!codey_get_mac_check_len(len))
  {
    return;
  }

  mac_addr_id = data[0];
  output_buf[0] = mac_addr_id;
  *output_len = 1;
  if(ESP_OK == esp_efuse_mac_get_default(output_buf+1))
  {
    if(WIFI_MAC_ID == mac_addr_id)
    {
      // The base mac + 0
      output_buf[ 1 + MAC_ADDR_LEN - 1 ] += 0;
    }
    else if(BLE_MAC_ID == mac_addr_id)
    {
      // The base mac + 2
      output_buf[ 1 + MAC_ADDR_LEN - 1 ] += 2;
    }
    else
    {
      // All 0
      memset(output_buf + 1, 0, MAC_ADDR_LEN);
    }
    *output_len += MAC_ADDR_LEN;
  }
  else
  {
    // All 0
    memset(output_buf + 1, 0, MAC_ADDR_LEN);
    *output_len += MAC_ADDR_LEN;
  }
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
static bool codey_get_mac_check_len(uint32_t len)
{
  return (GET_MAC_CMD_LEN == len?true:false);
}
