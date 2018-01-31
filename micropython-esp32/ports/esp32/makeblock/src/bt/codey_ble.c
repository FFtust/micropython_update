/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     
 * @file      codey_ble.c
 * @author    Leo lu
 * @version    V1.0.0
 * @date     2017/04/27
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
 * Leo lu             2017/04/27      1.0.0              Initial version
 * </pre>
 *
 */
  
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "py/nlr.h"
#include "py/obj.h"
#include "py/binary.h"
#include "py/mpstate.h"
#include "py/runtime.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "bt.h"
#include "bta_api.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gattc_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_heap_caps.h"
#include "codey_utils.h"
#include "objarray.h"
#include "codey_ble.h"
#include "codey_ble_prof.h"
#include "codey_ble_client.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#undef    TAG
#define   TAG                                 "BLE"
#define   DEF_APP_ID                          (0x00)
#define   DEF_GATTS_INT                       (0x00)
#define   PRI_SVR_DECLR_UUID_LEN              (2)
#define   PRI_SVR_DECLR_UUID_VALUE_0          (ESP_GATT_UUID_PRI_SERVICE & 0xFF)
#define   PRI_SVR_DECLR_UUID_VALUE_1          ((ESP_GATT_UUID_PRI_SERVICE >> 8) & 0xFF)
#define   SEC_SVR_DECLR_UUID_VALUE_0          (ESP_GATT_UUID_SEC_SERVICE & 0xFF)
#define   SEC_SVR_DECLR_UUID_VALUE_1          ((ESP_GATT_UUID_SEC_SERVICE >> 8) & 0xFF)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
typedef struct codey_ble_scan_ret{
  struct ble_scan_result_evt_param    scan_data;
  struct codey_ble_scan_ret           *next_scan_ret;
}codey_ble_scan_ret_t;

typedef struct codey_ble_obj{
  mp_obj_base_t                     base;
  bool                              is_init;
  uint8_t                           adv_dev_name[ESP_BLE_ADV_DATA_LEN_MAX];
  mp_obj_t                          gap_cb;
  codey_ble_prof_obj_t              *prof_list;
  codey_ble_scan_ret_t              *scan_list;
  codey_ble_client_obj_t            *client_list;
} codey_ble_obj_t;

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
static uint16_t s_app_id = 0;
static codey_ble_obj_t s_codey_ble_obj = { 0 };

static esp_ble_scan_params_t s_ble_scan_params = {
  .scan_type = BLE_SCAN_TYPE_ACTIVE,
  .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
  .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
  .scan_interval = 0x50,
  .scan_window = 0x30
};

