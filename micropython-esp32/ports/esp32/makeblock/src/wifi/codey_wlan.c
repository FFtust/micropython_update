/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock wlan module
 * @file    codey_wlan.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/05/24
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
 * This file is a drive wlan module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/05/24      1.0.0              build the new.
 * leo                2017/10/23      1.0.1              Add interface of getting stattion connected status
 * </pre>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_task.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "mphalport.h"
#include "esp_interface.h"
#include "nvs_flash.h"

#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/nlr.h"
#include "py/objexcept.h"
#include "py/obj.h"

#include "tcpip_adapter.h"
#include "codey_wlan.h"
/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG                         ("codey_wlan")
#define WIFI_STA_RECONNECT_TIMES    (100) 
#define WIFI_STA_RECONNECT_INTERVAL (500)  // ms

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
const char *codey_wifi_ap_ssid_default = "Makeblock_ESP32";
const char *codey_wifi_ap_password_default = "12345678";
const char *codey_wifi_sta_ssid_default = "Maker-office";
const char *codey_wifi_sta_password_default = "hulurobot423";
const int  CODEY_CONNECTED_BIT = BIT0;

typedef struct
{
  mp_obj_base_t base;
  volatile bool wifi_enabled_flag;
  volatile bool wifi_ap_connected_flag;  
  volatile bool wifi_sta_connect_flag;
  volatile bool wifi_sta_reconfig_flag;  
  volatile bool wifi_ap_ssid_add_mac_flag;
  wifi_mode_t   wifi_cur_mode;
  wifi_config_t wifi_ap_cur_config;
  wifi_config_t wifi_sta_cur_config;
  uint8_t       wifi_ap_mac[6];
  uint8_t       wifi_sta_mac[6];
}codey_wifi_info_t;

typedef struct
{
  char ssid[32];
  char pass[32]; 
} codey_wlan_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATAS
 ******************************************************************************/
/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t codey_wifi_event_group;

wifi_config_t wifi_cfg_ap;  // true wifi config struct, part of this union get from user configer data
wifi_config_t wifi_cfg_sta; // true wifi config struct, part of this union get from user configer data

static codey_wlan_obj_t   codey_wifi_ap_cfgpara  = {}; //user configer data
static codey_wlan_obj_t   codey_wifi_sta_cfgpara = {}; //user configer data

static codey_wifi_info_t  codey_wifi_info = 
{
  .wifi_enabled_flag = false ,
  .wifi_ap_connected_flag = false ,
  .wifi_sta_connect_flag = false,
  .wifi_ap_ssid_add_mac_flag = false,
  .wifi_sta_reconfig_flag = false,
  .wifi_cur_mode = WIFI_MODE_NULL
};
static uint16_t wifi_sta_reconnected_number = 0;

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC esp_err_t codey_wifi_event_handler(void *ctx, system_event_t *event);
STATIC void      codey_wifi_ap_config(void);
STATIC void      codey_wifi_sta_config(void);
STATIC void      codey_wifi_start_apmode(void);
STATIC void      codey_wifi_start_stamode(void);
STATIC void      codey_wifi_start_apstamode(void);
STATIC uint8_t*  codey_wifi_get_mac(wifi_interface_t ifx);
STATIC void      codey_wifi_scan(uint16_t ap_num, wifi_ap_record_t *ap_record);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_wifi_pre_init(void)
{
  codey_wifi_event_group = xEventGroupCreate(); 
  strcpy(codey_wifi_ap_cfgpara.ssid, codey_wifi_ap_ssid_default);
  strcpy(codey_wifi_ap_cfgpara.pass, codey_wifi_ap_password_default);
  strcpy(codey_wifi_sta_cfgpara.ssid, codey_wifi_sta_ssid_default);
  strcpy(codey_wifi_sta_cfgpara.pass, codey_wifi_sta_password_default);
}

