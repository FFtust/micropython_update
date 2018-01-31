/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     
 * @file      codey_ble_sys_dev_func.c
 * @author    Leo lu
 * @version    V1.0.0
 * @date     2017/08/10
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
 * Leo lu             2017/08/10      1.0.0              Initial version
 * Leo lu             2017/10/23      1.0.1              Tx using block sending method when connecting with android
 * </pre>
 *
 */
  
#include <stdint.h>
#include <string.h>
#include <stdio.h>
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
#include "codey_ringbuf.h"
#include "esp_system.h"
#include "codey_neurons_deal.h"
#include "codey_ble_sys_dev_func.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#undef  TAG
#define TAG                              ("CODEY_SYS_DEV")
#define DEVICE_NAME_PREFIX               ("Makeblock_LE")
#define CODEY_SYS_DEV_APP_ID             (0xA5)
#define CODEY_SYS_DEV_SVR_INST_ID        (0)

#define BLE_MAX_ONE_BLOCK_BYTES          (20)
#define CHAR_DECLARATION_SIZE            (sizeof(uint8_t))

#define CODEY_BLE_SYS_DEV_TX_MAX_LEN     (BLE_MAX_ONE_BLOCK_BYTES)
#define CODEY_BLE_SYS_DEV_RX_MAX_LEN     (64)
#define CODEY_BLE_DEV_BUF_SIZE           (256)

/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
 // Attributes State Machine
 enum
 {
     CODEY_SYS_DEV_IDX_SVC = 0,
 
     CODEY_SYS_DEV_RX_CHAR_DECL,
     CODEY_SYS_DEV_RX_CHAR_VAL,
 
     CODEY_SYS_DEV_TX_CHAR_DECL,
     CODEY_SYS_DEV_TX_CHAR_VAL,
     CODEY_SYS_DEV_TX_CHAR_NTF_CFG,
 
     CODEY_SYS_DEV_IDX_NB,
 };

/******************************************************************************
 DEFINE PRIVATE DATAS
 ******************************************************************************/
static const uint16_t s_primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t s_character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t s_character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t s_char_prop_notify = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t s_char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t s_char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t s_char_prop_write_write_nr = ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_WRITE_NR;

static const uint16_t s_codey_sys_dev_svc = 0xFFE1;
static const uint16_t s_codey_sys_dev_tx_uuid = 0xFFE2;
static const uint16_t s_codey_sys_dev_rx_uuid = 0xFFE3;
static const uint8_t  s_codey_sys_dev_tx_ccc[2] ={ 0x00, 0x00 };

static esp_ble_adv_data_t s_codey_sys_dev_adv_config = 
{
  .set_scan_rsp = false,
  .include_name = true,
  .include_txpower = true,
  .min_interval = 0x100,
  .max_interval = 0x100,
  .appearance = 0x00,
  .manufacturer_len = 0,
  .p_manufacturer_data =  NULL,
  .service_data_len = 0,
  .p_service_data = NULL,
  .service_uuid_len = 0,
  .p_service_uuid = NULL,
  .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t s_codey_sys_dev_adv_params = 
{
  .adv_int_min        = 0x100,
  .adv_int_max        = 0x100,
  .adv_type           = ADV_TYPE_IND,
  .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
  .channel_map        = ADV_CHNL_ALL,
  .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static uint8_t s_dev_name[ESP_BLE_ADV_DATA_LEN_MAX];
bool s_gatts_conned = false;
esp_gatt_if_t s_gatts_if;
uint16_t s_conn_id;
uint16_t s_codey_sys_dev_handle_table[CODEY_SYS_DEV_IDX_NB];

/// Used to add attributes into the database
static const esp_gatts_attr_db_t s_codey_sys_dev_gatt_db[CODEY_SYS_DEV_IDX_NB] =
{
  // Service Declaration
  [CODEY_SYS_DEV_IDX_SVC]                =  
  {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&s_primary_service_uuid, ESP_GATT_PERM_READ,
    sizeof(uint16_t), sizeof(s_codey_sys_dev_svc), (uint8_t *)&s_codey_sys_dev_svc}},

  // RX Characteristic Declaration
  [CODEY_SYS_DEV_RX_CHAR_DECL]           =  
  {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&s_character_declaration_uuid, ESP_GATT_PERM_READ,
    CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&s_char_prop_write_write_nr}},
  
  // RX Characteristic Value
  [CODEY_SYS_DEV_RX_CHAR_VAL]            = 
  {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&s_codey_sys_dev_rx_uuid, ESP_GATT_PERM_WRITE,
    CODEY_BLE_SYS_DEV_RX_MAX_LEN, 0, NULL}},

  // TX Characteristic Declaration
  [CODEY_SYS_DEV_TX_CHAR_DECL]           = 
  {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&s_character_declaration_uuid, ESP_GATT_PERM_READ,
    CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&s_char_prop_notify}},
    
  // TX Characteristic Value
  [CODEY_SYS_DEV_TX_CHAR_VAL]            =   
  {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&s_codey_sys_dev_tx_uuid, ESP_GATT_PERM_READ,
    CODEY_BLE_SYS_DEV_TX_MAX_LEN,0, NULL}},

  // TX Client Characteristic Configuration Descriptor
  [CODEY_SYS_DEV_TX_CHAR_NTF_CFG]        =    
  {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&s_character_client_config_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
    sizeof(uint16_t), sizeof(s_codey_sys_dev_tx_ccc), (uint8_t *)s_codey_sys_dev_tx_ccc}},
};