static esp_ble_adv_data_t s_adv_config = {
  .set_scan_rsp = false,
  .include_name = true,
  .include_txpower = true,
  .min_interval = 0x20,
  .max_interval = 0x40,
  .appearance = 0x00,
  .manufacturer_len = 0,
  .p_manufacturer_data = NULL,
  .service_data_len = 0,
  .p_service_data = NULL,
  .service_uuid_len = 0,
  .p_service_uuid = NULL,
  .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t s_adv_params = {
  .adv_int_min = 0x20,
  .adv_int_max = 0x40,
  .adv_type = ADV_TYPE_IND,
  .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
  .channel_map = ADV_CHNL_ALL,
  .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
static mp_obj_t codey_ble_build_scan_ret_cb_args(struct ble_scan_result_evt_param *scan_ret_data);
static mp_obj_t codey_ble_build_get_srv_cb_args(esp_gatt_srvc_id_t *srvc_id);
static mp_obj_t codey_ble_build_get_char_cb_args(esp_gatt_id_t *char_id);
static mp_obj_t codey_ble_build_get_char_desc_cb_args(esp_gatt_id_t *char_desc_id);
static mp_obj_t codey_ble_build_gatts_conn_cb_args(struct gatts_connect_evt_param *conn_evt);
static mp_obj_t codey_ble_build_gatts_disconn_cb_args(struct gatts_disconnect_evt_param *disconn_evt);
static mp_obj_t codey_ble_build_write_char_cb_args(struct gatts_write_evt_param *write_data);
static codey_ble_scan_ret_t *codey_ble_new_scan_ret(struct ble_scan_result_evt_param *scan_ret_data);
static void codey_ble_clear_scan_ret_list(codey_ble_obj_t *ble);
static void codey_ble_add_scan_ret(codey_ble_obj_t *ble, codey_ble_scan_ret_t *add_scan_ret);
static codey_ble_scan_ret_t *codey_ble_find_scan_ret(codey_ble_obj_t *ble, esp_ble_addr_type_t bda);
static void codey_ble_add_prof(codey_ble_obj_t *ble, codey_ble_prof_obj_t *add_prof);
static void codey_ble_add_client(codey_ble_obj_t *ble, codey_ble_client_obj_t *add_client);
static void codey_ble_print_prof_list(codey_ble_prof_obj_t *prof);
static void codey_ble_print_client_list(codey_ble_client_obj_t *client);
static mp_obj_list_t *codey_ble_scan_ret_handler(codey_ble_obj_t *ble, struct ble_scan_result_evt_param *scan_ret_data);
static void codey_ble_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void codey_ble_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

/******************************************************************************
 DEDINE PUBLIC FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
static mp_obj_t codey_ble_build_scan_ret_cb_args(struct ble_scan_result_evt_param *scan_ret_data)
{
  mp_obj_list_t *scan_ret_cb_args = mp_obj_new_list(0, NULL);
  
  mp_obj_list_t *addr = codey_build_int_list_obj(scan_ret_data->bda, ESP_BD_ADDR_LEN);
  mp_obj_list_append(scan_ret_cb_args, addr);
  mp_obj_list_append(scan_ret_cb_args, mp_obj_new_int(scan_ret_data->ble_addr_type));
  mp_obj_list_append(scan_ret_cb_args, mp_obj_new_int(scan_ret_data->ble_evt_type));
  mp_obj_list_append(scan_ret_cb_args, mp_obj_new_int(scan_ret_data->rssi));
  mp_obj_list_t *adv_data = codey_build_int_list_obj(scan_ret_data->ble_adv, scan_ret_data->adv_data_len);
  mp_obj_list_append(scan_ret_cb_args, adv_data);
  
  return scan_ret_cb_args;
}
 
static mp_obj_t codey_ble_build_get_srv_cb_args(esp_gatt_srvc_id_t *srvc_id)
{
  mp_obj_list_t *get_srvc_cb_args = mp_obj_new_list(0, NULL);

  mp_obj_list_append(get_srvc_cb_args, mp_obj_new_int(srvc_id->id.inst_id));
  mp_obj_list_append(get_srvc_cb_args, mp_obj_new_int(srvc_id->is_primary));
  mp_obj_list_append(get_srvc_cb_args, codey_build_int_list_obj((uint8_t *)(&(srvc_id->id.uuid.uuid)), srvc_id->id.uuid.len));

  return get_srvc_cb_args;
}

static mp_obj_t codey_ble_build_get_char_cb_args(esp_gatt_id_t *char_id)
{
  mp_obj_list_t *get_char_cb_args = mp_obj_new_list(0, NULL);

  mp_obj_list_append(get_char_cb_args, mp_obj_new_int(char_id->inst_id));
  mp_obj_list_append(get_char_cb_args, codey_build_int_list_obj((uint8_t *)(&(char_id->uuid.uuid)), (char_id->uuid.len)));

  return get_char_cb_args;
}

static mp_obj_t codey_ble_build_get_char_desc_cb_args(esp_gatt_id_t *char_desc_id)
{
  mp_obj_list_t *get_char_desc_cb_args = mp_obj_new_list(0, NULL);

  mp_obj_list_append(get_char_desc_cb_args, mp_obj_new_int(char_desc_id->inst_id));
  mp_obj_list_append(get_char_desc_cb_args, codey_build_int_list_obj((uint8_t *)(&(char_desc_id->uuid.uuid)), (char_desc_id->uuid.len)));

  return get_char_desc_cb_args;
}

static mp_obj_t codey_ble_build_gatts_conn_cb_args(struct gatts_connect_evt_param *conn_evt)
{
  mp_obj_list_t *gatts_conn_cb_args = codey_build_int_list_obj((uint8_t *)(conn_evt->remote_bda), ESP_BD_ADDR_LEN);
  return gatts_conn_cb_args;
}

static mp_obj_t codey_ble_build_gatts_disconn_cb_args(struct gatts_disconnect_evt_param *disconn_evt)
{
  mp_obj_list_t *gatts_disconn_cb_args = codey_build_int_list_obj((uint8_t *)(disconn_evt->remote_bda), ESP_BD_ADDR_LEN);
  return gatts_disconn_cb_args;
}

static mp_obj_t codey_ble_build_write_char_cb_args(struct gatts_write_evt_param *write_data)
{
  mp_obj_list_t *write_char_cb_args = mp_obj_new_list(0, NULL);

  mp_obj_list_append(write_char_cb_args, mp_obj_new_int(write_data->handle));
  mp_obj_list_append(write_char_cb_args, codey_build_int_list_obj(write_data->value, write_data->len));

  return write_char_cb_args;
}

static codey_ble_scan_ret_t *codey_ble_new_scan_ret(struct ble_scan_result_evt_param *scan_ret_data)
{
  codey_ble_scan_ret_t *new_scan_ret;

  new_scan_ret = pvPortMalloc(sizeof(codey_ble_scan_ret_t));
  if(!new_scan_ret)
  {
    return NULL;
  }
  else
  {
    memcpy(&(new_scan_ret->scan_data), scan_ret_data, sizeof(struct ble_scan_result_evt_param));
    new_scan_ret->next_scan_ret = NULL;
    return new_scan_ret;
  }
}

static void codey_ble_clear_scan_ret_list(codey_ble_obj_t *ble)
{
  codey_ble_scan_ret_t *scan_ret, *next_scan_ret;

  scan_ret = ble->scan_list;
  while(scan_ret)
  {
    next_scan_ret = scan_ret->next_scan_ret;
    vPortFree(scan_ret);
    scan_ret = next_scan_ret;
  }

  ble->scan_list = NULL;
}

static void codey_ble_add_scan_ret(codey_ble_obj_t *ble, codey_ble_scan_ret_t *add_scan_ret)
{
  codey_ble_scan_ret_t *scan_ret;

  scan_ret = ble->scan_list;
  if(!scan_ret)
  {
    ble->scan_list = add_scan_ret;
  }
  else
  {
    while(scan_ret->next_scan_ret) scan_ret = scan_ret->next_scan_ret;
    scan_ret->next_scan_ret = add_scan_ret;
  }
}

static codey_ble_scan_ret_t *codey_ble_find_scan_ret(codey_ble_obj_t *ble, esp_ble_addr_type_t bda)
{
  codey_ble_scan_ret_t *scan_ret;

  scan_ret = ble->scan_list;
  while(scan_ret)
  {
    if(0 == memcmp(scan_ret->scan_data.bda, (void *)bda, ESP_BD_ADDR_LEN))
    {
      return scan_ret;
    }
    else
    {
      scan_ret = scan_ret->next_scan_ret;
    }
  }

  return NULL;
}

static void codey_ble_add_prof(codey_ble_obj_t *ble, codey_ble_prof_obj_t *add_prof)
{
  codey_ble_prof_obj_t *prof;

  prof = ble->prof_list;
  if(!prof)
  {
    ble->prof_list = add_prof;
  }
  else
  {
    while(prof->next_prof) prof = prof->next_prof;
    prof->next_prof = add_prof;
  }
}

static void codey_ble_add_client(codey_ble_obj_t *ble, codey_ble_client_obj_t *add_client)
{
  codey_ble_client_obj_t *client;

  client = ble->client_list;
  if(!client)
  {
    ble->client_list = add_client;
  }
  else
  {
    while(client->next_client) client = client->next_client;
    client->next_client = add_client;
  }
}

static void codey_ble_print_prof_list(codey_ble_prof_obj_t *prof)
{
  while(prof)
  {
    // ESP_LOGI(TAG,  "Prof.name = %s\n", prof->name);
    prof = prof->next_prof;
  }
}

static void codey_ble_print_client_list(codey_ble_client_obj_t *client)
{
  while(client)
  {
    codey_ble_client_print_c(client);
    client = client->next_client;
  }
}

static mp_obj_list_t *codey_ble_scan_ret_handler(codey_ble_obj_t *ble, struct ble_scan_result_evt_param *scan_ret_data)
{
  codey_ble_scan_ret_t *new_scan_ret;
  mp_obj_list_t *scan_ret_cb_args;

  scan_ret_cb_args = NULL;
  switch(scan_ret_data->search_evt)
  {
    case  ESP_GAP_SEARCH_INQ_RES_EVT:
          if(!codey_ble_find_scan_ret(ble, (esp_ble_addr_type_t)(scan_ret_data->bda)))
          {
            new_scan_ret = codey_ble_new_scan_ret(scan_ret_data);
            if(new_scan_ret)
            {
              codey_ble_add_scan_ret(ble, new_scan_ret);
              scan_ret_cb_args = codey_ble_build_scan_ret_cb_args(scan_ret_data);
            }
          }
    break;
    
    case  ESP_GAP_SEARCH_INQ_CMPL_EVT:
          // Do nothing
    break;
  
    default:
          // Do nothing
    break;
  }

  return scan_ret_cb_args;
}

static void codey_ble_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
  mp_obj_list_t *gap_cb_args;
  mp_obj_list_t *scan_ret_cb_args;

  if(ESP_GAP_BLE_SCAN_RESULT_EVT != event)
  {
    ESP_LOGI(TAG, "GAP_EVT, event %d", event);
  }

  switch (event) 
  {
    /*!< When advertising data set complete, the event comes */
    case  ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:    
    break;

    /*!< When scan response data set complete, the event comes */
    case  ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
    break;

    /*!< When scan parameters set complete, the event comes */
    case  ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
          esp_ble_gap_start_scanning(0xFFFF);
    break;

    /*!< When one scan result ready, the event comes each time */
    case  ESP_GAP_BLE_SCAN_RESULT_EVT:
          scan_ret_cb_args = codey_ble_scan_ret_handler(&s_codey_ble_obj, &(param->scan_rst));
          if(scan_ret_cb_args && s_codey_ble_obj.gap_cb)
          {
            gap_cb_args = mp_obj_new_list(0, NULL);
            mp_obj_list_append(gap_cb_args, mp_obj_new_int(event));
            mp_obj_list_append(gap_cb_args, scan_ret_cb_args);
            mp_sched_schedule(s_codey_ble_obj.gap_cb, gap_cb_args);
          }
    break;

    /*!< When raw advertising data set complete, the event comes */
    case  ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
    break;

    /*!< When raw advertising data set complete, the event comes */
    case  ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
    break;

    /*!< When start advertising complete, the event comes */
    case  ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
    break;

    /*!< When start scan complete, the event comes */
    case  ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
    break;

    /* Authentication complete indication. */
    case  ESP_GAP_BLE_AUTH_CMPL_EVT:
    break;

    /* BLE  key event for peer device keys */
    case  ESP_GAP_BLE_KEY_EVT:
    break;

    /* BLE  security request */
    case  ESP_GAP_BLE_SEC_REQ_EVT:
    break;

    /* passkey notification event */
    case  ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
    break;

    /* passkey request event */
    case  ESP_GAP_BLE_PASSKEY_REQ_EVT:
    break;

    /* OOB request event */
    case  ESP_GAP_BLE_OOB_REQ_EVT:
    break;

    /* BLE local IR event */
    case  ESP_GAP_BLE_LOCAL_IR_EVT:
    break;

    /* BLE local ER event */
    case  ESP_GAP_BLE_LOCAL_ER_EVT:
    break;

    /* Numeric Comparison request event */
    case  ESP_GAP_BLE_NC_REQ_EVT:
    break;

    /*!< When stop adv complete, the event comes */
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
    break;

    /*!< When stop scan complete, the event comes */
    case  ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
    break;
    
    default:
         ESP_LOGE(TAG, "GAP UNHANDLE EVENT: %d", event);
    break;
  }
}

