/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     
 * @file      codey_void.c
 * @author    Leo lu
 * @version    V1.0.0
 * @date      2017/08/31
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
 * Leo lu             2017/08/31      1.0.0              Initial version
 * Leo lu             2017/10/30      1.0.1              1 ) Fix a bug of division by zero
 *                                                       2 ) Fix a bug of memory leak
 * Leo lu             2017/12/27      1.0.2              1 ) Fia a bug in play to stop, considering play buffer duration time
 * </pre>
 *
 */
  
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "codey_utils.h"
#include "codey_sys.h"
#include "soc/rtc_io_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "rtc_io.h"
#include "adc.h"
#include "dac.h"
#include "timer.h"
#include "sys/lock.h"
#include "driver/rtc_cntl.h"
#include "codey_voice.h"
#include "extmod/vfs_fat.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "mpthread.h"
#include "py/mphal.h"

/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/
#undef    TAG
#define   TAG                         ("VOICE")

#define   MAX_SIMUL_MUSIC             (8)
#define   MUSIC_TASK_LOOP_TIME_MS     (10)
#define   MUSIC_WAIT_TIME_MS          (10)
#define   TIMER_RUN_FREQ              (5000000)
#define   USING_TIMER_GROUP           (TIMER_GROUP_0)
#define   USING_TIMER_NUM             (TIMER_1)
#define   MUSIC_TASK_STACK_LEN        (2048 + 1024)
#define   MUSIC_TASK_PRIORITY         (ESP_TASK_PRIO_MIN + 2)
#define   MUSICE_DAC_CHN              (DAC_CHANNEL_1)
#define   MAX_WAV_HEAD_LENGTH         (128)
#define   MUSIC_ISR_BLOCK_NUM         (2)
#define   MUSIC_ISR_BLOCK_BUF_SIZE    (1024)
#define   MUSIC_DROP_STEP             (1)

#define   MUSIC_TAKE_PLAY_LIST_SEM    do\
                                      {\
                                        xSemaphoreTake(s_music_file_list_sem, portMAX_DELAY);\
                                        /* ESP_LOGI(TAG, "%s Taking play list sem", __FUNCTION__); */\
                                      }while(0)
#define   MUSIC_GIVE_PLAY_LIST_SEM    do\
                                      {\
                                        /* ESP_LOGI(TAG, "%s Give play list sem\r\n", __FUNCTION__); */\
                                        xSemaphoreGive(s_music_file_list_sem);\
                                      }while(0)

#define   GET_MAX_ALIGN_READ_SIZE(min_align_block, buf_size)  ((buf_size/min_align_block) * min_align_block)

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/ 
typedef struct
{
  uint32_t  freq;
  uint32_t  block_cnt;
  uint32_t  block_idx;
  uint8_t   block_buf[MUSIC_ISR_BLOCK_BUF_SIZE];
}wav_play_block_t;

typedef struct
{
  uint32_t          in_block_idx;
  uint32_t          out_block_idx;
  wav_play_block_t  block[MUSIC_ISR_BLOCK_NUM];
}wav_play_buf_t;

extern SemaphoreHandle_t g_fatfs_sema;

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
static uint32_t get_wav_id(void);
static uint32_t get_wav_file_lcm_freq(codey_wav_info_t *header);  
static uint32_t get_min_align_block(codey_wav_info_t *header);
static void remove_finish_voice_node(codey_wav_info_t **header);
static void clean_play_list(codey_wav_info_t **header);
static uint32_t move_music_to_block_buffer(uint32_t block_idx);
static void codey_music_play_main_loop(void * parameter);
static void touch_music_play_action(void);
static void add_wav_to_play_list(codey_wav_info_t **header, codey_wav_info_t *node);
static bool reach_max_simul_music(codey_wav_info_t *header);
static void remove_wav_from_play_list(codey_wav_info_t **header, codey_wav_info_t *node);
static uint32_t get_wav_play_list_cnt(codey_wav_info_t *header);
static void IRAM_ATTR music_dac_out_voltage(uint8_t dac_value);
static void IRAM_ATTR music_timer_isr(void *para);
static void music_timer_init(uint32_t freq);
static void start_play_music(void);

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
static timg_dev_t *s_timer;
static bool s_is_init = false;
static uint32_t s_wav_id = 0;
static SemaphoreHandle_t s_wav_id_sem;
static codey_wav_info_t *s_music_data_header = NULL;
static bool s_is_music_on_play = false;
static SemaphoreHandle_t s_wav_buf_sem;
static SemaphoreHandle_t s_music_file_list_sem;
static wav_play_buf_t s_wav_play_block_buffer;
static uint8_t s_read_file_buf[MUSIC_ISR_BLOCK_BUF_SIZE + 4];
static TaskHandle_t s_music_play_task_handle;
static bool s_change_block = false;
/* add by fftust to set volume */
/* date type of float shoule not be in ISR */
static uint8_t s_volume_div = 100; 
static float s_current_volume = 100.0; 

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/ 
bool codey_voice_init(void)
{
  memset(&s_wav_play_block_buffer, 0, sizeof(wav_play_buf_t));
  
  dac_output_enable(MUSICE_DAC_CHN);

  s_wav_buf_sem = xSemaphoreCreateCounting(1, 0);
  
  s_music_file_list_sem = xSemaphoreCreateMutex();
  
  s_wav_id_sem = xSemaphoreCreateMutex();

  s_wav_id = 0;

  s_is_init = true;

  xTaskCreatePinnedToCore( codey_music_play_main_loop, 
                            "music_task", 
                            MUSIC_TASK_STACK_LEN, 
                            NULL, 
                            MUSIC_TASK_PRIORITY, 
                            &s_music_play_task_handle, 
                            0 
                         );

  return true;
}