static uint8_t s_rx_buf[ CODEY_BLE_DEV_BUF_SIZE ];
static codey_ring_buf_t s_rx_rb = { CODEY_BLE_DEV_BUF_SIZE-1, 0, 0, s_rx_buf };
static ble_connected_indicate_func s_ble_connected_indicate_handle = NULL;

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
static void codey_ble_sys_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void codey_ble_sys_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_ble_register_connected_func(ble_connected_indicate_func func)
{
  s_ble_connected_indicate_handle = func;
}

int codey_ble_enter_sys_dev_mode(void)
{
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  if(ESP_OK != esp_bt_controller_init(&bt_cfg)) 
  {
    return -1;
  }

  if(ESP_OK != esp_bt_controller_enable(ESP_BT_MODE_BLE)) 
  {
     return -1;
  }
  
  extern void esp32_heap_info_show_t();
  printf("befor release:\n");
  esp32_heap_info_show_t();
  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
  printf("after release:\n");
  esp32_heap_info_show_t();
  
  if(ESP_OK != esp_bluedroid_init()) 
  {
    return -1;
  }

  if(ESP_OK != esp_bluedroid_enable()) 
  {
    return -1;
  }
  
  if(ESP_OK != esp_ble_gatts_register_callback(codey_ble_sys_gatts_event_handler))
  {
    return -1;
  }

  if(ESP_OK != esp_ble_gap_register_callback(codey_ble_sys_gap_event_handler))
  {
    return -1;
  }

  if(ESP_OK != esp_ble_gatts_app_register(CODEY_SYS_DEV_APP_ID))
  {
    return -1;
  }
  /* set the security iocap & auth_req & key size & init key response key parameters to the stack*/
  esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND;     //bonding with peer device after authentication
  esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
  uint8_t key_size = 16;      //the key size should be 7~16 bytes
  uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
  /* If your BLE device act as a Slave, the init_key means you hope which types of key of the master should distribut to you,
  and the response key means which key you can distribut to the Master;
  If your BLE device act as a master, the response key means you hope which types of key of the slave should distribut to you, 
  and the init key means which key you can distribut to the slave. */
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));

  /* Just show how to clear all the bonded devices 
   * Delay 30s, clear all the bonded devices
   * 
   * vTaskDelay(30000 / portTICK_PERIOD_MS);
   * remove_all_bonded_devices();
   */
  return 0;
}

int codey_ble_dev_get_char(uint8_t *c)
{
  if(!RING_BUF_IS_EMPTY(&s_rx_rb))
  {
    *c = codey_ring_buf_get(&s_rx_rb);
    return 0;
  }
  else
  {
    return -1;
  }
}

void codey_ble_dev_get_data(uint8_t *out_buf, uint16_t *len)
{
  uint16_t i;
  uint16_t buf_size = *len;

  for(i = 0; i < buf_size; i++)
  {
    if(!RING_BUF_IS_EMPTY(&s_rx_rb))
    {
      out_buf[i] = codey_ring_buf_get(&s_rx_rb);
    }
    else
    {
      break;
    }
  }

  *len = i;
}

int codey_ble_dev_put_data(uint8_t *data, uint16_t len)
{
  uint16_t i = 0;

  // ESP_LOGI(TAG, "Total tx len: %d", len); 
  for(; i < len/BLE_MAX_ONE_BLOCK_BYTES; i++)
  {
    if(s_gatts_conned)
    {
      // ESP_LOGI(TAG, "BLE tx: %d block", i); 
      // ESP_LOGI(TAG, "BLE tx: %d th data, len: %d", (uint32_t)(BLE_MAX_ONE_BLOCK_BYTES*i), BLE_MAX_ONE_BLOCK_BYTES);
      if(ESP_OK != esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id, s_codey_sys_dev_handle_table[CODEY_SYS_DEV_TX_CHAR_VAL], 
                                                  (uint16_t)BLE_MAX_ONE_BLOCK_BYTES, data + BLE_MAX_ONE_BLOCK_BYTES*i, false))
      {
        return -1;
      }
    }
    else
    {
      return -1;
    }

  }
  
  if(s_gatts_conned)
  {
    // ESP_LOGI(TAG, "BLE tx: %d block", i); 
    // ESP_LOGI(TAG, "BLE tx: %dth data, len: %d", (uint32_t)(BLE_MAX_ONE_BLOCK_BYTES*i), len%BLE_MAX_ONE_BLOCK_BYTES);
    if(ESP_OK != esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id, s_codey_sys_dev_handle_table[CODEY_SYS_DEV_TX_CHAR_VAL], 
                                                (uint16_t)len%BLE_MAX_ONE_BLOCK_BYTES, data + BLE_MAX_ONE_BLOCK_BYTES*i, false))
    {
      return -1;
    }
  }
  else
  {
    return -1;
  }
  
  return 0;
}