void codey_wifi_mac_to_string_t(uint8_t ifx, uint8_t *macstr)
{
  uint8_t *mac;
  
  mac = codey_wifi_get_mac(ifx);
  if(mac != NULL)
  {
    for(uint8_t i = 0; i < 6; i++)
    {
      macstr[2 * i] = (*mac) >> 4;
      macstr[2 * i + 1] = (*mac) & 0x0f;
      mac++;
    }

    for(uint8_t i = 0; i < 12; i++)
    {
      if(macstr[i] < 10)
      {
        macstr[i] += 0x30; // '0' ascii 0x30
      }
      else    
      {
        macstr[i] = macstr[i] - 10 + 0x41; // 'A' ascii 0x41
      }
    }

    macstr[12] = '\0';  
  }
}

void codey_wifi_ap_ssid_add_mac(void)
{
  uint8_t macstr[13];
  if(codey_wifi_info.wifi_ap_ssid_add_mac_flag == true)
  {
    codey_wifi_mac_to_string_t(ESP_IF_WIFI_AP, macstr);
    strcat(codey_wifi_ap_cfgpara.ssid, (const char *)macstr);
  }
}

void codey_wifi_enable(void)
{
  esp_err_t ret = ESP_FAIL;
  if(codey_wifi_info.wifi_enabled_flag)
  {
    ESP_LOGI(TAG, "en wifi had already enabled and init");
  }
  else
  {
    codey_wifi_pre_init();
    tcpip_adapter_init();
    ret = esp_event_loop_init(codey_wifi_event_handler, NULL);
    if(ret != ESP_OK)
    {
      ESP_LOGE(TAG, "en loop init error: %d", ret);
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if(ret != ESP_OK)
    {
      ESP_LOGE(TAG, "en wifi init error: %d", ret);
    }

    ret = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if(ret != ESP_OK)
    {
      ESP_LOGE(TAG, "en set storage error: %d", ret);
    }    
    
    ret = esp_wifi_set_mode(WIFI_MODE_NULL); 
    if(ret != ESP_OK)
    {
      ESP_LOGE(TAG, "en set mode error: %d", ret);
    }
    
    ret = esp_wifi_start();
    if(ret != ESP_OK)
    {
      ESP_LOGE(TAG, "en wifi start error: %d", ret);
    }

    codey_wifi_info.wifi_enabled_flag = true;
  }
}

void codey_wifi_disenable(void)
{
  esp_err_t ret = ESP_FAIL;
  wifi_mode_t mode;
  if(esp_wifi_get_mode(&mode) == ESP_OK) 
  {
    if(mode & WIFI_MODE_STA)
    {
      esp_wifi_disconnect();
    }
  }
  ret = esp_wifi_stop();
  if(ret != ESP_OK)
  {
    ESP_LOGE(TAG, "dis wifi stop error: %d", ret);
  }

  ret = esp_wifi_set_mode(WIFI_MODE_NULL);
  if(ret != ESP_OK)
  {
    ESP_LOGE(TAG, "dis wifi stop error: %d", ret);
  }
  
  ret = esp_wifi_deinit();
  if(ret != ESP_OK)
  {
    ESP_LOGE(TAG, "dis wifi deinit error: %d", ret);
  }

  vEventGroupDelete(codey_wifi_event_group);
  codey_wifi_event_group = NULL;

  codey_wifi_info.wifi_enabled_flag = false;
  codey_wifi_info.wifi_ap_connected_flag = false;
  codey_wifi_info.wifi_sta_connect_flag = false;
  codey_wifi_info.wifi_cur_mode = WIFI_MODE_NULL;
}

void codey_wifi_restart(void)
{
  codey_wifi_disenable();
  codey_wifi_enable();
}

bool codey_wifi_get_ap_status(void)
{
  return codey_wifi_info.wifi_ap_connected_flag;  
}

bool codey_wifi_get_sta_status(void)
{
  return codey_wifi_info.wifi_sta_connect_flag;
}

bool codey_wifi_sta_is_connected()
{
  if(xEventGroupGetBits(codey_wifi_event_group) & CODEY_CONNECTED_BIT) 
  { 
    return true;
  }
  else
  {
    return false;
  }
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC esp_err_t codey_wifi_event_handler(void *ctx, system_event_t *event)
{
  ESP_LOGD(TAG, "***event*** %d", event->event_id);
  switch(event->event_id)
  {
    case SYSTEM_EVENT_STA_START:
      ESP_LOGD(TAG, "wifi sta: start to connect");
    break;
    case SYSTEM_EVENT_STA_STOP:
      ESP_LOGD(TAG, "wifi sta: stop");
    break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ESP_LOGD(TAG, "wifi sta: got ap");
      xEventGroupSetBits(codey_wifi_event_group, CODEY_CONNECTED_BIT);
    break;
    case SYSTEM_EVENT_STA_CONNECTED:
      ESP_LOGD(TAG, "wifi sta: sta connect to ap");
    break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      xEventGroupClearBits(codey_wifi_event_group, CODEY_CONNECTED_BIT);
      {
        system_event_sta_disconnected_t *disconn = &event->event_info.disconnected;
        ESP_LOGD(TAG, "STA_DISCONNECTED, reason:%d", disconn->reason);
        switch(disconn->reason)
        {
          case WIFI_REASON_AUTH_FAIL:
            ESP_LOGD(TAG, "authentication failed");
          break;
          case WIFI_REASON_NO_AP_FOUND:
            ESP_LOGD(TAG, "ap  not found ");
          break;
          default:
            // Let other errors through and try to reconnect.
          break;
        }
        if(codey_wifi_info.wifi_sta_connect_flag)
        {
          ESP_LOGD(TAG, "wifi sta reconnect");
          wifi_mode_t mode;
          if(esp_wifi_get_mode(&mode) == ESP_OK) 
          {
            if(mode & WIFI_MODE_STA)
            {
              // STA is active so attempt to reconnect.
              esp_err_t e = esp_wifi_connect();
              wifi_sta_reconnected_number++;
              if(e != ESP_OK)
              { 
                ESP_LOGD(TAG, "error attempting to reconnect: 0x%04x", e);
              }
            }
          } 
        }
      }
    break;
    case SYSTEM_EVENT_AP_START:
      ESP_LOGD(TAG, "wifi ap: ap start");
    break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      codey_wifi_info.wifi_ap_connected_flag = true;
      ESP_LOGD(TAG, "wifi ap: a sta connect this ap");
    break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      codey_wifi_info.wifi_ap_connected_flag = false;
      ESP_LOGD(TAG, "wifi ap: a sta disconnect this ap");
    break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
      ESP_LOGD(TAG, "wifi ap: Receive probe request packet");
    break;
    default:
      ESP_LOGD(TAG, "default event %d", event->event_id);
    break;
  }
  return ESP_OK;
}

STATIC void codey_wifi_ap_config(void)
{
  strcpy((char *)wifi_cfg_ap.ap.ssid, codey_wifi_ap_cfgpara.ssid);
  strcpy((char *)wifi_cfg_ap.ap.password, codey_wifi_ap_cfgpara.pass);
  wifi_cfg_ap.ap.ssid_len = strlen(codey_wifi_ap_cfgpara.ssid); 
  wifi_cfg_ap.ap.authmode = WIFI_AUTH_WPA2_PSK;
  wifi_cfg_ap.ap.channel = 6;
  wifi_cfg_ap.ap.max_connection = 2;
  wifi_cfg_ap.ap.beacon_interval = 100;
}

STATIC void codey_wifi_sta_config(void)
{
  if(strcmp((char *)wifi_cfg_sta.sta.ssid, codey_wifi_sta_cfgpara.ssid) || strcmp((char *)wifi_cfg_sta.sta.password, codey_wifi_sta_cfgpara.pass)
     || !codey_wifi_sta_is_connected())
  {
    strcpy((char *)wifi_cfg_sta.sta.ssid, codey_wifi_sta_cfgpara.ssid);
    strcpy((char *)wifi_cfg_sta.sta.password, codey_wifi_sta_cfgpara.pass);
    codey_wifi_info.wifi_sta_reconfig_flag = true;
    ESP_LOGI(TAG, "wifi sta config change");
  }
  else
  {
    ESP_LOGI(TAG, "wifi sta config not change");
    codey_wifi_info.wifi_sta_reconfig_flag = false;
  }
}

STATIC void codey_wifi_start_apmode(void)
{ 
  esp_err_t ret = ESP_FAIL;
  ret = esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_cfg_ap);
  if(ret != ESP_OK)
  {
    ESP_LOGE(TAG, "sa wifi set config error: %d", ret);
  }
}

STATIC void codey_wifi_start_stamode(void)
{
  esp_err_t ret = ESP_FAIL;
  wifi_sta_reconnected_number = 0;
  if(codey_wifi_info.wifi_sta_reconfig_flag == true)
  {
    if(codey_wifi_sta_is_connected()) 
    {
      codey_wifi_info.wifi_sta_connect_flag = false;
      ret = esp_wifi_disconnect();
      if(ret != ESP_OK)
      {
        ESP_LOGE(TAG, "ss wifi disconnect error: %d", ret);
      }
    }
    ret = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg_sta);
    if(ret != ESP_OK)
    {
      ESP_LOGE(TAG, "sa wifi set config error: %d", ret);
    }
    
    MP_THREAD_GIL_EXIT();
    ret = esp_wifi_connect();
    MP_THREAD_GIL_ENTER();
    if(ret != ESP_OK)
    {
      ESP_LOGE(TAG, "sa wifi connect error: %d", ret);
    }
    codey_wifi_info.wifi_sta_connect_flag = true;
  }
}

