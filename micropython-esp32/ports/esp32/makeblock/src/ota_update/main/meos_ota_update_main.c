/* OTA example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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

#define EXAMPLE_WIFI_SSID "Maker-office"//CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS "hulurobot423"//CONFIG_WIFI_PASSWORD
#define EXAMPLE_SERVER_IP "10.0.1.212"  //CONFIG_SERVER_IP
#define EXAMPLE_SERVER_PORT "8070" //CONFIG_SERVER_PORT
#define EXAMPLE_FILENAME "/ota_0.bin"//CONFIG_EXAMPLE_FILENAME

#define BUFFSIZE 1024
#define TEXT_BUFFSIZE 1024

static const char *TAG = "ota";
static char ota_write_data[BUFFSIZE + 1] = { 0 };
static char text[BUFFSIZE + 1] = { 0 };
static int binary_file_length = 0;

static int socket_id = -1;
static char http_request[64] = {0};

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

static ota_update_info_t s_ota_update_info;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
  switch (event->event_id) 
  {
    case SYSTEM_EVENT_STA_START:
         esp_wifi_connect();
         break;
    case SYSTEM_EVENT_STA_GOT_IP:
         xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
         break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
         esp_wifi_connect();
         xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
         break;
    default:
         break;
  }
  return ESP_OK;
}


static void wifi_config(ota_update_info_t update_info)
{
  

}


static void initialise_wifi(void)
{
  tcpip_adapter_init();
  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  wifi_config_t wifi_config = 
  {
    .sta = 
    {
      .ssid = EXAMPLE_WIFI_SSID,
      .password = EXAMPLE_WIFI_PASS,
    },
  };
  ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
  ESP_ERROR_CHECK( esp_wifi_start() );
}


static int read_until(char *buffer, char delim, int len)
{
  int i = 0;
  while (buffer[i] != delim && i < len)
  {
    ++i;
  }
  return i + 1;
}

static bool read_past_http_header(char text[], int total_len, esp_ota_handle_t update_handle)
{
  int i = 0, i_read_len = 0;
  while (text[i] != 0 && i < total_len)
  {
    i_read_len = read_until(&text[i], '\n', total_len);
    // if we resolve \r\n line,we think packet header is finished
    if (i_read_len == 2) 
    {
      int i_write_len = total_len - (i + 2);
      memset(ota_write_data, 0, BUFFSIZE);
      memcpy(ota_write_data, &(text[i + 2]), i_write_len);

      esp_err_t err = esp_ota_write( update_handle, (const void *)ota_write_data, i_write_len);
      if (err != ESP_OK) 
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
  ESP_LOGI(TAG, "Server IP: %s Server Port:%s", EXAMPLE_SERVER_IP, EXAMPLE_SERVER_PORT);
  ESP_LOGI( TAG, http_request, "GET %s HTTP/1.1\r\nHost: %s:%s \r\n\r\n", EXAMPLE_FILENAME, EXAMPLE_SERVER_IP, EXAMPLE_SERVER_PORT);

  int http_connect_flag = -1;
  struct sockaddr_in sock_info;

  socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_id == -1) 
  {
    ESP_LOGE(TAG, "Create socket failed!");
    return false;
  }

  memset(&sock_info, 0, sizeof(struct sockaddr_in));
  sock_info.sin_family = AF_INET;
  sock_info.sin_addr.s_addr = inet_addr(EXAMPLE_SERVER_IP);
  sock_info.sin_port = htons(atoi(EXAMPLE_SERVER_PORT));

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
  
  assert(configured == running); 
  ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
           configured->type, configured->subtype, configured->address);
  
  xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
  ESP_LOGI(TAG, "Connect to Wifi ! Start to Connect to Server....");
  
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
  //update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP,  ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
  
  ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
           update_partition->subtype, update_partition->address);
  assert(update_partition != NULL);
  
  err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
  if (err != ESP_OK) 
  {
    ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);
    task_fatal_error();
  }
  ESP_LOGI(TAG, "esp_ota_begin succeeded");
  
  bool resp_body_start = false, flag = true;

  while (flag) 
  {
    memset(text, 0, TEXT_BUFFSIZE);
    memset(ota_write_data, 0, BUFFSIZE);
    int buff_len = recv(socket_id, text, TEXT_BUFFSIZE, 0);
    if (buff_len < 0) 
	{ 
      ESP_LOGE(TAG, "Error: receive data error! errno=%d", errno);
      task_fatal_error();
    } 
	else if (buff_len > 0 && !resp_body_start) 
	{ 
      memcpy(ota_write_data, text, buff_len);
      resp_body_start = read_past_http_header(text, buff_len, update_handle);
    }
    else if(buff_len > 0 && resp_body_start) 
    { 
      memcpy(ota_write_data, text, buff_len);
      err = esp_ota_write( update_handle, (const void *)ota_write_data, buff_len);
      if (err != ESP_OK) 
      {
        ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
        task_fatal_error();
      }
      binary_file_length += buff_len;
      ESP_LOGI(TAG, "Have written image length %d", binary_file_length);
    } 
    else if(buff_len == 0)
    { 
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
  
  if (esp_ota_end(update_handle) != ESP_OK) 
  {
    ESP_LOGE(TAG, "esp_ota_end failed!");
    task_fatal_error();
  }
  err = esp_ota_set_boot_partition(update_partition);
  if (err != ESP_OK) 
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
  esp_err_t err_ota_partition;
  esp_err_t err = nvs_flash_init();

  if (err == ESP_ERR_NVS_NO_FREE_PAGES) 
  {
    const esp_partition_t* nvs_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
    assert(nvs_partition && "partition table must have an NVS partition");
    ESP_ERROR_CHECK( esp_partition_erase_range(nvs_partition, 0, nvs_partition->size) );
	
    err = nvs_flash_init();
  }
  
  ESP_ERROR_CHECK( err );

  
  err_ota_partition = meos_ota_update_read_partition_info(&s_ota_update_info); 
  ESP_LOGI(TAG, "the err_ota_partition is:%d\n", err_ota_partition);
  
  if((err_ota_partition == ESP_OK) && (strcmp(s_ota_update_info.ota_current_version, s_ota_update_info.ota_update_version) < 0))
  {
    
    initialise_wifi();
    xTaskCreate(&ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);    

  }
  else
  {
    ESP_LOGI(TAG, "the version is newest, donot need update\n");
    while(1)
    {
      ;
	}
  }

  
  //meos_ota_update_set_default_partition_info(&s_ota_update_info);
  //meos_ota_update_write_partition_info(&s_ota_update_info);

  
  //initialise_wifi();
  //xTaskCreate(&ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);
}