bool codey_void_deinit(void)
{
  if(s_is_init)
  {
    codey_voice_stop_play();

    vTaskDelete(s_music_play_task_handle);
    
    dac_output_disable(MUSICE_DAC_CHN);

    vSemaphoreDelete(s_wav_buf_sem);

    vSemaphoreDelete(s_music_file_list_sem);

    vSemaphoreDelete(s_wav_id_sem);

    s_is_init = false;
  }

  return true;
}

bool codey_voice_deinit(void)
{
  vTaskDelete(s_music_play_task_handle);
  s_is_init = false;
  return true;
}

/* add by fftust to set volume */
void codey_voice_set_volume_t(float volume)
{
  if(volume > 100)
  {
    volume = 100;
  }
  else if(volume < 0)
  {
    volume = 0;
  }
  s_current_volume = volume;
  s_volume_div = (uint8_t)s_current_volume;
}

/* add by fftust to set volume */
void codey_voice_change_volume_t(float volume)
{
  float vol = s_current_volume;
  vol += volume; 
  
  if(vol > 100)
  {
    vol = 100;
  }
  else if(vol < 0)
  {
    vol = 0;
  }
  s_current_volume = vol;
  s_volume_div = (uint8_t)s_current_volume;
}

/* add by fftust to set volume */
float codey_voice_get_volume_t()
{
  return (float)(s_current_volume);
}

bool codey_voice_parse_wav_file(codey_wav_info_t *wav_info, uint8_t *data_buffer)
{
  bool result = false;
  
  memcpy(wav_info->wav_head, data_buffer, 4);
  wav_info->wav_head_length = 0;
  if(strcmp("RIFF", wav_info->wav_head) == 0)
  { 
    wav_info->wav_head_length += 4;
    wav_info->file_length = codey_read_long(data_buffer, wav_info->wav_head_length);
    wav_info->wav_head_length += 4;
    
    memcpy(wav_info->wav_format_desc, data_buffer + wav_info->wav_head_length, 8);
    if(strcmp("WAVEfmt ", wav_info->wav_format_desc) == 0)
    {
      wav_info->wav_head_length += 8;
      wav_info->pcm_wav_format_length = codey_read_long(data_buffer, wav_info->wav_head_length);
      wav_info->wav_head_length += 4;
      wav_info->encode_format_type = codey_read_short(data_buffer, wav_info->wav_head_length);
      wav_info->wav_head_length += 2;
      wav_info->number_of_channels = codey_read_short(data_buffer, wav_info->wav_head_length);
      wav_info->wav_head_length += 2;
      wav_info->sampling_frequency = codey_read_long(data_buffer, wav_info->wav_head_length);
      wav_info->wav_head_length += 4;
      wav_info->data_transferred_per_second = codey_read_long(data_buffer, wav_info->wav_head_length);
      wav_info->wav_head_length += 4;
      wav_info->number_of_data_blocks = codey_read_short(data_buffer, wav_info->wav_head_length);
      wav_info->wav_head_length += 2;
      wav_info->number_of_bits_per_sample = codey_read_short(data_buffer, wav_info->wav_head_length);
      wav_info->wav_head_length += 2;
      if(wav_info->pcm_wav_format_length == 0x12)
      {
        wav_info->additional_optional_information = codey_read_short(data_buffer, wav_info->wav_head_length);
        wav_info->wav_head_length += 2;
      }
      memcpy(wav_info->fact_optional_desc, data_buffer + wav_info->wav_head_length, 4);
      if(strcmp("Fact", wav_info->fact_optional_desc) == 0) 
      {
        wav_info->wav_head_length += 4;
        wav_info->fact_data_length = codey_read_long(data_buffer, wav_info->wav_head_length);
        wav_info->wav_head_length += 4;
        if(wav_info->fact_data_length > 8)
        {
          memcpy(wav_info->fact_data, data_buffer + wav_info->wav_head_length, 8);
        }
        else
        {
          memcpy(wav_info->fact_data, data_buffer + wav_info->wav_head_length, wav_info->fact_data_length);
        }
        wav_info->wav_head_length += wav_info->fact_data_length;
      }
      memcpy(wav_info->data_desc, data_buffer+wav_info->wav_head_length, 4);
      if(strcmp("data", wav_info->data_desc) == 0)
      {
        wav_info->wav_head_length += 4;
        wav_info->wav_data_length = codey_read_long(data_buffer, wav_info->wav_head_length);
        wav_info->wav_head_length += 4;
        wav_info->read_offset = wav_info->wav_head_length;
        result = true;
      }
      else
      {
        result = false;
      }
    }
    else
    {
      result = false;
    }
  }
  else
  {
    result = false;
  }

  if(0 == wav_info->sampling_frequency)
  {
    result = false;
  }
  
  return result;
}

