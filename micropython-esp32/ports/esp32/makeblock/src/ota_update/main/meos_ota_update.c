/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for makeblock OTA module
 * @file    meos_ota_update.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/08/02
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
 * This file is a drive gyro_sensor module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/08/02      1.0.0              build the new.
 * </pre>
 *
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "meos_ota_update.h"

/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/
#define OTA_TAG "OTA_UPDATE"

#define MAGIC_DEFAULT 0x55

#define SOFTWARE_VERSION_DEFAULT "V1.0"

#define WIFI_SSID_DEFAULT "Maker-office"
#define WIFI_PASS_DEFAULT "hulurobot423"

#define HTTP_SERVER_URL_DEFAULT ""
#define HTTP_SERVER_IP_DEFAULT  "10.0.1.212"
#define HTTP_SERVER_PORT_DEFAULT "8070"

#define OTA_UPDATE_FILE_NAME_DEFAULT "application.bin"

#define WIFI_SSID_LEN_MAX 32
#define WIFI_PASS_LEN_MAX 32
#define HTTP_URL_LEN_MAX  128
#define HTTP_IP_LEN_MAX   32
#define HTTP_PORT_LEN_MAX 8
#define VERSION_LEN_MAX   8
#define UPDATE_FILE_NAME_LEN_MAX 32

#define OTA_MAX(x, y) ((x) > (y) ? (x) : (y))
/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/
 
/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/


/******************************************************************************
 DEFINE PUBLIC VARIABLES
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/


/******************************************************************************
 DECLARE PUBLIC FUNCTIONS  
 ******************************************************************************/
 

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/



/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void mb_user_partition_test_t()
{
  const esp_partition_t *user_partition = NULL;
  
  ESP_LOGI(OTA_TAG, "makeblock: user partition test start\n");
  esp_err_t ff_ret = 0;
  uint8_t ff_test[256];
  memset(ff_test, 0 , 256);
  /* the user partition set in the partition_two_ota_bak.csv  */
  /* and type is set to data , subtype is set to coredump(system hold, but not be used now)*/
  user_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_COREDUMP, NULL); 
  ff_ret = esp_partition_read(user_partition, 0, ff_test, 100);
  ESP_LOGI(OTA_TAG, "init read ret is %d ,data[15] is: %d \n", ff_ret, ff_test[15]);
  
  memset(ff_test, 10 , 256);
  ff_ret = esp_partition_erase_range(user_partition, 0, user_partition->size);
  ESP_LOGI(OTA_TAG, "erase ret is %d", ff_ret);
  ff_ret = esp_partition_write(user_partition, 0, ff_test, 100);
  ESP_LOGI(OTA_TAG, "write ret is %d \n", ff_ret);	
  ff_ret = esp_partition_read(user_partition, 0, ff_test, 100);
  ESP_LOGI(OTA_TAG, "second read ret is %d ,data[15] is: %d \n", ff_ret, ff_test[15]);
  
  
  
  ESP_LOGI(OTA_TAG, "makeblock: user partition test end\n");



}

esp_err_t meos_ota_update_set_wifi_info(char *ssid, char *pass, ota_update_info_t *ota_info)
{
  if((strlen(ssid) > WIFI_SSID_LEN_MAX) || (strlen(pass) > WIFI_PASS_LEN_MAX))
  {
    return ESP_ERR_INVALID_ARG;
  }
  strcpy(ota_info->wifi_ssid, ssid);
  strcpy(ota_info->wifi_pass, pass);
  
  return ESP_OK;
}


esp_err_t meos_ota_update_set_http_info(char *url, char *ip, char *port, ota_update_info_t *ota_info)
{
  if((strlen(url) > HTTP_URL_LEN_MAX) || (strlen(ip) > HTTP_IP_LEN_MAX) || (strlen(port) > HTTP_PORT_LEN_MAX))
  {
    return ESP_ERR_INVALID_ARG;
  }  
  strcpy(ota_info->http_server_url, url);
  strcpy(ota_info->http_server_ip, ip);
  strcpy(ota_info->http_port, port);

  return ESP_OK;
}

esp_err_t  meos_ota_update_set_region_type(ota_update_region_type_t update_region_type, ota_update_info_t *ota_info)
{
  if(update_region_type >= OTA_UPDATE_MAX)
  {
    return ESP_ERR_INVALID_ARG;
  }
  ota_info->update_region_type = update_region_type;
  return ESP_OK;
}