/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
static void codey_ble_sys_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
  uint32_t i;
  uint8_t dev_addr[BD_ADDR_LEN];
  
  struct gatts_write_evt_param *write_data;
  // ESP_LOGI(TAG, "GATTS_EVT %d, interface %d", event, gatts_if);

  if(ESP_GATTS_REG_EVT == event)
  {
    if(ESP_GATT_OK == param->reg.status) 
    {
      // ESP_LOGI(TAG, "%s %d\n", __func__, __LINE__);
      memcpy(s_dev_name, DEVICE_NAME_PREFIX, sizeof(DEVICE_NAME_PREFIX)-1);
      esp_efuse_mac_get_default(dev_addr);
      // Why do these? I do't known, but if real device addr is E, then we get 'C', so just ++
      dev_addr[BD_ADDR_LEN-1] += 2;
      codey_to_hex_str(dev_addr, BD_ADDR_LEN, s_dev_name + sizeof(DEVICE_NAME_PREFIX)-1);
      s_dev_name[sizeof(DEVICE_NAME_PREFIX)-1 + BD_ADDR_LEN*2] = 0;
      ESP_LOGI(TAG, "BLE device name: %s\r\n", s_dev_name)
      esp_ble_gap_set_device_name((const char *)s_dev_name);
      // esp_ble_gap_set_device_name(DEVICE_NAME_PREFIX);
      esp_ble_gap_config_adv_data(&s_codey_sys_dev_adv_config);
      esp_ble_gatts_create_attr_tab(s_codey_sys_dev_gatt_db, gatts_if, CODEY_SYS_DEV_IDX_NB, CODEY_SYS_DEV_SVR_INST_ID);
      s_gatts_if = gatts_if;
    }
    else
    {
      // // ESP_LOGE(TAG, "Profile register err");
    }
    return;
  }

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
    write_data = (struct gatts_write_evt_param *)param;
    // ESP_LOGI(TAG, "write handle: 0x%04x\r\n", write_data->handle);
    if(s_codey_sys_dev_handle_table[CODEY_SYS_DEV_RX_CHAR_VAL] == write_data->handle)
    {
      // codey_print_hex(write_data->value, write_data->len);
      // ESP_LOGI(TAG, "p:%d, g:%d\r\n", s_rx_rb.iput, s_rx_rb.iget);
      for(i = 0; i < write_data->len; i++)
      {
        if(!RING_BUF_IS_FULL(&s_rx_rb))
        {
          codey_ring_buf_put(&s_rx_rb, write_data->value[i]);
        }
        else
        {
          // // ESP_LOGE(TAG, "rx ring buffer overflow\r\n");
          break;
        }
      }
      codey_give_data_recv_sem();
    }
    else
    {
      // ESP_LOGI(TAG, "for other path\r\n");
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
    s_gatts_conned = true;
    s_conn_id = param->connect.conn_id;
    if(s_ble_connected_indicate_handle != NULL)
    {
      s_ble_connected_indicate_handle();
    }
    break;

    /*!< When gatt client disconnect, the event comes */
    case ESP_GATTS_DISCONNECT_EVT:
    // restart adv
    esp_ble_gap_start_advertising(&s_codey_sys_dev_adv_params);
    s_gatts_conned = false;
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
      // ESP_LOGI(TAG, "The number handle = %x\n", param->add_attr_tab.num_handle);
      if(param->add_attr_tab.status == ESP_GATT_OK)
      {
          memcpy(s_codey_sys_dev_handle_table, param->add_attr_tab.handles, sizeof(s_codey_sys_dev_handle_table));
          esp_ble_gatts_start_service(s_codey_sys_dev_handle_table[CODEY_SYS_DEV_IDX_SVC]);
      }
    break;

    case ESP_GATTS_SET_ATTR_VAL_EVT:
    break;
    
    default:
    break;
  }
}

static void codey_ble_sys_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
  if(ESP_GAP_BLE_SCAN_RESULT_EVT != event)
  {
    // ESP_LOGI(TAG, "GAP_EVT, event %d", event);
  }

  switch (event) 
  {
    /*!< When advertising data set complete, the event comes */
    case  ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&s_codey_sys_dev_adv_params);
    break;

    /*!< When scan response data set complete, the event comes */
    case  ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
    break;

    /*!< When scan parameters set complete, the event comes */
    case  ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
    break;

    /*!< When one scan result ready, the event comes each time */
    case  ESP_GAP_BLE_SCAN_RESULT_EVT:
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
        /* send the positive(true) security response to the peer device to accept the security request.
        If not accept the security request, should sent the security response with negative(false) accept value*/
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
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
         // // ESP_LOGE(TAG, "GAP UNHANDLE EVENT: %d", event);
    break;
  }
}

// END OF FILE
