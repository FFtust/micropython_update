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

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"


#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"


#include "nvs.h"
#include "nvs_flash.h"

#include "meos_ota_update.h"

#define EXAMPLE_WIFI_SSID "Maker-office"//CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS "hulurobot423"//CONFIG_WIFI_PASSWORD
#define EXAMPLE_SERVER_IP "10.0.1.212"  //CONFIG_SERVER_IP
#define EXAMPLE_SERVER_PORT "8070" //CONFIG_SERVER_PORT
#define EXAMPLE_FILENAME "/ota_0.bin"//CONFIG_EXAMPLE_FILENAME

#define DEFAULT_WIFI_SSID "Maker-guest"
#define DEFAULT_WIFI_PASS "makeblock"
#define DEFAULT_WEB_SERVER "raw.githubusercontent.com" 
#define DEFAULT_WEB_URL "https://raw.githubusercontent.com/YanMinge/esp32_pro/master/micropython-esp32/esp32/Makefile" 

static const char *REQUEST = "GET " DEFAULT_WEB_URL " HTTP/1.0\r\n"
    "Host: "WEB_SERVER"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";


static volatile char meos_ota_wifi_ssid[32]= {0};
static volatile char meos_ota_wifi_pass[32]= {0};
static volatile char meos_ota_server_ip[32] = {0} ;
static volatile char meos_ota_server_url[128] = {0};
static volatile char meos_ota_server_port[8] = {0};
static volatile char ota_server_filename[32] = {0};


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

static void meos_ota_config(ota_update_info_t update_info)
{

  memcpy(meos_ota_wifi_pass, update_info.wifi_pass, strlen(update_info.wifi_pass)+1);
  memcpy(meos_ota_wifi_ssid, update_info.wifi_ssid, strlen(update_info.wifi_ssid)+1);

  memcpy(meos_ota_server_ip, update_info.http_server_ip, strlen(update_info.http_server_ip) + 1);
  memcpy(meos_ota_server_url, update_info.http_server_url, strlen(update_info.http_server_url) + 1);  
  memcpy(meos_ota_server_port, update_info.http_port, strlen(update_info.http_port) + 1);
  memcpy(ota_server_filename, update_info.file_name, strlen(update_info.file_name) + 1);

}


static void initialise_wifi(void)
{
  tcpip_adapter_init();
  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

 
  static wifi_config_t wifi_config;
  memcpy(wifi_config.sta.ssid, meos_ota_wifi_ssid, strlen(meos_ota_wifi_ssid)+1);
  memcpy(wifi_config.sta.password, meos_ota_wifi_pass, strlen(meos_ota_wifi_pass)+1); 

  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
  ESP_ERROR_CHECK( esp_wifi_start() );
}


/*************************************************************************
https update--------START
**************************************************************************/

static bool connect_to_https_server() 
{
  char buf[512];
  int ret, flags, len;
	
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ssl_context ssl;
  mbedtls_x509_crt cacert;
  mbedtls_ssl_config conf;
  mbedtls_net_context server_fd;
	
  mbedtls_ssl_init(&ssl);
  mbedtls_x509_crt_init(&cacert);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  ESP_LOGI(TAG, "Seeding the random number generator");
	
  mbedtls_ssl_config_init(&conf);
	
  mbedtls_entropy_init(&entropy);
  if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0)) != 0)
  {
    ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
    abort();
  }
	
  ESP_LOGI(TAG, "Loading the CA root certificate...");
	
  ret = mbedtls_x509_crt_parse(&cacert, server_root_cert_pem_start,
                                server_root_cert_pem_end-server_root_cert_pem_start);
	
  if(ret < 0)
  {
    ESP_LOGE(TAG, "mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
    abort();
  }
	
  ESP_LOGI(TAG, "Setting hostname for TLS session...");
  	
  /* Hostname set here should match CN in server certificate */
  if((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
  {
    ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
    abort();
  }
	
  ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");
	
  if((ret = mbedtls_ssl_config_defaults(&conf,
                                        MBEDTLS_SSL_IS_CLIENT,
                                        MBEDTLS_SSL_TRANSPORT_STREAM,
                                        MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
  {
    ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
    goto exit;
  }
	
  /* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
     a warning if CA verification fails but it will continue to connect.
	
     You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
  */
  mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
  mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
  mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
#ifdef CONFIG_MBEDTLS_DEBUG
  mbedtls_esp_enable_debug_log(&conf, 4);
#endif
	
  if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
  {
    ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
    goto exit;
  }
	
  while(1) 
  {
  /* Wait for the callback to set the CONNECTED_BIT in the
     event group.
  */
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");
  	
    mbedtls_net_init(&server_fd);
  	
    ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);
  	
    if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
                                    WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
      ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
      goto exit;
    }
  	
    ESP_LOGI(TAG, "Connected.");
	
    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
	
    ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");
	
    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
    {
      if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
      {
        ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
        goto exit;
      }
    }
   	
    ESP_LOGI(TAG, "Verifying peer X.509 certificate...");
	
    if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
    {
      /* In real life, we probably want to close connection if ret != 0 */
      ESP_LOGW(TAG, "Failed to verify peer certificate!");
      bzero(buf, sizeof(buf));
      mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
      ESP_LOGW(TAG, "verification info: %s", buf);
    }
    else 
    {
      ESP_LOGI(TAG, "Certificate verified.");
    }
	
    ESP_LOGI(TAG, "Writing HTTP request...");
	
    while((ret = mbedtls_ssl_write(&ssl, (const unsigned char *)REQUEST, strlen(REQUEST))) <= 0)
    {
      if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
      {
        ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
        goto exit;
      }
    }
	
    len = ret;
    ESP_LOGI(TAG, "%d bytes written", len);
    ESP_LOGI(TAG, "Reading HTTP response...");
  }
  
  exit:
      mbedtls_ssl_session_reset(&ssl);
      mbedtls_net_free(&server_fd);
  
      if(ret != 0)
      {
        mbedtls_strerror(ret, buf, 100);
        ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
      }
  
      for(int countdown = 10; countdown >= 0; countdown--) 
      {
        ESP_LOGI(TAG, "%d...", countdown);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
      ESP_LOGI(TAG, "Starting again!");

}

static void ota_for_https_task(void *pvParameter)
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


/*************************************************************************
https update--------end
**************************************************************************/



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
  //ESP_LOGI( TAG, http_request, "GET %s HTTP/1.1\r\nHost: %s:%s \r\n\r\n", EXAMPLE_FILENAME, EXAMPLE_SERVER_IP, EXAMPLE_SERVER_PORT);

  ESP_LOGI( TAG, http_request, "GET %s HTTP/1.1\r\nHost: %s:%s \r\n\r\n", ota_server_filename, meos_ota_server_ip, meos_ota_server_port);
  
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
  
  if((err_ota_partition == ESP_OK) /*&& (strcmp(s_ota_update_info.ota_current_version, s_ota_update_info.ota_update_version) < 0)*/)
  {
    meos_ota_config(s_ota_update_info);
    initialise_wifi();
    xTaskCreate(&ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);    

  }
  else if(err_ota_partition == ESP_OK)
  {
    ESP_LOGI(TAG, "the version is newest, donot need update\n");
    while(1)
    {
      ;
	}
  }
  else
  {
	ESP_LOGI(TAG, "ota partition not found \n");
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