esp_err_t meos_ota_update_set_file_name(char *file_name, ota_update_info_t *ota_info)
{
  if(strlen(file_name)> UPDATE_FILE_NAME_LEN_MAX)
  {
    return ESP_ERR_INVALID_ARG;
  }
  strcpy(ota_info->file_name, file_name);

  return ESP_OK;

}

esp_err_t meos_ota_update_set_version(char *version, ota_update_info_t *ota_info)
{
  if(strlen(version)> VERSION_LEN_MAX)
  {
    return ESP_ERR_INVALID_ARG;
  }

  strcpy(ota_info->ota_update_version, version);
  return ESP_OK;
}

void meos_ota_update_set_default_partition_info(ota_update_info_t *ota_info)
{
  ota_info->magic = MAGIC_DEFAULT;
  strcpy(ota_info->ota_current_version, SOFTWARE_VERSION_DEFAULT);
  strcpy(ota_info->ota_update_version, SOFTWARE_VERSION_DEFAULT);
  strcpy(ota_info->wifi_ssid, WIFI_SSID_DEFAULT);
  ota_info->wifi_ssid_len = strlen(ota_info->wifi_ssid);  
  strcpy(ota_info->wifi_pass, WIFI_PASS_DEFAULT);
  ota_info->wifi_pass_len = strlen(ota_info->wifi_pass);
  strcpy(ota_info->http_server_url, HTTP_SERVER_URL_DEFAULT);
  strcpy(ota_info->http_server_ip, HTTP_SERVER_IP_DEFAULT);
  strcpy(ota_info->http_port, HTTP_SERVER_PORT_DEFAULT);
  strcpy(ota_info->file_name, OTA_UPDATE_FILE_NAME_DEFAULT);
  
  ota_info->update_region_type = OTA_1_APPLICATION;
 

}

esp_err_t meos_ota_update_read_partition_info(ota_update_info_t *update_info)
{
  const esp_partition_t *user_partition = NULL;
  esp_err_t ff_ret = 0;
  uint8_t ff_buffer[sizeof(ota_update_info_t)];
  memset(ff_buffer, 0 , sizeof(ota_update_info_t));
  user_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_COREDUMP, NULL); 
  if(user_partition != NULL)
  {
    ff_ret = esp_partition_read(user_partition, 0, ff_buffer, sizeof(ota_update_info_t));
    ESP_LOGI(OTA_TAG, "init read ret is %d ,data[15] is: %d \n", ff_ret, ff_buffer[0]);
    if(ff_ret == ESP_OK)
    {
      if(ff_buffer[0] == 0x55)
      {
        memcpy(update_info, ff_buffer, sizeof(ota_update_info_t));
      }
      else // have never been written 
      {
        return ESP_ERR_INVALID_RESPONSE;
      }
    }
  }  
  else
  {
    return ESP_ERR_NOT_FOUND;
  }

  return ESP_OK;
}

esp_err_t meos_ota_update_write_partition_info(ota_update_info_t *update_info)
{
  const esp_partition_t *user_partition = NULL;
  esp_err_t ff_ret = ESP_FAIL;

  user_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_COREDUMP, NULL); 
  if(user_partition != NULL)
  {  
    ff_ret = esp_partition_erase_range(user_partition, 0, user_partition->size);
    ESP_LOGI(OTA_TAG, "erase ret is %d", ff_ret);
    ff_ret = esp_partition_write(user_partition, 0, (void *)update_info, sizeof(ota_update_info_t));
    ESP_LOGI(OTA_TAG, "write ret is %d ,data[15] is: %d \n", ff_ret, update_info->magic);

    ota_update_info_t info_check;
    ff_ret = esp_partition_read(user_partition, 0, (void *)(&info_check), sizeof(ota_update_info_t));
    if(memcmp(update_info, &info_check,sizeof(ota_update_info_t)) == 0)
    {
      ff_ret = ESP_OK;
      ESP_LOGI(OTA_TAG, "infomation write succeed \n");
    }
    else
    {
      ff_ret = ESP_FAIL;
      ESP_LOGI(OTA_TAG, "infomation write failed \n");
    }
  }
  
  return ff_ret;
}

#if 0
#define DEFAULT_WIFI_SSID "Maker-office"
#define DEFAULT_WIFI_PASS "hulurobot423"