static void codey_ble_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
  ESP_LOGI(TAG, "GATTS_EVT %d, interface %d", event, gatts_if);

  codey_ble_prof_obj_t *prof;
  codey_ble_srv_obj_t *srv;
  mp_obj_list_t *gatts_cb_args;

  if(ESP_GATTS_REG_EVT == event)
  {
    if(ESP_GATT_OK == param->reg.status) 
    {
      prof = s_codey_ble_obj.prof_list;
      while(prof)
      {
        if(prof->app_id == param->reg.app_id)
        {
          prof->interface = gatts_if;
          break;
        }
        else
        {
          prof = prof->next_prof;
        }
      }
      if(!prof)
      {
        ESP_LOGE(TAG, "Can not find a profile with interface: %d", gatts_if);
      }
    }
    else
    {
      ESP_LOGE(TAG, "Profile register err");
    }
    return;
  }

  prof = s_codey_ble_obj.prof_list;
  while(prof)
  {
    if(prof->interface == gatts_if)
    {
      break;
    }
  }
  if(!prof)
  {
    ESP_LOGE(TAG, "Can not find a profile with interface: %d", gatts_if);
    return;
  }

  gatts_cb_args = mp_obj_new_list(0, NULL);
  mp_obj_list_append(gatts_cb_args, mp_obj_new_int(event));
  switch (event) 
  {
    /*!< When register application id, the event comes */
    case ESP_GATTS_REG_EVT: 
    break;

    /*!< When gatt client request read operation, the event comes */
    case ESP_GATTS_READ_EVT:
    break;

    /*!< When gatt client request write operation, the event comes */
    case ESP_GATTS_WRITE_EVT:
    if(prof->gatts_cb)
    {
      mp_obj_list_append(gatts_cb_args, codey_ble_build_write_char_cb_args(&(param->write)));
      mp_sched_schedule(prof->gatts_cb, gatts_cb_args);
    }
    break;

    /*!< When gatt client request execute write, the event comes */
    case ESP_GATTS_EXEC_WRITE_EVT:
    break;

    /*!< When set mtu complete, the event comes */
    case ESP_GATTS_MTU_EVT:
    break;

    /*!< When receive confirm, the event comes */
    case ESP_GATTS_CONF_EVT:
    break;

    /*!< When unregister application id, the event comes */
    case ESP_GATTS_UNREG_EVT:
    break;

    /*!< When create service complete, the event comes */
    case ESP_GATTS_CREATE_EVT:
    break;

    /*!< When add included service complete, the event comes */
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
    break;

    /*!< When add characteristic complete, the event comes */
    case ESP_GATTS_ADD_CHAR_EVT:
    break;

    /*!< When add descriptor complete, the event comes */
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
    break;

    /*!< When delete service complete, the event comes */
    case ESP_GATTS_DELETE_EVT:
    break;

    /*!< When start service complete, the event comes */
    case ESP_GATTS_START_EVT:
    break;

    /*!< When stop service complete, the event comes */
    case ESP_GATTS_STOP_EVT:
    break;

    /*!< When gatt client connect, the event comes */
    case ESP_GATTS_CONNECT_EVT:
    prof->is_conn = true;
    prof->conn_id = param->connect.conn_id;
    memcpy(prof->client_bda, param->connect.remote_bda, ESP_BD_ADDR_LEN);
    if(prof->gatts_cb)
    {
      mp_obj_list_append(gatts_cb_args, codey_ble_build_gatts_conn_cb_args(&(param->connect)));
      mp_sched_schedule(prof->gatts_cb, gatts_cb_args);
    }
    break;

    /*!< When gatt client disconnect, the event comes */
    case ESP_GATTS_DISCONNECT_EVT:
    prof->is_conn = false;
    if(prof->gatts_cb)
    {
      mp_obj_list_append(gatts_cb_args, codey_ble_build_gatts_disconn_cb_args(&(param->disconnect)));
      mp_sched_schedule(prof->gatts_cb, gatts_cb_args);
    }
    esp_ble_gap_start_advertising(&s_adv_params);
    break;

    /*!< When connect to peer, the event comes */
    case ESP_GATTS_OPEN_EVT:
    break;

    /*!< When disconnect from peer, the event comes */
    case ESP_GATTS_CANCEL_OPEN_EVT:
    break;

    /*!< When gatt server close, the event comes */
    case ESP_GATTS_CLOSE_EVT:
    break;

    /*!< When gatt listen to be connected the event comes */
    case ESP_GATTS_LISTEN_EVT:
    break;

    /*!< When congest happen, the event comes */
    case ESP_GATTS_CONGEST_EVT:
    break;

    /*!< When gatt send response complete, the event comes */
    case ESP_GATTS_RESPONSE_EVT:
    break;
    
    case ESP_GATTS_CREAT_ATTR_TAB_EVT:
    if(param->add_attr_tab.num_handle > 0)
    {
      srv = codey_ble_prof_find_srv_by_uuid_c(prof, &(param->add_attr_tab.svc_uuid));
      if(srv)
      {
        memcpy(srv->hd_tab, param->add_attr_tab.handles, sizeof(uint16_t)*param->add_attr_tab.num_handle);
      }
    }
    break;

    case ESP_GATTS_SET_ATTR_VAL_EVT:
    break;
    
    default:
    break;
  }
}

