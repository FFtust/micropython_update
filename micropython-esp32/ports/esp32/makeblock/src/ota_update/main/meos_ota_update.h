/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for meos_ota_update.c.
 * @file    meos_ota_update.c.
 * @author  fftust
 * @version V1.0.0
 * @date    2017/08/04
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
 * This file is a drive  meos_ota module
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/08/04      1.0.0              build the new.
 * </pre>
 *
 */


#ifndef _MEOS_OTA_UPDATE_H
#define _MEOS_OTA_UPDATE_H

typedef enum
{
  OTA_0_UPDATE_APPLICATION = 0,
  OTA_1_APPLICATION = 1,
  OTA_UPDATE_MAX,
}ota_update_region_type_t;


typedef struct
{
  uint8_t magic;
  ota_update_region_type_t update_region_type;
  char    ota_current_version[8];
  char    ota_update_version[8];
  uint8_t wifi_ssid_len;
  char    wifi_ssid[32];
  uint8_t wifi_pass_len;
  char    wifi_pass[32];
  char    http_server_url[128];
  char    http_server_ip[32];
  char    http_port[8];
  char    file_name[32];
  
}ota_update_info_t;


void      meos_ota_update_set_default_partition_info(ota_update_info_t *ota_info);
esp_err_t meos_ota_update_write_partition_info(ota_update_info_t *update_info);
esp_err_t meos_ota_update_read_partition_info(ota_update_info_t *update_info);




#endif