bool codey_voice_play_file(const char *file_name)
{
  bool ret;
  uint8_t music_head[MAX_WAV_HEAD_LENGTH];
  codey_wav_info_t *wav_info;
  uint8_t *path, *name;
  UINT n_music;
  uint32_t remain_cnt;

  if(!s_is_init)
  {
    if(!codey_voice_init())
    {
      return false;
    }
  }

  if(reach_max_simul_music(s_music_data_header))
  {
    return false;
  }

  path = codey_get_path_from_full_file_name((uint8_t *)file_name);
  name = codey_get_name_from_full_file_name((uint8_t *)file_name);
  if((!path) || (!name))
  {
    if(path)
    {
      free(path);
    }
    if(name)
    {
      free(name);
    }
    return false;
  }

  wav_info = malloc(sizeof(codey_wav_info_t));
  if(!wav_info)
  {
    free(path);
    free(name);
    return false;
  }

  memset(wav_info, 0, sizeof(codey_wav_info_t));
  if(xSemaphoreTake(g_fatfs_sema, portMAX_DELAY) == pdTRUE)
  {
    if(FR_OK == f_chdir(&codey_sflash_vfs_fat->fatfs, (const char *)path)) 
    {
      if(FR_OK == f_open(&codey_sflash_vfs_fat->fatfs, &(wav_info->fp_music), (const char *)name, FA_READ))
      {
        wav_info->is_file_open = true;
        if(FR_OK == f_read(&(wav_info->fp_music), music_head, MAX_WAV_HEAD_LENGTH, &n_music))
        {
          if(codey_voice_parse_wav_file(wav_info, music_head))
          {
            // codey_voice_print_wav_info(wav_info);
            wav_info->music_type = FILE_MUSIC;
            wav_info->start_play = false;
            wav_info->wav_id = get_wav_id();
            MUSIC_TAKE_PLAY_LIST_SEM;
            add_wav_to_play_list(&s_music_data_header, wav_info);
            MUSIC_GIVE_PLAY_LIST_SEM;
            remain_cnt = wav_info->wav_head_length + wav_info->wav_data_length - wav_info->read_offset;
            wav_info->play_duration_time_ms = remain_cnt * 1000 / wav_info->sampling_frequency;
            ret = true;
          }
          else
          {
            f_close(&(wav_info->fp_music));
            free(wav_info);
            ret = false;
          }
        }
        else
        {
          f_close(&(wav_info->fp_music));;
          free(wav_info);
          ret = false;
        }
      }
      else
      {
        free(wav_info);
        ret = false;
      }
    }
    else
    {
      free(wav_info);
      ret = false;
    }
    xSemaphoreGive(g_fatfs_sema);
  }
  else
  {
    ret = false;
  }
  free(path);
  free(name);
  
  return ret;
}

bool codey_voice_play_file_to_stop(const char *file_name)
{
  bool ret;
  uint8_t music_head[MAX_WAV_HEAD_LENGTH];
  codey_wav_info_t *wav_info;
  uint8_t *path, *name;
  UINT n_music;
  uint32_t remain_cnt;

  if(!s_is_init)
  {
    if(!codey_voice_init())
    {
      return false;
    }
  }

  if(reach_max_simul_music(s_music_data_header))
  {
    return false;
  }

  path = codey_get_path_from_full_file_name((uint8_t *)file_name);
  name = codey_get_name_from_full_file_name((uint8_t *)file_name);
  if((!path) || (!name))
  {
    if(path)
    {
      free(path);
    }
    if(name)
    {
      free(name);
    }
    return false;
  }

  wav_info = malloc(sizeof(codey_wav_info_t));
  if(!wav_info)
  {
    free(path);
    free(name);
    return false;
  }

  memset(wav_info, 0, sizeof(codey_wav_info_t));
  if(xSemaphoreTake(g_fatfs_sema, portMAX_DELAY) == pdTRUE)
  {
    if(FR_OK == f_chdir(&codey_sflash_vfs_fat->fatfs, (const char *)path)) 
    {
      if(FR_OK == f_open(&codey_sflash_vfs_fat->fatfs, &(wav_info->fp_music), (const char *)name, FA_READ))
      {
        wav_info->is_file_open = true;
        if(FR_OK == f_read(&(wav_info->fp_music), music_head, MAX_WAV_HEAD_LENGTH, &n_music))
        {
          if(codey_voice_parse_wav_file(wav_info, music_head))
          {
            //codey_voice_print_wav_info(wav_info);
            wav_info->music_type = FILE_MUSIC;
            wav_info->wav_id = get_wav_id();
            wav_info->start_play = false;
            remain_cnt = wav_info->wav_head_length + wav_info->wav_data_length - wav_info->read_offset;
            wav_info->play_duration_time_ms = remain_cnt * 1000 / wav_info->sampling_frequency;
            MUSIC_TAKE_PLAY_LIST_SEM;
            add_wav_to_play_list(&s_music_data_header, wav_info);
            MUSIC_GIVE_PLAY_LIST_SEM;
            ret = true;
          }
          else
          {
            f_close(&(wav_info->fp_music));
            free(wav_info);
            ret = false;
          }
        }
        else
        {
          f_close(&(wav_info->fp_music));
          free(wav_info);
          ret = false;
        }
      }
      else
      {
        free(wav_info);
        ret = false;
      }
    }
    else
    {
      free(wav_info);
      ret = false;
    }
    xSemaphoreGive(g_fatfs_sema);
  }
  else
  {
    ret  = false;
  }

  free(path);
  free(name);
  if(true == ret)
  {
    // wait for finisth
    while(codey_voice_find_wav_by_id(wav_info->wav_id))
    {
      vTaskDelay(MUSIC_WAIT_TIME_MS / portTICK_PERIOD_MS);
    }
  }
  
  return ret;
}