static void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{ 
  ESP_LOGI(TAG, "GATTC_EVT %d, interface %d", event, gattc_if);

  codey_ble_client_obj_t *client;
  esp_gatt_srvc_id_t *srvc;
  codey_ble_find_srv_t *find_srv;
  codey_ble_find_char_elem_t *find_char;
  struct gattc_get_descr_evt_param *desc;
  struct gattc_read_char_evt_param *read;
  struct gattc_notify_evt_param *notify;
  struct gattc_reg_for_notify_evt_param *reg_for_notify;
  mp_obj_list_t *cb_args;

  if(ESP_GATTC_REG_EVT == event)
  {
    if(ESP_GATT_OK == param->reg.status) 
    {
      client = s_codey_ble_obj.client_list;
      while(client)
      {
        if(param->reg.app_id == client->app_id)
        {
          client->interface = gattc_if;
          break;
        }
        else
        {
          client = client->next_client;
        }
      }
      if(!client)
      {
        ESP_LOGE(TAG, "Can not find a client with interface: %d", gattc_if);
      }
    }
    else
    {
      ESP_LOGE(TAG, "Client register err");
    }
    return;
  }

  // Find a client with the gattc_if
  client = s_codey_ble_obj.client_list;
  while(client)
  {
    if(gattc_if== client->interface)
    {
      break;
    }
  }
  if(!client)
  {
    ESP_LOGE(TAG, "Can not find a client with interface: %d", gattc_if);
    return;
  }

  cb_args = mp_obj_new_list(0, NULL);
  mp_obj_list_append(cb_args, mp_obj_new_int(event));
  switch (event) {
    case ESP_GATTC_REG_EVT:
    // ESP_LOGI(TAG, "REG_EVT");
    break;
        
    case ESP_GATTC_OPEN_EVT:
      client->is_conn = true;
      client->conn_id = param->open.conn_id;
      memcpy(client->remote_bda, param->open.remote_bda, sizeof(esp_bd_addr_t));
      if(client->gattc_cb)
      {
        mp_sched_schedule(client->gattc_cb, cb_args);
      }
    break;

    case ESP_GATTC_READ_CHAR_EVT:
      read = &(param->read);
      if(ESP_GATT_OK == read->status)
      {
        // ESP_LOGI(TAG,  "char value: ");
        // codey_print_hex(read->value, read->value_len);
        // ESP_LOGI(TAG,  "\n");
      }
      else
      {
        // Do nothing
      }
    break;
        
    case ESP_GATTC_SEARCH_RES_EVT:
      srvc = &(param->search_res.srvc_id);
      // ESP_LOGI(TAG, "= ESP_GATTC_SEARCH_RES_EVT =");
      // ESP_LOGI(TAG, "conn_id: %d", param->search_res.conn_id);
      // ESP_LOGI(TAG,  "srv_uuid: ");
      // codey_print_hex((uint8_t *)(&(srvc->id.uuid.uuid)), srvc->id.uuid.len);
      // ESP_LOGI(TAG,  "\n");
      if(client->is_search_all_srv)
      {
        find_srv = pvPortMalloc(sizeof(codey_ble_find_srv_t));
        memcpy(&(find_srv->srv), srvc, sizeof(esp_gatt_srvc_id_t));
        codey_ble_add_find_srv(client, find_srv);
      }
      else
      {
        if(client->gattc_cb)
        {
          mp_obj_list_append(cb_args, codey_ble_build_get_srv_cb_args(srvc));
          mp_sched_schedule(client->gattc_cb, cb_args);
        }
      }
    break;

    case ESP_GATTC_SEARCH_CMPL_EVT:
      if(client->is_search_all_srv)
      {
        if(client->gattc_cb)
        {
          find_srv = client->find_srv_list;
          while(find_srv)
          {
            mp_obj_list_append(cb_args, codey_ble_build_get_srv_cb_args(&(find_srv->srv)));
            find_srv = find_srv->next_srv;
          }
          mp_sched_schedule(client->gattc_cb, cb_args);
        }
        client->is_search_all_srv = false;
      }
    break;

    case ESP_GATTC_NOTIFY_EVT:
      notify = &(param->notify);
      // ESP_LOGI(TAG, "= ESP_GATTC_NOTIFY_EVT =");
      // ESP_LOGI(TAG,  "notify: ");
      // codey_print_hex(notify->value, notify->value_len);
    break;

    case ESP_GATTC_GET_CHAR_EVT:
      if(ESP_GATT_OK == param->get_char.status)
      {
        // ESP_LOGI(TAG,  "SRV ");
        // codey_ble_print_uuid(&(param->get_char.srvc_id.id.uuid));
        // ESP_LOGI(TAG,  "CHAR ");
        // codey_ble_print_uuid(&(param->get_char.char_id.uuid));
        // ESP_LOGI(TAG,  "instan: %d\n", param->get_char.char_id.inst_id);

        codey_ble_add_find_char(client, &(param->get_char.char_id));
        if(client->is_search_char)
        {
          esp_ble_gattc_get_characteristic(client->interface, client->conn_id, &(client->find_char_list.srv), &(param->get_char.char_id));
        }
        else
        {
          if(client->gattc_cb)
          {            
            mp_obj_list_append(cb_args, codey_ble_build_get_srv_cb_args(&(param->get_char.srvc_id)));
            mp_obj_list_append(cb_args, codey_ble_build_get_char_cb_args(&(param->get_char.char_id)));
            mp_sched_schedule(client->gattc_cb, cb_args);
          }
        }
      }
      else
      {
        client->is_search_char = false;
        if(client->gattc_cb)
        {
          mp_obj_list_append(cb_args, codey_ble_build_get_srv_cb_args(&(param->get_char.srvc_id)));
          find_char = client->find_char_list.char_list;
          while(find_char)
          {
            mp_obj_list_append(cb_args, codey_ble_build_get_char_cb_args(&(find_char->char_data)));
            find_char = find_char->next_char;
          }
          mp_sched_schedule(client->gattc_cb, cb_args);
        }
      }
    break;

    case ESP_GATTC_GET_DESCR_EVT:
      desc = &(param->get_descr);
      if(ESP_GATT_OK == desc->status)
      {
        mp_obj_list_append(cb_args, codey_ble_build_get_srv_cb_args(&(desc->srvc_id)));
        mp_obj_list_append(cb_args, codey_ble_build_get_char_cb_args(&(desc->char_id)));
        mp_obj_list_append(cb_args, codey_ble_build_get_char_desc_cb_args(&(desc->descr_id)));
        mp_sched_schedule(client->gattc_cb, cb_args);
      }
      else
      {
        // Do nothing
      }
    break;

    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
    reg_for_notify = &(param->reg_for_notify);
    ESP_LOGI(TAG, "= ESP_GATTC_REG_FOR_NOTIFY_EVT =");
    if(ESP_OK == reg_for_notify->status)
    {
      ESP_LOGI(TAG, "register for notify OK");
    }
    break;
    
    default:
    break;
  }
}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
static mp_obj_t codey_ble_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  codey_ble_obj_t *self = &s_codey_ble_obj;
  self->base.type = &codey_ble_type;
  return self;
}