STATIC void codey_wifi_start_apstamode(void)
{
  codey_wifi_start_apmode();
  codey_wifi_start_stamode();
}

STATIC uint8_t* codey_wifi_get_mac(wifi_interface_t ifx)
{
  if(ifx == ESP_IF_WIFI_STA)
  {
    if(esp_wifi_get_mac(ifx, codey_wifi_info.wifi_sta_mac) == ESP_OK)
    {
      return codey_wifi_info.wifi_sta_mac;
    }  
    else
    {
      return NULL;
    }
  }
  else if(ifx == ESP_IF_WIFI_AP)
  {
    if(esp_wifi_get_mac(ifx, codey_wifi_info.wifi_ap_mac) == ESP_OK)
    {
      return codey_wifi_info.wifi_ap_mac;
    }
    else
    {
      return NULL;
    }
  }
  else
  {
    //ESP_LOGI(TAG, "makeblock:para wrong\n");
  }
  
  return NULL;
}

STATIC wifi_config_t* codey_wifi_get_config(wifi_interface_t ifx)
{
  if(ifx == ESP_IF_WIFI_STA)
  {
    if(esp_wifi_get_config(ifx, &codey_wifi_info.wifi_sta_cur_config) == ESP_OK)
    {
     return &codey_wifi_info.wifi_sta_cur_config;
    }   
    else
    {
      return NULL;
    }
  }
  else if(ifx == ESP_IF_WIFI_AP)
  {
    if(esp_wifi_get_config(ifx, &codey_wifi_info.wifi_ap_cur_config) == ESP_OK)
    {
      return &codey_wifi_info.wifi_ap_cur_config;
    }
    else
    {
      return NULL;
    }
  }
  else
  {

  }
  
  return NULL;
}