bool codey_voice_play_note(uint32_t freq, uint32_t time_ms, codey_voice_note_effect_t voice_eff, uint8_t louder)
{
  codey_wav_info_t *wav_info;

  if(!time_ms && !freq)
  {
    return false;
  }

  if(!s_is_init)
  {
    if(!codey_voice_init())
    {
      return false;
    }
  }

  if(reach_max_simul_music(s_music_data_header))
  {
    return false;
  }

  wav_info = malloc(sizeof(codey_wav_info_t));
  if(!wav_info)
  {
    return false;
  }
  
  memset(wav_info, 0, sizeof(codey_wav_info_t));
  wav_info->louder = louder;
  wav_info->music_type = NOTE_MUSIC;
  wav_info->note_freq = freq * 2;
  wav_info->note_duration_ms = time_ms;
  wav_info->start_play = false;
  wav_info->wav_id = get_wav_id();
  wav_info->play_duration_time_ms = time_ms;
  MUSIC_TAKE_PLAY_LIST_SEM;
  add_wav_to_play_list(&s_music_data_header, wav_info);
  MUSIC_GIVE_PLAY_LIST_SEM;
  
  return true;
}

bool codey_voice_play_note_to_stop(uint32_t freq, uint32_t time_ms, codey_voice_note_effect_t voice_eff, uint8_t louder)
{
  codey_wav_info_t *wav_info;

  if(!time_ms && !freq)
  {
    return false;
  }

  if(!s_is_init)
  {
    if(!codey_voice_init())
    {
      return false;
    }
  }

  if(reach_max_simul_music(s_music_data_header))
  {
    return false;
  }

  wav_info = malloc(sizeof(codey_wav_info_t));
  if(!wav_info)
  {
    return false;
  }
  
  memset(wav_info, 0, sizeof(codey_wav_info_t));
  wav_info->louder = louder;
  wav_info->music_type = NOTE_MUSIC;
  wav_info->note_freq = freq * 2;
  wav_info->note_duration_ms = time_ms;
  wav_info->wav_id = get_wav_id();
  wav_info->start_play = false;
  wav_info->play_duration_time_ms = time_ms;
  MUSIC_TAKE_PLAY_LIST_SEM;
  add_wav_to_play_list(&s_music_data_header, wav_info);
  MUSIC_GIVE_PLAY_LIST_SEM;

  // wait for finisth
  while(codey_voice_find_wav_by_id(wav_info->wav_id))
  {
    vTaskDelay(MUSIC_WAIT_TIME_MS / portTICK_PERIOD_MS);
  }
  return true;
}

void codey_voice_print_wav_info(const codey_wav_info_t *wav_info)
{
  // ESP_LOGI(TAG, "%s", FILE_MUSIC == wav_info->music_type?"WAV_FILE":"MUSIC_NOTE");
  // ESP_LOGI(TAG, "sampling_frequency: %d",               wav_info->sampling_frequency);
  // ESP_LOGI(TAG, "number_of_channels: %d",               wav_info->number_of_channels);
  // ESP_LOGI(TAG, "data_transferred_per_second: %d",      wav_info->data_transferred_per_second);
  // ESP_LOGI(TAG, "wav_data_length: %d",                  wav_info->wav_data_length);
}

void codey_voice_traverse_music_file(void)
{
  codey_wav_info_t *node;

  node = s_music_data_header;
  while(node)
  {
    codey_voice_print_wav_info(node);
    node = node->next;
  }
}