#define DEFAULT_SERVER_IP "10.0.1.212"
#define DEFAULT_SERVER_PORT "8070"
#define DEFAULT_FILENAME "ota_0.bin"
#define BUFFSIZE 1024
#define TEXT_BUFFSIZE 1024

static const char *TAG = "meos_ota";
/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };
/*an packet receive buffer*/
static char text[BUFFSIZE + 1] = { 0 };
/* an image total length*/
static int binary_file_length = 0;
/*socket id*/
static int socket_id = -1;
static char http_request[64] = {0};

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{

  switch(event->event_id)
  {
    case SYSTEM_EVENT_AP_START: 
         break;
		   
    case SYSTEM_EVENT_AP_STACONNECTED:
         xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
         // ESP_LOGI( TAG, "fftust: a sta connect this ap\n");
         break;
		   
    case SYSTEM_EVENT_AP_STADISCONNECTED:
         xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
         // ESP_LOGI( TAG, "fftust: a sta disconnect this ap\n");
         break;	  
		   
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
         // ESP_LOGI( TAG, "fftust: Receive probe request packet\n");
         break;  
		   
    default:
         break;
	}
	
	return ESP_OK;
	
}

static void initialise_wifi(void)
{
  tcpip_adapter_init();
  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  
  wifi_config_t wifi_config = 
  {
    .ap = 
    {
      .ssid = DEFAULT_WIFI_SSID,
      .password = DEFAULT_WIFI_PASS,
      .ssid_len = strlen(DEFAULT_WIFI_SSID),
      .authmode = WIFI_AUTH_WPA2_PSK,
      .channel = 6,
	  .max_connection = 2,
	  .beacon_interval = 100;
    },
  };
  ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.ap.ssid);
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_clear_fast_connect());
}

/*read buffer by byte still delim ,return read bytes counts*/
static int read_until(char *buffer, char delim, int len)
{
  /*TODO: delim check,buffer check,further: do an buffer length limited*/
  int i = 0;
  while (buffer[i] != delim && i < len)
  {
    ++i;
  }
  return i + 1;
}

/* resolve a packet from http socket
 * return true if packet including \r\n\r\n that means http packet header finished,start to receive packet body
 * otherwise return false
 * */
static bool read_past_http_header(char text[], int total_len, esp_ota_handle_t update_handle)
{
  /* i means current position */
  int i = 0, i_read_len = 0;
  while (text[i] != 0 && i < total_len)
  {
    i_read_len = read_until(&text[i], '\n', total_len);
    // if we resolve \r\n line,we think packet header is finished
    if(i_read_len == 2)
    {
      int i_write_len = total_len - (i + 2);
      memset(ota_write_data, 0, BUFFSIZE);
      /*copy first http packet body to write buffer*/
      memcpy(ota_write_data, &(text[i + 2]), i_write_len);

      esp_err_t err = esp_ota_write( update_handle, (const void *)ota_write_data, i_write_len);
      if(err != ESP_OK)
      {
        ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
        return false;
      }
      else
      {
        ESP_LOGI(TAG, "esp_ota_write header OK");
        binary_file_length += i_write_len;
      }
      return true;
    }
    i += i_read_len;
  }
  return false;
}

static bool connect_to_http_server()
{
  ESP_LOGI(TAG, "Server IP: %s Server Port:%s", DEFAULT_SERVER_IP, DEFAULT_SERVER_PORT);
  ESP_LOGI( TAG, http_request, "GET %s HTTP/1.1\r\nHost: %s:%s \r\n\r\n", DEFAULT_FILENAME, DEFAULT_SERVER_IP, DEFAULT_SERVER_PORT);

  int  http_connect_flag = -1;   
  struct sockaddr_in sock_info;

  socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_id == -1) 
  {
    ESP_LOGE(TAG, "Create socket failed!");
    return false;
  }

  // set connect info
  memset(&sock_info, 0, sizeof(struct sockaddr_in));
  sock_info.sin_family = AF_INET;
  sock_info.sin_addr.s_addr = inet_addr(DEFAULT_SERVER_IP);
  sock_info.sin_port = htons(atoi(DEFAULT_SERVER_PORT));

  // connect to http server
  http_connect_flag = connect(socket_id, (struct sockaddr *)&sock_info, sizeof(sock_info));
  if (http_connect_flag == -1)
  {
    ESP_LOGE(TAG, "Connect to server failed! errno=%d", errno);
    close(socket_id);
    return false;
  } 
  else
  {
    ESP_LOGI(TAG, "Connected to server");
    return true;
  }
  return false;
}