STATIC  void codey_wifi_scan(uint16_t ap_num, wifi_ap_record_t *ap_record) 
{
  if(codey_wifi_info.wifi_cur_mode == WIFI_MODE_AP)
  {
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "the requested operation is not possible"));
  }

  esp_wifi_scan_start(NULL, true);

  if(ap_record == NULL)
  {
    return;
  }
  
  if(esp_wifi_scan_get_ap_records(&ap_num, (wifi_ap_record_t *)ap_record) == ESP_OK) 
  {
    // ap_record = ap_record_buffer;
  }

  esp_wifi_scan_stop();
}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
STATIC mp_obj_t codey_wlan_enable(mp_obj_t self_in)
{
  codey_wifi_enable();
  
  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    return mp_obj_new_bool(true); 
  }
  else
  {
    return mp_obj_new_bool(false);
  }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_wlan_enable_obj,codey_wlan_enable);

STATIC mp_obj_t codey_wlan_ap_ssid_add_mac_set(mp_obj_t self_in, mp_obj_t en)
{
  bool enabled = false;
  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    enabled = (bool)(mp_obj_get_int(en));
    codey_wifi_info.wifi_ap_ssid_add_mac_flag = enabled;

    return mp_obj_new_bool(true);
  }

  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_wlan_ap_ssid_add_mac_set_obj,codey_wlan_ap_ssid_add_mac_set);