void codey_voice_remove_nth_file(uint32_t idx)
{
  uint32_t i;
  codey_wav_info_t *node;

  MUSIC_TAKE_PLAY_LIST_SEM;
  node = s_music_data_header;
  i = 0;
  while(node)
  {
    if(i == idx)
    {
      remove_wav_from_play_list(&s_music_data_header, node);
    }
    node = node->next;
    i++;
  }
  MUSIC_GIVE_PLAY_LIST_SEM;
}

void codey_voice_stop_play(void)
{
  // ESP_LOGI(TAG, "codey_voice_stop_play");
  timer_pause(USING_TIMER_GROUP, USING_TIMER_NUM);
  if(s_music_data_header != NULL)
  {
    clean_play_list(&s_music_data_header);
  }
  s_is_music_on_play = false;
}

codey_wav_info_t *codey_voice_find_wav_by_id(uint32_t wav_id)
{
  codey_wav_info_t *ret, *node;

  ret = NULL;
  MUSIC_TAKE_PLAY_LIST_SEM;
  node = s_music_data_header;
  while(node)
  {
    if(wav_id == node->wav_id)
    {
      ret = node;
      break;
    }
    else
    {
      node = node->next;
    }
  }
  MUSIC_GIVE_PLAY_LIST_SEM;
  return ret;
}

/******************************************************************************
 DEFINITION PRIVATE FUNCTIONS
 ******************************************************************************/
static uint32_t get_wav_id(void)
{
  uint32_t id;
  xSemaphoreTake(s_wav_id_sem, portMAX_DELAY);
  id = s_wav_id++;
  xSemaphoreGive(s_wav_id_sem);
  return id;
}

static uint32_t get_wav_file_lcm_freq(codey_wav_info_t *header)
{
  uint32_t lcm_freq;
  codey_wav_info_t *node;

  lcm_freq = 1;
  node = header;
  while(node)
  {
    if(FILE_MUSIC == node->music_type)
    {
      lcm_freq = lcm(lcm_freq, node->sampling_frequency);
    }
    node = node->next;
  }

  return lcm_freq;
}

static uint32_t get_min_align_block(codey_wav_info_t *header)
{
  uint32_t align_block;
  codey_wav_info_t *node;
  uint8_t step;
  uint32_t lcm_freq;

  lcm_freq = get_wav_file_lcm_freq(header);
  align_block = 1;
  node = header;
  
  while(node)
  {
    if(FILE_MUSIC == node->music_type)
    {
      step = lcm_freq/node->sampling_frequency;
      align_block = lcm(align_block, step);
    }
    node = node->next;
  }
  
  return align_block;
}

static void remove_finish_voice_node(codey_wav_info_t **header)
{
  uint32_t remain_cnt;
  codey_wav_info_t *node, *next_node;

  MUSIC_TAKE_PLAY_LIST_SEM;
  node = *header;
  while(node)
  {
    next_node = node->next;

    if(FILE_MUSIC == node->music_type)
    {
      remain_cnt = node->wav_head_length + node->wav_data_length - node->read_offset;
      if(!remain_cnt && !node->drop_tail_voice)
      {
        if ( millis() > node->play_buffer_stop_tick )
        {
          remove_wav_from_play_list(header, node);
          free(node);
        }
      }
    }
    else
    {
      if(0 == node->note_duration_ms)
      {
        if ( millis() > node->play_buffer_stop_tick )
        {
          remove_wav_from_play_list(header, node);
          free(node);  
        }
      }
    }
   
    node = next_node;
  }  
  MUSIC_GIVE_PLAY_LIST_SEM;
}

static void clean_play_list(codey_wav_info_t **header)
{
  codey_wav_info_t *node, *next_node;

  if(!header)
  {
    return;
  }

  MUSIC_TAKE_PLAY_LIST_SEM;
  node = *header;
  while(node)
  {
    next_node = node->next;
    remove_wav_from_play_list(header, node);
    free(node);
    node = next_node;
  }  
  MUSIC_GIVE_PLAY_LIST_SEM;
}