static void __attribute__((noreturn)) task_fatal_error()
{
  ESP_LOGE(TAG, "Exiting task due to fatal error...");
  close(socket_id);
  (void)vTaskDelete(NULL);

  while (1)
  {
    ;
  }
}

static void ota_example_task(void *pvParameter)
{
  esp_err_t err;
  /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
  esp_ota_handle_t update_handle = 0 ;
  const esp_partition_t *update_partition = NULL;

  ESP_LOGI(TAG, "Starting OTA example...");

  const esp_partition_t *configured = esp_ota_get_boot_partition();
  const esp_partition_t *running = esp_ota_get_running_partition();

  assert(configured == running); /* fresh from reset, should be running from configured boot partition */
  ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
           configured->type, configured->subtype, configured->address);

  /* Wait for the callback to set the CONNECTED_BIT in the
     event group.
  */
  xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
  ESP_LOGI(TAG, "Connect to Wifi ! Start to Connect to Server....");

  /*connect to http server*/
  if (connect_to_http_server())
  {
    ESP_LOGI(TAG, "Connected to http server");
  }
  else
  {
    ESP_LOGE(TAG, "Connect to http server failed!");
    task_fatal_error();
  }

  int res = -1;
  /*send GET request to http server*/
  res = send(socket_id, http_request, strlen(http_request), 0);
  if (res == -1) 
  {
    ESP_LOGE(TAG, "Send GET request to server failed");
    task_fatal_error();
  }
  else 
  {
    ESP_LOGI(TAG, "Send GET request to server succeeded");
  }

  //update_partition = esp_ota_get_next_update_partition(NULL);
  update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
  
  ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);
  assert(update_partition != NULL);

  err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
  if(err != ESP_OK)
  {
    ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);
    task_fatal_error();
  }
  ESP_LOGI(TAG, "esp_ota_begin succeeded");

  bool resp_body_start = false, flag = true;
  /*deal with all receive packet*/
  while (flag)
  {
    memset(text, 0, TEXT_BUFFSIZE);
    memset(ota_write_data, 0, BUFFSIZE);
    int buff_len = recv(socket_id, text, TEXT_BUFFSIZE, 0);
    if (buff_len < 0)
    { 
      /*receive error*/
      ESP_LOGE(TAG, "Error: receive data error! errno=%d", errno);
      task_fatal_error();
    } 
    else if(buff_len > 0 && !resp_body_start)
    {
      /*deal with response header*/
      memcpy(ota_write_data, text, buff_len);
      resp_body_start = read_past_http_header(text, buff_len, update_handle);
    } 
    else if(buff_len > 0 && resp_body_start) 
    { 
      /*deal with response body*/
      memcpy(ota_write_data, text, buff_len);
      err = esp_ota_write( update_handle, (const void *)ota_write_data, buff_len);
      if(err != ESP_OK)
      {
        ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
        task_fatal_error();
      }
      binary_file_length += buff_len;
      ESP_LOGI(TAG, "Have written image length %d", binary_file_length);
    }
    else if(buff_len == 0)
    { 
      /*packet over*/
      flag = false;
      ESP_LOGI(TAG, "Connection closed, all packets received");
      close(socket_id);
    }
    else 
    {
      ESP_LOGE(TAG, "Unexpected recv result");
    }
  }

  ESP_LOGI(TAG, "Total Write binary data length : %d", binary_file_length);

  if(esp_ota_end(update_handle) != ESP_OK) 
  {
    ESP_LOGE(TAG, "esp_ota_end failed!");
    task_fatal_error();
  }
  err = esp_ota_set_boot_partition(update_partition);
  if(err != ESP_OK) 
  {
    ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
    task_fatal_error();
  }
  ESP_LOGI(TAG, "Prepare to restart system!");
  esp_restart();
  return ;
}

void app_main()
{
  // Initialize NVS.
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES)
  {
    // OTA app partition table has a smaller NVS partition size than the non-OTA
    // partition table. This size mismatch may cause NVS initialization to fail.
    // If this happens, we erase NVS partition and initialize NVS again.
    const esp_partition_t* nvs_partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
    assert(nvs_partition && "partition table must have an NVS partition");
    ESP_ERROR_CHECK( esp_partition_erase_range(nvs_partition, 0, nvs_partition->size) );
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK( err );

  initialise_wifi();
  xTaskCreate(&ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);
}
#endif