STATIC mp_obj_t codey_wlan_set_mode(mp_obj_t self_in, mp_obj_t mode)
{
  wifi_mode_t wifimode;
  
  wifimode = mp_obj_get_int(mode);
  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    esp_err_t res = ESP_FAIL;
    if(wifimode < WIFI_MODE_MAX)
    {
      switch(wifimode)
      {
        case WIFI_MODE_NULL:    // 0
          res = esp_wifi_set_mode(WIFI_MODE_NULL);
          break;
        case WIFI_MODE_STA:     // 1
          res = esp_wifi_set_mode(WIFI_MODE_STA);
          break;
        case WIFI_MODE_AP:      // 2
          res = esp_wifi_set_mode(WIFI_MODE_AP);
          break;
        case WIFI_MODE_APSTA:   // 3
          res = esp_wifi_set_mode(WIFI_MODE_APSTA);
          break;
        default:
          break;
      }
    }
  
    if(res == ESP_OK)
    {   
      codey_wifi_info.wifi_cur_mode = wifimode;
      return mp_obj_new_bool(true);
    }
    else
    {
      ESP_LOGE(TAG, "wifi set mode failed");
      return mp_obj_new_bool(false);
    }
  }
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_wlan_set_mode_obj,codey_wlan_set_mode);

STATIC mp_obj_t codey_wlan_get_mode(mp_obj_t self_in)
{
  wifi_mode_t wifimode;
  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    if(esp_wifi_get_mode(&wifimode) == ESP_OK)
    {
      codey_wifi_info.wifi_cur_mode = wifimode;
      return mp_obj_new_int(codey_wifi_info.wifi_cur_mode);   
    }
    else
    {
      return mp_obj_new_int(WIFI_MODE_ERR);
    }
  }
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_wlan_get_mode_obj,codey_wlan_get_mode);

STATIC mp_obj_t codey_wlan_set_ap(mp_uint_t n_args, const mp_obj_t *args)
{
  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    if(n_args == 1) 
    {
      return mp_obj_new_str((const char *)codey_wifi_ap_cfgpara.ssid, strlen((const char *)codey_wifi_ap_cfgpara.ssid), false);
    } 
    else 
    {
      if(n_args >= 2)
      {
        size_t len;
        const char *ssid = mp_obj_str_get_data(args[1], &len);
        if(len > 32)
        {
          return mp_obj_new_bool(false);
        }
        else
        {
          strcpy((char *)codey_wifi_ap_cfgpara.ssid, ssid);
          codey_wifi_ap_ssid_add_mac();
        } 
      }

      if(n_args == 3)
      {
        size_t len;
        const char *pword = mp_obj_str_get_data(args[2], &len);
        if(len > 32)
        {
          return mp_obj_new_bool(false);
        }
        else
        {
          strcpy((char *)codey_wifi_ap_cfgpara.pass, pword);
        } 
      }
    }

    return mp_obj_new_bool(true);
  }
  
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(codey_wlan_set_ap_obj, 1, 3, codey_wlan_set_ap);

STATIC mp_obj_t codey_wlan_set_sta(mp_uint_t n_args, const mp_obj_t *args)
{
  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    if(n_args == 1) 
    {
      return mp_obj_new_str((const char *)codey_wifi_sta_cfgpara.ssid, strlen((const char *)codey_wifi_sta_cfgpara.ssid), false);
    } 
    else 
    {
      if(n_args >= 2)
      {
        size_t len;
        const char *ssid = mp_obj_str_get_data(args[1], &len);
        if(len > 32)
        {
          return mp_obj_new_bool(false);
        }
        else
        {
          strcpy((char *)codey_wifi_sta_cfgpara.ssid, ssid);
        } 
      }

      if(n_args == 3)    
      {
        size_t len;
        const char *pword = mp_obj_str_get_data(args[2], &len);
        if(len > 32)
        {
          return mp_obj_new_bool(false);
        }
        else
        {
          strcpy((char *)codey_wifi_sta_cfgpara.pass, pword);
        } 
      }
    }

    return mp_obj_new_bool(true);
  }
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(codey_wlan_set_sta_obj, 1, 3, codey_wlan_set_sta);