static uint32_t move_music_to_block_buffer(uint32_t block_idx)
{
  int i, j;
  int file_cnt;
  uint32_t mix_cnt;
  codey_wav_info_t *node;
  uint32_t remain_cnt;
  uint32_t read_cnt;
  UINT n_music;
  uint8_t *p;
  uint32_t lcm_freq;
  uint32_t max_align_read_cnt;
  uint32_t step;
  int step_co;
  bool music, note;
  
  p = s_wav_play_block_buffer.block[block_idx].block_buf;
  s_wav_play_block_buffer.block[block_idx].block_cnt = 0;
  s_wav_play_block_buffer.block[block_idx].block_idx = 0;

  if(xSemaphoreTake(g_fatfs_sema, portMAX_DELAY) == pdTRUE)
  {
    MUSIC_TAKE_PLAY_LIST_SEM;
    mix_cnt = get_wav_play_list_cnt(s_music_data_header);
    lcm_freq = get_wav_file_lcm_freq(s_music_data_header);

    memset(p, 0, MUSIC_ISR_BLOCK_BUF_SIZE);
    file_cnt = 0;
    music = note = false;
    max_align_read_cnt = GET_MAX_ALIGN_READ_SIZE(get_min_align_block(s_music_data_header), MUSIC_ISR_BLOCK_BUF_SIZE);
    node = s_music_data_header;
    while(node)
    {
      if(FILE_MUSIC == node->music_type)
      {
        if(note)
        {
          node = node->next;
          file_cnt++;
          continue;
        }

        if(!mix_cnt)
        {
          node = node->next;
          file_cnt++;
          continue;
        }
        
        s_wav_play_block_buffer.block[block_idx].freq = lcm_freq;
        read_cnt = 0;
        n_music = 0;
        
        step = lcm_freq/node->sampling_frequency;
        if(0 == step)
        {
          step = 1;
        }
        remain_cnt = node->wav_head_length + node->wav_data_length - node->read_offset;
        if(remain_cnt)
        {
          music = true;
          if(remain_cnt <= (max_align_read_cnt/step))
          {
            read_cnt = remain_cnt;
          }
          else
          {
            read_cnt = (max_align_read_cnt/step);
          }

          // read another more 4 bytes, for calculate the step_co of this block to another block
          f_lseek(&(node->fp_music), node->read_offset);
          f_read(&(node->fp_music), s_read_file_buf, read_cnt + 4, &n_music);

          node->read_offset += read_cnt;
          if(node->read_offset == node->wav_head_length + node->wav_data_length)
          {
            node->drop_tail_voice = s_read_file_buf[read_cnt-1]/mix_cnt;
            if(0 == node->drop_tail_voice)
            {
              node->drop_step = 0;
            }
            else
            {
              node->drop_step = lcm_freq/node->drop_tail_voice/3;
            }          
            node->drop_step_cnt = 0;

            for(i = 0; i < read_cnt - 1; i++)
            {
              if(s_read_file_buf[i+1] >= s_read_file_buf[i])
              {
                step_co = (s_read_file_buf[i+1] - s_read_file_buf[i])/step;
                for(j = 0; j < step; j++)
                {
                  p[i*step + j] += (s_read_file_buf[i] + step_co * j)/mix_cnt;
                }
              }
              else
              {
                step_co = (s_read_file_buf[i] - s_read_file_buf[i+1])/step;
                for(j = 0; j < step; j++)
                {
                  p[i*step + j] += (s_read_file_buf[i] - step_co * j)/mix_cnt;
                }
              }
            }
            
            // The last step deal with specifically
            p[i*step] += s_read_file_buf[i]/mix_cnt;

            // deal with the drop tail
            for(i = i*step + 1; i < max_align_read_cnt; i++)
            {
              if(node->drop_tail_voice)
              {
                p[i] += node->drop_tail_voice;
                node->drop_step_cnt++;
                if(node->drop_step_cnt == node->drop_step)
                {
                  node->drop_tail_voice--;
                  node->drop_step_cnt = 0;
                }
              }
              else
              {
                break;
              }            
            }
          }
          else
          {
            for(i = 0; i < read_cnt; i++)
            {
              if(s_read_file_buf[i+1] >= s_read_file_buf[i])
              {
                step_co = (s_read_file_buf[i+1] - s_read_file_buf[i])/step;
                for(j = 0; j < step; j++)
                {
                  p[i*step + j] += (s_read_file_buf[i] + step_co * j)/mix_cnt;
                }
              }
              else
              {
                step_co = (s_read_file_buf[i] - s_read_file_buf[i+1])/step;
                for(j = 0; j < step; j++)
                {
                  p[i*step + j] += (s_read_file_buf[i] - step_co * j)/mix_cnt;
                }
              }
            }

            // update the in block info
            if(s_wav_play_block_buffer.block[s_wav_play_block_buffer.in_block_idx].block_cnt < read_cnt*step)
            {
              s_wav_play_block_buffer.block[s_wav_play_block_buffer.in_block_idx].block_cnt = read_cnt*step;
            }
          }
        }
        else
        {
          // deal with the drop tail
          if(node->drop_tail_voice)
          {
            for(i = 0; i < max_align_read_cnt; i++)
            {
              if(node->drop_tail_voice)
              {
                p[i] += node->drop_tail_voice;
                node->drop_step_cnt++;
                if(node->drop_step_cnt == node->drop_step)
                {
                  node->drop_tail_voice--;
                  node->drop_step_cnt = 0;
                }
              }
              else
              {
                break;
              }            
            }
            if(s_wav_play_block_buffer.block[s_wav_play_block_buffer.in_block_idx].block_cnt < i)
            {
              s_wav_play_block_buffer.block[s_wav_play_block_buffer.in_block_idx].block_cnt = i;
            }
          }
        }

        /*
        Calculate the stop tick
        */
        if(!node->start_play)
        {
          node->play_buffer_stop_tick = millis() + node->play_duration_time_ms;
          node->start_play = true;
        }
      }
      else
      {      
        if(music || note)
        {
          node = node->next;
          file_cnt++;
          continue;
        }

        lcm_freq = node->note_freq;
        s_wav_play_block_buffer.block[block_idx].freq = lcm_freq;
        read_cnt = 0;
        n_music = 0;
        max_align_read_cnt = (MUSIC_ISR_BLOCK_BUF_SIZE * node->note_freq)/lcm_freq;
        step = lcm_freq/node->note_freq;
        if(0 == step)
        {
          step = 1;
        }
        remain_cnt = (node->note_duration_ms * node->note_freq)/1000;
        if(remain_cnt)
        {
          note = true;
          if(remain_cnt <= max_align_read_cnt)
          {
            read_cnt = remain_cnt;
          }
          else
          {
            read_cnt = max_align_read_cnt;
          }

          for(i = 0; i < read_cnt; i++)
          {
            if(0 == i%2)
            {
              for(j = 0; j < step; j++)
              {
                p[i*step + j] += 0;
              }
            }
            else
            {
              for(j = 0; j < step; j++)
              {
                p[i*step + j] += (0xFF*node->louder/(100));
              }
            }
          }

          // update the in block info
          if(s_wav_play_block_buffer.block[s_wav_play_block_buffer.in_block_idx].block_cnt < read_cnt*step)
          {
            s_wav_play_block_buffer.block[s_wav_play_block_buffer.in_block_idx].block_cnt = read_cnt*step;
          }

          if((read_cnt * 1000)/node->note_freq)
          {
            node->note_duration_ms -= (read_cnt * 1000)/node->note_freq;
          }
          else
          {
            node->note_duration_ms = 0;
          }
        }
        else
        {
          node->note_duration_ms = 0;
        }

        /*
        Calculate the stop tick
        */
        if(!node->start_play)
        {
          node->play_buffer_stop_tick = millis() + node->play_duration_time_ms;
          node->start_play = true;
        }

      }
      node = node->next;
      file_cnt++;
    } 
    MUSIC_GIVE_PLAY_LIST_SEM;
  }
  xSemaphoreGive(g_fatfs_sema);  

  return s_wav_play_block_buffer.block[s_wav_play_block_buffer.in_block_idx].block_cnt;
}