static void codey_ble_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
  return;
}

static mp_obj_t codey_ble_deinit(mp_obj_t ble_in);
static mp_obj_t codey_ble_init(mp_obj_t ble_in)
{
  codey_ble_obj_t *ble = ble_in;

  if(ble->is_init)
  {
    codey_ble_deinit(ble);
  }

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  if(ESP_OK != esp_bt_controller_init(&bt_cfg)) 
  {
    return mp_const_false;
  }

  if(ESP_OK != esp_bt_controller_enable(ESP_BT_MODE_BTDM)) 
  {
     return mp_const_false;
  }
  
  if(ESP_OK != esp_bluedroid_init()) 
  {
    return mp_const_false;
  }

  if(ESP_OK != esp_bluedroid_enable()) 
  {
    return mp_const_false;
  }
  
  if(ESP_OK != esp_ble_gatts_register_callback(codey_ble_gatts_event_handler))
  {
    return mp_const_false;
  }
  
  if(ESP_OK != esp_ble_gattc_register_callback(gattc_event_handler))
  {
    return mp_const_false;
  }

  if(ESP_OK != esp_ble_gap_register_callback(codey_ble_gap_event_handler))
  {
    return mp_const_false;
  }

  ble->is_init = true;

  return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_init_obj, codey_ble_init);