STATIC mp_obj_t codey_wlan_scan(mp_obj_t self_in) 
{
  uint16_t apnum = 0;
  wifi_ap_record_t *ap_rec_buffer = NULL;
  wifi_ap_record_t *ap_rec = NULL;
  esp_wifi_scan_start(NULL, true);
  esp_wifi_scan_get_ap_num(&apnum);
  esp_wifi_scan_stop(); 
  ap_rec_buffer = pvPortMalloc(apnum * sizeof(wifi_ap_record_t));
  if(ap_rec_buffer == NULL)
  {
    return mp_const_none;
  }
  else
  {
    codey_wifi_scan(apnum, ap_rec_buffer);
    ap_rec = ap_rec_buffer;
    for(int i = 0; i < apnum; i++)
    {
      ESP_LOGI(TAG, "%s\n", (char *)ap_rec->ssid);
      ap_rec++;
    }
    vPortFree(ap_rec_buffer);
  }
  
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_wlan_scan_obj, codey_wlan_scan);

STATIC mp_obj_t codey_wlan_set_auto_connect(mp_obj_t self_in, mp_obj_t en)
{
  uint8_t autocon = 0;

  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    autocon = mp_obj_get_int(en);
    if(esp_wifi_set_auto_connect((bool)autocon) == ESP_OK)
    {
      if(autocon == 0)
      {
        // codey_wifi_info.wifi_sta_auto_connect_flag = false;
      }
      else
      {
        // codey_wifi_info.wifi_sta_auto_connect_flag = true;
      }

      return mp_obj_new_bool(true);
    }
    else
    {
      return mp_obj_new_bool(false);
    }
  }
  
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_wlan_set_auto_connect_obj, codey_wlan_set_auto_connect);

STATIC mp_obj_t codey_wlan_connect(mp_obj_t self_in) 
{
  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    MP_THREAD_GIL_EXIT();
    if(esp_wifi_connect() == ESP_OK)
    { 
      MP_THREAD_GIL_ENTER();
      codey_wifi_info.wifi_sta_connect_flag = true;
      ESP_LOGI(TAG, "connected succeed");
      return mp_obj_new_bool(true);
    }
    else
    {
      MP_THREAD_GIL_ENTER();
      codey_wifi_info.wifi_sta_connect_flag = true;
      ESP_LOGI(TAG, "connected failed");
      return mp_obj_new_bool(false);
    }
   
  }
  
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_wlan_connect_obj, codey_wlan_connect);

STATIC mp_obj_t codey_wlan_disconnect(mp_obj_t self_in) 
{
  if(codey_wifi_info.wifi_enabled_flag == true)
  {
     codey_wifi_info.wifi_sta_connect_flag = false;
     if(esp_wifi_disconnect() == ESP_OK)
     {
       return mp_obj_new_bool(true);
     }
     else
     {
       return mp_obj_new_bool(false);
     }
  }
  
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_wlan_disconnect_obj, codey_wlan_disconnect);

STATIC mp_obj_t codey_wlan_start(mp_obj_t self_in, mp_obj_t type)
{
  uint8_t starttype;

  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    starttype = mp_obj_get_int(type);
    switch(starttype)
    {
      case 0:
      break;
      case 1:
        codey_wifi_sta_config();
        codey_wifi_start_stamode();
      break;
      case 2:
        codey_wifi_ap_config();
        codey_wifi_start_apmode();
      break;
      case 3:
        codey_wifi_sta_config();
        codey_wifi_ap_config();
        codey_wifi_start_apstamode();
      break;    
      default:
      break;
    }
    return mp_obj_new_bool(true);//if errs happend,it will reboot
  }
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_wlan_start_obj,codey_wlan_start);

STATIC mp_obj_t codey_wlan_stop(mp_obj_t self_in)
{
  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    if(esp_wifi_stop() == ESP_OK)
    {
      return mp_obj_new_bool(true);
    }
    else
    {
      return mp_obj_new_bool(false);
    }
  }
  return mp_const_none;
  
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_wlan_stop_obj, codey_wlan_stop);