static void touch_music_play_action(void)
{
  if(!s_is_music_on_play)
  {
    // ESP_LOGI(TAG, "Try to move music data to play buffer");
    s_wav_play_block_buffer.in_block_idx = 0;
    s_wav_play_block_buffer.out_block_idx = 1;
    if(move_music_to_block_buffer(s_wav_play_block_buffer.in_block_idx))
    {
      s_wav_play_block_buffer.in_block_idx = 1;
      s_wav_play_block_buffer.out_block_idx = 0;
      move_music_to_block_buffer(s_wav_play_block_buffer.in_block_idx);
      start_play_music();
    }
  }
  else
  {
    // ESP_LOGI(TAG, "music is on playing");
  }
}

static void add_wav_to_play_list(codey_wav_info_t **header, codey_wav_info_t *node)
{
  codey_wav_info_t *p;

  codey_voice_print_wav_info(node);
  
  if(!(*header))
  {
    *header = node;
  }
  else
  {
    p = *header;
    while(p->next)
    {
      p = p->next;
    }
    p->next = node;
  }
}

static bool reach_max_simul_music(codey_wav_info_t *header)
{
  bool ret;
  uint32_t cnt;

  ret = false;
  cnt = 0;

  MUSIC_TAKE_PLAY_LIST_SEM;
  while(header)
  {
    cnt++;
    if(cnt >= MAX_SIMUL_MUSIC)
    {
      ret = true;
      break;
    }
    else
    {
      header = header->next;
    }
  }
  MUSIC_GIVE_PLAY_LIST_SEM;
  
  return ret;
}

static void remove_wav_from_play_list(codey_wav_info_t **header, codey_wav_info_t *node)
{
  codey_wav_info_t *pre, *cur;

  // ESP_LOGI(TAG, "remove wav file: %d", node->wav_id);
  if(*header)
  {
    pre = cur = *header;
    while(cur)
    {
      if(cur == node)        // find the node
      {
        if(pre == cur)       // The first node
        {
          *header = cur->next;
        }
        else
        {
          pre->next = cur->next;
        }
        cur = NULL;
      }
      else
      {
        // First node, just move the cur to next, pre state on this node
        if(pre == cur)
        {
          cur = cur->next;
        }
        else
        {
          pre = cur;
          cur = cur->next;
        }
      }
    }
  }
}

static uint32_t get_wav_play_list_cnt(codey_wav_info_t *header)
{
  uint32_t cnt;

  cnt = 0;
  while(header)
  {
    if(FILE_MUSIC == header->music_type)
    {
      cnt++;
    }
    header = header->next;
  }
  return cnt;
}

static void IRAM_ATTR music_dac_out_voltage(uint8_t dac_value)
{
  CLEAR_PERI_REG_MASK(SENS_SAR_DAC_CTRL1_REG, SENS_SW_TONE_EN);
  CLEAR_PERI_REG_MASK(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_CW_EN1_M);
  SET_PERI_REG_BITS(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_DAC, dac_value, RTC_IO_PDAC1_DAC_S);
  SET_PERI_REG_MASK(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_XPD_DAC | RTC_IO_PDAC1_DAC_XPD_FORCE);
}