static mp_obj_t codey_ble_deinit(mp_obj_t ble_in)
{
  codey_ble_obj_t *ble = ble_in;

  if(!ble->is_init)
  {
    return mp_const_false;
  }

  esp_ble_gatts_app_unregister(DEF_APP_ID);  

  if(ESP_OK != esp_bluedroid_disable())
  {
    return mp_const_false;
  }

  if(ESP_OK != esp_bluedroid_deinit())
  {
    return mp_const_false;
  }
  
  if(ESP_OK != esp_bt_controller_disable(ESP_BT_MODE_BTDM))
  {
    return mp_const_false;
  }
  
  esp_bt_controller_deinit();
  ble->is_init = false;
  
  return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_deinit_obj, codey_ble_deinit);

static mp_obj_t codey_ble_reg_gap_cb(mp_obj_t ble_in, mp_obj_t gap_cb_in)
{
  codey_ble_obj_t *ble = ble_in;
  if(mp_obj_is_callable(gap_cb_in))
  {
    ble->gap_cb = gap_cb_in;
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_2(codey_ble_reg_gap_cb_obj, codey_ble_reg_gap_cb);

static mp_obj_t codey_ble_start_adv(mp_obj_t ble_in, mp_obj_t dev_name_in)
{
  const char *dev_name = NULL;
  codey_ble_obj_t *ble = ble_in;

  if(!ble->is_init)
  {
    return mp_const_false;
  }

  dev_name = mp_obj_str_get_str(dev_name_in);
  if(!dev_name)
  {
    return mp_const_false;
  }

  if(strlen(dev_name) > ESP_BLE_ADV_DATA_LEN_MAX)
  {
    return mp_const_false;
  }
  strcpy((char *)ble->adv_dev_name, dev_name);

  esp_ble_gap_stop_advertising();

  if(ESP_OK != esp_ble_gap_set_device_name((const char *)s_codey_ble_obj.adv_dev_name))
  {
    return mp_const_false;
  }
  
  if(ESP_OK != esp_ble_gap_config_adv_data(&s_adv_config))
  {
    return mp_const_false;
  }

  esp_ble_gap_start_advertising(&s_adv_params);
  
  return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_2(codey_ble_start_adv_obj, codey_ble_start_adv);

static mp_obj_t codey_ble_stop_adv(mp_obj_t ble_in)
{
  codey_ble_obj_t *ble = ble_in;

  if(!ble->is_init)
  {
    return mp_const_false;
  }

  if(ESP_OK != esp_ble_gap_stop_advertising())
  {
    return mp_const_false;
  }
  else
  {
    return mp_const_true;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_stop_adv_obj, codey_ble_stop_adv);

static mp_obj_t codey_ble_scan(mp_obj_t ble_in)
{
  codey_ble_obj_t *ble = (codey_ble_obj_t *)ble_in;

  if(!ble->is_init)
  {
    return mp_const_false;
  }

  if(ESP_OK == esp_ble_gap_set_scan_params(&s_ble_scan_params))
  {
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_scan_obj, codey_ble_scan);

static mp_obj_t codey_ble_stop_scan(mp_obj_t ble_in)
{
  codey_ble_obj_t *ble = ble_in;

  if(!ble->is_init)
  {
    return mp_const_false;
  }

  if(ESP_OK == esp_ble_gap_stop_scanning())
  {
    codey_ble_clear_scan_ret_list(ble);
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_1(codey_ble_stop_scan_obj, codey_ble_stop_scan);

static mp_obj_t codey_ble_new_prof(mp_obj_t ble_in, mp_obj_t prof_name_in)
{
  codey_ble_obj_t *ble = ble_in;
  const char *name;
  codey_ble_prof_obj_t *new_prof;

  if(!ble->is_init)
  {
    return mp_const_none;
  }

  new_prof = NULL;
  name = NULL;

  new_prof = pvPortMalloc(sizeof(codey_ble_prof_obj_t));
  memset(new_prof, 0, sizeof(codey_ble_prof_obj_t));

  new_prof->base.type = &codey_ble_prof_type;
  name = mp_obj_str_get_str(prof_name_in);
  new_prof->name = pvPortMalloc(strlen(name));
  strcpy(new_prof->name, name);
  new_prof->app_id = s_app_id++;
  new_prof->srv_list = NULL;
  new_prof->ble = (codey_ble_obj_t *)ble;
  codey_ble_add_prof(ble, new_prof);
  codey_ble_print_prof_list(ble->prof_list);
  
  if(ESP_OK == esp_ble_gatts_app_register(new_prof->app_id))
  {
    return new_prof;
  }
  else
  {
    vPortFree(new_prof->name);
    vPortFree(new_prof);
    return mp_const_none;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_2(codey_ble_new_prof_obj, codey_ble_new_prof);

static mp_obj_t codey_ble_new_client(mp_obj_t ble_in, mp_obj_t client_name_in)
{
  codey_ble_obj_t *ble = ble_in;
  const char *name;
  codey_ble_client_obj_t *client;

  if(!ble->is_init)
  {
    return mp_const_none;
  }
  
  client = NULL;
  name = NULL;

  client = pvPortMalloc(sizeof(codey_ble_client_obj_t));
  memset(client, 0, sizeof(codey_ble_client_obj_t));

  client->base.type = &codey_ble_client_type;
  name = mp_obj_str_get_str(client_name_in);
  client->name = pvPortMalloc(strlen(name));
  strcpy(client->name, name);
  client->app_id = s_app_id++;
  client->ble = ble;
  codey_ble_add_client(ble, client);
  codey_ble_print_client_list(ble_in);

  if(ESP_OK == esp_ble_gattc_app_register(client->app_id))
  {
    return client;
  }
  else
  {
    vPortFree(client->name);
    vPortFree(client);
    return mp_const_none;
  }
}
static MP_DEFINE_CONST_FUN_OBJ_2(codey_ble_new_client_obj, codey_ble_new_client);

static const mp_map_elem_t codey_ble_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_init),                (mp_obj_t)&codey_ble_init_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_deinit),              (mp_obj_t)&codey_ble_deinit_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_reg_gap_cb),          (mp_obj_t)&codey_ble_reg_gap_cb_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_start_adv),           (mp_obj_t)&codey_ble_start_adv_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_stop_adv),            (mp_obj_t)&codey_ble_stop_adv_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_scan),                (mp_obj_t)&codey_ble_scan_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_stop_scan),           (mp_obj_t)&codey_ble_stop_scan_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_new_prof),            (mp_obj_t)&codey_ble_new_prof_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_new_client),          (mp_obj_t)&codey_ble_new_client_obj },
};

static MP_DEFINE_CONST_DICT(codey_ble_locals_dict, codey_ble_locals_dict_table);
const mp_obj_type_t codey_ble_type =
{
  { &mp_type_type },
  .name = MP_QSTR_ble,
  .print = codey_ble_print,
  .make_new = codey_ble_make_new,
  .locals_dict = (mp_obj_t)&codey_ble_locals_dict,
};


// END OF FILE