STATIC mp_obj_t codey_wlan_deinit(mp_obj_t self_in)
{
  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    codey_wifi_disenable();

    return mp_obj_new_bool(true);
  }

  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_wlan_deinit_obj, codey_wlan_deinit);

STATIC mp_obj_t codey_wlan_get_config(mp_obj_t self_in, mp_obj_t ifx)
{
  wifi_interface_t wifi_interface;

  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    wifi_interface=mp_obj_get_int(ifx);
    if(wifi_interface < ESP_IF_MAX)
    {
      if(codey_wifi_get_config(wifi_interface) != NULL)
      {
        return mp_const_none;
      }
    }
  }
  
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_wlan_get_config_obj, codey_wlan_get_config);

STATIC mp_obj_t codey_wlan_get_mac(mp_obj_t self_in, mp_obj_t ifx)
{
  uint8_t* mac = NULL;
  if(codey_wifi_info.wifi_enabled_flag == true)
  {
    wifi_interface_t wifi_ifx = mp_obj_get_int(ifx);
    mac = codey_wifi_get_mac(wifi_ifx);
    if(mac != NULL)
    {
      mp_obj_list_t *newlist = mp_obj_new_list(6,NULL);
      for(uint8_t i = 0; i < 6; i++)
      {
        newlist->items[i] = MP_OBJ_NEW_SMALL_INT(*mac);   
        mac++;
      }
      return (mp_obj_t)(newlist);  
    }

  }
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_wlan_get_mac_obj, codey_wlan_get_mac);

// Add by leo
STATIC mp_obj_t codey_wlan_sta_is_conn(mp_obj_t self_in)
{
  if(codey_wifi_sta_is_connected()) 
  { 
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_wlan_sta_is_conn_obj, codey_wlan_sta_is_conn);

STATIC mp_obj_t codey_wlan_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  // setup the object
  codey_wifi_info_t *self = &codey_wifi_info;
  self->base.type = &codey_wlan_type;
  
  return self;
}

STATIC void codey_wlan_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC const mp_map_elem_t codey_wlan_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_enable),                (mp_obj_t)&codey_wlan_enable_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_ap),                (mp_obj_t)&codey_wlan_set_ap_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_sta),               (mp_obj_t)&codey_wlan_set_sta_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_start),                 (mp_obj_t)&codey_wlan_start_obj },	
  { MP_OBJ_NEW_QSTR(MP_QSTR_stop),                  (mp_obj_t)&codey_wlan_stop_obj },	
  { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),                (mp_obj_t)&codey_wlan_deinit_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_scan),                  (mp_obj_t)&codey_wlan_scan_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_mac),               (mp_obj_t)&codey_wlan_get_mac_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_auto_connect),      (mp_obj_t)&codey_wlan_set_auto_connect_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_connect),               (mp_obj_t)&codey_wlan_connect_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_disconnect),            (mp_obj_t)&codey_wlan_disconnect_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_set_mode),              (mp_obj_t)&codey_wlan_set_mode_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_mode),              (mp_obj_t)&codey_wlan_get_mode_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_config),            (mp_obj_t)&codey_wlan_get_config_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_apssid_add_mac),        (mp_obj_t)&codey_wlan_ap_ssid_add_mac_set_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_sta_is_conn),           (mp_obj_t)&codey_wlan_sta_is_conn_obj },
  
  // class constants
  { MP_OBJ_NEW_QSTR(MP_QSTR_STA),                   MP_OBJ_NEW_SMALL_INT(WIFI_MODE_STA) },
  { MP_OBJ_NEW_QSTR(MP_QSTR_AP),                    MP_OBJ_NEW_SMALL_INT(WIFI_MODE_AP) },
  { MP_OBJ_NEW_QSTR(MP_QSTR_STA_AP),                MP_OBJ_NEW_SMALL_INT(WIFI_MODE_APSTA) },

};

STATIC MP_DEFINE_CONST_DICT(codey_wlan_locals_dict, codey_wlan_locals_dict_table);

const mp_obj_type_t codey_wlan_type =
{
  { &mp_type_type },
  .name = MP_QSTR_wlan,
  .print = codey_wlan_print,
  .make_new = codey_wlan_make_new,
  .locals_dict = (mp_obj_t)&codey_wlan_locals_dict,
};