static void codey_music_play_main_loop(void * parameter)
{
  while(1)
  {
    if(xSemaphoreTake(s_wav_buf_sem, MUSIC_TASK_LOOP_TIME_MS/portTICK_PERIOD_MS) == pdTRUE)
    {
      move_music_to_block_buffer(s_wav_play_block_buffer.in_block_idx);
    }
    
    remove_finish_voice_node(&s_music_data_header);
    touch_music_play_action();
  }
}

static void IRAM_ATTR music_timer_isr(void *para)
{
  uint8_t data;
  uint32_t idx;
  uint32_t cnt;
  uint64_t freq;
  s_timer->int_clr_timers.t1 = 1;

  cnt = s_wav_play_block_buffer.block[s_wav_play_block_buffer.out_block_idx].block_cnt;
  idx = s_wav_play_block_buffer.block[s_wav_play_block_buffer.out_block_idx].block_idx;
  if(s_change_block)
  {
    freq = s_wav_play_block_buffer.block[s_wav_play_block_buffer.out_block_idx].freq;
    s_timer->hw_timer[USING_TIMER_NUM].load_high = (uint32_t) ((TIMER_RUN_FREQ/freq) >> 32);
    s_timer->hw_timer[USING_TIMER_NUM].load_low = (uint32_t) (TIMER_RUN_FREQ/freq);
    s_timer->hw_timer[USING_TIMER_NUM].config.alarm_en = 1;
    s_timer->hw_timer[USING_TIMER_NUM].reload = 1;
    s_timer->hw_timer[USING_TIMER_NUM].update = 1;
    s_change_block = false;
  }

  if(idx < cnt)
  {
    data = s_wav_play_block_buffer.block[s_wav_play_block_buffer.out_block_idx].block_buf[idx++];
    data = (uint8_t)((data * s_volume_div) / 100);;
    music_dac_out_voltage(data);
    s_wav_play_block_buffer.block[s_wav_play_block_buffer.out_block_idx].block_idx = idx;    
    if(idx == cnt)
    {
      s_change_block = true;
      // This block play out, change to another block
      s_wav_play_block_buffer.out_block_idx = !s_wav_play_block_buffer.out_block_idx;
      s_wav_play_block_buffer.in_block_idx = !s_wav_play_block_buffer.in_block_idx;
      cnt = s_wav_play_block_buffer.block[s_wav_play_block_buffer.out_block_idx].block_cnt;
      if(cnt)
      {
        s_change_block = true;
        s_timer->hw_timer[USING_TIMER_NUM].config.alarm_en = 1;
        xSemaphoreGiveFromISR(s_wav_buf_sem, pdFALSE);
      }
      else
      {
        s_timer->hw_timer[USING_TIMER_NUM].config.alarm_en = 0;
        s_timer->hw_timer[USING_TIMER_NUM].config.enable = 0;
        s_is_music_on_play = false;
      }
    }
    else
    {
      s_timer->hw_timer[USING_TIMER_NUM].config.alarm_en = 1;
    }
  }
  else
  {
    s_timer->hw_timer[USING_TIMER_NUM].config.alarm_en = 0;
    s_timer->hw_timer[USING_TIMER_NUM].config.enable = 0;   
    s_is_music_on_play = false;
  }
}

static void music_timer_init(uint32_t freq)
{
  int timer_group = USING_TIMER_GROUP;
  int timer_idx = USING_TIMER_NUM;
  timer_config_t config;
  
  config.alarm_en = 1;
  config.auto_reload = 1;
  config.counter_dir = TIMER_COUNT_DOWN;
  config.divider = 16;
  config.intr_type = TIMER_INTR_LEVEL;
  config.counter_en = TIMER_PAUSE;

  // ESP_LOGI(TAG, "timer freq: %d", freq);

  timer_init(timer_group, timer_idx, &config);
  timer_pause(timer_group, timer_idx);
  timer_set_counter_value(timer_group, timer_idx, (TIMER_RUN_FREQ/freq));
  timer_set_alarm_value(timer_group, timer_idx, 0x00000000ULL);
  timer_enable_intr(timer_group, timer_idx);
  timer_isr_register(timer_group, timer_idx, music_timer_isr, NULL, 0, NULL);
  timer_start(timer_group, timer_idx);
}

static void start_play_music(void)
{
  uint32_t freq;

  // ESP_LOGI(TAG, ">>>>>>>>>>>>>>>>>>>>>>start_play_music");
  s_is_music_on_play = true;
  s_change_block = false;
  s_timer = (USING_TIMER_GROUP == TIMER_GROUP_0)?(&TIMERG0):(&TIMERG1);
  freq = s_wav_play_block_buffer.block[s_wav_play_block_buffer.out_block_idx].freq;
  music_timer_init(freq);
}

// END OF FILE

