/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Driver for IR module
 * @file    codey_rmt_board.c
 * @author  fftust
 * @version V1.0.0
 * @date    2017/05/27
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
 * This file is a drive rmt_board module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/05/27      1.0.0              build the new.
 * </pre>
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "py/mpstate.h"
#include "py/runtime.h"
#include "esp_log.h"

#include "py/nlr.h"
#include "py/obj.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "uart.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"

#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"

#include "codey_sys.h"
#include "codey_rmt_board.h"
#include "mpthread.h"

/******************************************************************************
 DEFINE MACROS
 ******************************************************************************/
#define TAG                          ("codey_rmt")
#define IR_REC_BUFFER_CLEAR_INTERVAL (130) // ms
#define RMT_REC_WAIT_TICK            (50) // ms
#define CHAR_STRING_END              (0x0A) // \n
#define TX_DONE_WAIT_TIME            (500)  // ms
/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/
 typedef struct
 {
   mp_obj_base_t base;  
   bool tx_enabled;
   bool rx_enabled;
   uint16_t rec_timewait;
 }codey_rmt_board_obj_t;

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
static const char* NEC_TAG = "NEC";

STATIC codey_rmt_board_obj_t s_codey_rmt_board_obj = {.base = {&codey_rmt_board_type},
                                                      .rec_timewait = 5000};
static TaskHandle_t s_rmt_rec_task_handle;
static uint16_t s_codey_rmt_rx_timewait;
static bool s_codey_rmt_rx_init_flag = false;
static bool s_codey_rmt_tx_init_flag = false;

static codey_rmt_board_rectask_status_t s_rectask_status = CODEY_RMT_BOARD_RECTASK_NEED_CREATE;
static codey_rmt_board_receive_data_buffer_t s_codey_rmt_board_receive_data_buffer;
static bool s_ir_button_hold_flag = false;
// static codey_rmt_board_learning_t s_codey_rmt_board_learning; // will be used later
static RingbufHandle_t s_codey_rmt_board_rb = NULL;
#if 0
/* it will be used in IR learning */
static rmt_item32_t s_test_data[200];
static uint8_t s_t_data[50];
static uint16_t s_data_num = 0; 
static bool s_test_flag = false;
static uint16_t s_test_size = 0;
#endif

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC inline void nec_fill_item_level(rmt_item32_t *item, int high_us, int low_us);
STATIC inline bool nec_check_in_range(int duration_ticks, int target_us, int margin_us);

STATIC void nec_fill_item_header(rmt_item32_t *item);
STATIC void nec_fill_item_bit_one(rmt_item32_t *item);
STATIC void nec_fill_item_bit_zero(rmt_item32_t *item);
STATIC void nec_fill_item_end(rmt_item32_t *item);

STATIC bool nec_header_if(rmt_item32_t *item);
STATIC bool nec_bit_one_if(rmt_item32_t *item);
STATIC bool nec_bit_zero_if(rmt_item32_t *item);
STATIC int  nec_parse_items(rmt_item32_t *item, int item_num, uint16_t *addr, uint16_t *data);
STATIC int  nec_build_items(int channel, rmt_item32_t *item, int item_num, uint16_t addr, uint16_t cmd_data);
STATIC void nec_build_n_items_t(rmt_item32_t *item, int item_num, uint8_t *data );

STATIC void codey_rmt_board_tx_config_t(void);
STATIC void codey_rmt_board_rx_config_t(void);
STATIC void codey_rmt_board_rx_task_create_t(uint16_t waittime);
STATIC void codey_rmt_board_rx_task_deleted_t(void);
STATIC void codey_rmt_board_set_rx_task_status_t(codey_rmt_board_rectask_status_t sta);

STATIC void codey_rmt_board_receive_data_buffer_init_t(void);
STATIC void codey_rmt_board_receive_data_buffer_item_create_t(uint16_t addr, uint16_t cmd, codey_rmt_board_receive_item_t *item);
STATIC void codey_rmt_board_receive_data_buffer_push_t(uint8_t size, codey_rmt_board_receive_item_t *item);
STATIC void codey_rmt_board_receive_data_buffer_pop_t(codey_rmt_board_receive_item_t *const item);
STATIC void codey_rmt_board_receive_data_buffer_get_t(codey_rmt_board_receive_item_t *const item);
STATIC void codey_rmt_board_nec_rx_task(void* pvParameter);
STATIC codey_rmt_board_rectask_status_t codey_rmt_board_get_rx_task_status_t(void);

/*****************************************************************************
DEFINE PUBLIC FUNCTION 
******************************************************************************/
uint8_t codey_rmt_board_get_buffer_data_num(void)
{ 
  return s_codey_rmt_board_receive_data_buffer.data_receive_num;
}

bool codey_rmt_board_get_buffer_over_status(void)
{ 
  return s_codey_rmt_board_receive_data_buffer.buffer_over_flag;
}

bool codey_rmt_board_tx_init_t(void)
{
  if(s_codey_rmt_tx_init_flag == false)
  {
    codey_rmt_board_tx_config_t();
    s_codey_rmt_tx_init_flag = true;
  }
  return true;
}

bool codey_rmt_board_rx_init_t(void)
{
  if(s_codey_rmt_rx_init_flag == false)
  {
    codey_rmt_board_receive_data_buffer_init_t();
    codey_rmt_board_rx_config_t();

    /*
    rmt_get_ringbuf_handler(codey_RMT_BOARD_RX_CHANNEL, &s_codey_rmt_board_rb);
    if(s_codey_rmt_board_rb == NULL)
    {
      return false;
    }
    rmt_rx_start(CODEY_RMT_BOARD_RX_CHANNEL, 1);
    */
    codey_rmt_board_rx_task_create_t(0);
    s_codey_rmt_rx_init_flag = true;
    return true;
  }
  else
  {
    return true;
  }
}

void codey_rmt_board_tx_remove_t(void)
{
  rmt_tx_stop(CODEY_RMT_BOARD_TX_CHANNEL);
  rmt_driver_uninstall(CODEY_RMT_BOARD_TX_CHANNEL);
  s_codey_rmt_tx_init_flag = false;
}

void codey_rmt_board_rx_remove_t(void)
{
  codey_rmt_board_rx_task_deleted_t();
  rmt_rx_stop(CODEY_RMT_BOARD_RX_CHANNEL);
  rmt_memory_rw_rst(CODEY_RMT_BOARD_RX_CHANNEL);
  rmt_driver_uninstall(CODEY_RMT_BOARD_RX_CHANNEL);
  s_codey_rmt_rx_init_flag = false;
}

#if 0
void codey_rmt_board_rx_output_data_t(uint8_t *const cmd, uint8_t *const addr)
{
  uint8_t size = codey_rmt_board_get_buffer_data_num();
  if(size == 0)
  {
    return;
  }
  codey_rmt_board_receive_item_t receive_item[size];
  codey_rmt_board_receive_data_buffer_get_t(receive_item);
  if(cmd != NULL)
  {
    for(uint8_t i = 0; i < size; i++)
    {
      cmd[i] = receive_item[i].command.cmd_b[0];
    }
  }

  if(addr != NULL)
  {
    for(uint8_t i = 0; i < size; i++)
    {
      addr[i] = receive_item[i].address.addr_b[0];
    }
  }
}
#endif

void codey_rmt_board_send_t(uint16_t address, uint16_t cmd_num, uint16_t * const command)
{
  uint16_t   channel = RMT_TX_CHANNEL;
  uint16_t   nec_tx_num = cmd_num;

  size_t size = (sizeof(rmt_item32_t) * NEC_DATA_ITEM_NUM * nec_tx_num);
  rmt_item32_t *item = (rmt_item32_t*)heap_caps_malloc(size, MALLOC_CAP_8BIT);
 
  int item_num = NEC_DATA_ITEM_NUM * nec_tx_num;

  memset((void*)item, 0, size);

  int item_index = 0;
  int offset = 0;
  uint16_t *p_cmd = command;
  while(1)
  {
    item_index = nec_build_items(channel, item + offset, item_num - offset, ((~address) << 8) | address, ((~(*p_cmd)) << 8) | (*p_cmd));
    if(item_index < 0)
    {
      break;
    }
    p_cmd++;
    offset += item_index;
  }
  
  rmt_write_items(channel, item, item_num, true);
  MP_THREAD_GIL_EXIT();
  rmt_wait_tx_done(channel, TX_DONE_WAIT_TIME / portTICK_PERIOD_MS);
  MP_THREAD_GIL_ENTER();
  heap_caps_free(item);
}

void codey_rmt_board_send_n_bits_t(uint16_t item_num, uint8_t *data)
{
  uint16_t channel = RMT_TX_CHANNEL;

  size_t size = (sizeof(rmt_item32_t) * (item_num));
  rmt_item32_t *item = (rmt_item32_t*)heap_caps_malloc(size, MALLOC_CAP_8BIT);
 
  memset((void*)item, 0, size);
  /* substract head and end item */
  nec_build_n_items_t(item, item_num -2, data);
  rmt_write_items(channel, item, item_num, true);
  rmt_wait_tx_done(channel, TX_DONE_WAIT_TIME / portTICK_PERIOD_MS);
  heap_caps_free(item);
}

uint8_t codey_rmt_board_get_single_char_t()
{
  uint8_t item_num = 0;
  item_num = codey_rmt_board_get_buffer_data_num();
  item_num = item_num > RMT_REC_DATA_BUFFER_LEN ? RMT_REC_DATA_BUFFER_LEN : item_num;
  codey_rmt_board_receive_item_t item[item_num];
  
  if(item_num == 1)
  {
    codey_rmt_board_receive_data_buffer_get_t(item); 
    return item[0].command.cmd_b[0];
  }
  else
  {
    return 0;
  }
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
/*  drive functions start (NEC) */
/*
 * @brief Build register value of waveform for NEC one data bit
 */
STATIC inline void nec_fill_item_level(rmt_item32_t* item, int high_us, int low_us)
{
  item->level0 = 1;
  item->duration0 = (high_us) / 10 * RMT_TICK_10_US;
  item->level1 = 0;
  item->duration1 = (low_us) / 10 * RMT_TICK_10_US;
}

/*
 * @brief Generate NEC header value: active 9ms + negative 4.5ms
 */
STATIC void nec_fill_item_header(rmt_item32_t* item)
{
  nec_fill_item_level(item, NEC_HEADER_HIGH_US, NEC_HEADER_LOW_US);
}

/*
 * @brief Generate NEC data bit 1: positive 0.56ms + negative 1.69ms
 */
STATIC void nec_fill_item_bit_one(rmt_item32_t* item)
{
  nec_fill_item_level(item, NEC_BIT_ONE_HIGH_US, NEC_BIT_ONE_LOW_US);
}

/*
 * @brief Generate NEC data bit 0: positive 0.56ms + negative 0.56ms
 */
STATIC void nec_fill_item_bit_zero(rmt_item32_t* item)
{
  nec_fill_item_level(item, NEC_BIT_ZERO_HIGH_US, NEC_BIT_ZERO_LOW_US);
}

/*
 * @brief Generate NEC end signal: positive 0.56ms
 */
STATIC void nec_fill_item_end(rmt_item32_t* item)
{
  nec_fill_item_level(item, NEC_BIT_END, 0x7fff);
}

/*
 * @brief Check whether duration is around target_us
 */
STATIC inline bool nec_check_in_range(int duration_ticks, int target_us, int margin_us)
{
  if((NEC_ITEM_DURATION(duration_ticks) < (target_us + margin_us)) && (NEC_ITEM_DURATION(duration_ticks) > (target_us - margin_us)))
  { 
    return true;
  }
  else
  {
    return false;
  }
}

/*
 * @brief Check whether this value represents an NEC header
 */
STATIC bool nec_header_if(rmt_item32_t *item)
{
  if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
    && nec_check_in_range(item->duration0, NEC_HEADER_HIGH_US, NEC_BIT_MARGIN)
    && nec_check_in_range(item->duration1, NEC_HEADER_LOW_US, NEC_BIT_MARGIN)) 
  {

    return true;
  }
  
  return false;
}

STATIC bool nec_repetition_if(rmt_item32_t *item)
{
  if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
    && nec_check_in_range(item->duration0, NEC_REPETITION_HIGH_US, NEC_BIT_MARGIN)
    && nec_check_in_range(item->duration1, NEC_REPETITION_LOW_US, NEC_BIT_MARGIN)) 
  {
    return true;
  }
  
  return false;
}

/*
 * @brief Check whether this value represents an NEC data bit 1
 */
STATIC bool nec_bit_one_if(rmt_item32_t *item)
{
  if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
    && nec_check_in_range(item->duration0, NEC_BIT_ONE_HIGH_US, NEC_BIT_MARGIN)
    && nec_check_in_range(item->duration1, NEC_BIT_ONE_LOW_US, NEC_BIT_MARGIN))
  {
    return true;
  }

  return false;
}

/*
 * @brief Check whether this value represents an NEC data bit 0
 */
STATIC bool nec_bit_zero_if(rmt_item32_t *item)
{
  if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
    && nec_check_in_range(item->duration0, NEC_BIT_ZERO_HIGH_US, NEC_BIT_MARGIN)
    && nec_check_in_range(item->duration1, NEC_BIT_ZERO_LOW_US, NEC_BIT_MARGIN))
  {
    return true;
  }
  
  return false;
}

/*
 * @brief Parse NEC 32 bit waveform to address and command.
 */
STATIC int nec_parse_items(rmt_item32_t *item, int item_num, uint16_t *addr, uint16_t *data)
{
  int w_len = item_num;
  if(nec_repetition_if(item))
  {
    s_ir_button_hold_flag = true;
    return -1;
  }
  else
  {
    s_ir_button_hold_flag = false;
    if(w_len < NEC_DATA_ITEM_NUM)
    {
      return -1;
    }
    int i = 0, j = 0;
  
    if(!nec_header_if(item++)) 
    {
      return -1;
    }
    uint16_t addr_t = 0;
    for(j = 0; j < 16; j++)
    {
      if(nec_bit_one_if(item)) 
      {
        addr_t |= (1 << j);
      } 
      else if(nec_bit_zero_if(item))
      {
        addr_t |= (0 << j);
      } 
      else
      {
        return -1;
      }
      item++;
      i++;
    }
    uint16_t data_t = 0;
    for(j = 0; j < 16; j++)
    {
      if(nec_bit_one_if(item))
      {
        data_t |= (1 << j);
      }  
      else if(nec_bit_zero_if(item))
      {
        data_t |= (0 << j);
      }  
      else
      {
        return -1;
      }
      item++;
      i++;
    }
    *addr = addr_t;
    *data = data_t;
    
    return i;
  }
}

/*
 * @brief Build NEC 32bit waveform.
 */
STATIC int nec_build_items(int channel, rmt_item32_t *item, int item_num, uint16_t addr, uint16_t cmd_data)
{
  int i = 0, j = 0;
  if(item_num < NEC_DATA_ITEM_NUM)
  {
    return -1;
  }
  nec_fill_item_header(item++);
  i++;
  for(j = 0; j < 16; j++) 
  {
    if(addr & 0x1)
    {
      nec_fill_item_bit_one(item);
    }
    else 
    {
      nec_fill_item_bit_zero(item);
    }
    item++;
    i++;
    addr >>= 1;
  }
  for(j = 0; j < 16; j++)
  {
    if(cmd_data & 0x1)
    {
      nec_fill_item_bit_one(item);
    }
    else
    {
      nec_fill_item_bit_zero(item);
    }
    item++;
    i++;
    cmd_data >>= 1;
  }
  nec_fill_item_end(item);
  i++;

  return i;
}

/*
 * @brief Build n bits waveform.
 */
STATIC void nec_build_n_items_t(rmt_item32_t *item, int item_num, uint8_t *data)
{
  int i = 0, j = 0;
  uint8_t *p_temp = data;
  rmt_item32_t *p_item_temp = item;
  uint8_t temp_data = NULL;
  uint8_t u8_num = item_num / 8;
  uint32_t u8_mod_num = item_num % 8;
  
  nec_fill_item_header(p_item_temp++);
  for(i = 0; i < u8_num; i++)
  {
    temp_data = *(p_temp + i);
    for(j = 0; j < 8; j++) 
    {
      if(temp_data & 0x1)
      {
        nec_fill_item_bit_one(p_item_temp);
      }
      else 
      {
        nec_fill_item_bit_zero(p_item_temp);
      }
      p_item_temp++;
      temp_data >>= 1;
    }
    
  }
  if(u8_mod_num != 0)
  {
    temp_data = *(p_temp + i);
    for(j = 0; j < u8_mod_num; j++)
    {      
      if(temp_data & 0x1)
      {
        nec_fill_item_bit_one(p_item_temp);
      }
      else 
      {
        nec_fill_item_bit_zero(p_item_temp);
      }
      p_item_temp++;
      temp_data >>= 1; 
    }

  }
  nec_fill_item_end(p_item_temp);
}

/*
 * @brief parse n bits waveform.
 */
#if 0 
/* it will be used in IR learning */
STATIC void nec_parse_n_items_t(rmt_item32_t *item, int rx_size, codey_rmt_board_learning_t *rmt_learning)
{
  memcpy(s_test_data, item, rx_size);
  rmt_learning->item_num = rx_size / 4;
        
  uint8_t u8_num = 0;
  uint8_t u8_mod_num = 0;
  rmt_item32_t *p_temp = item;

  /* substract head and end item */ 
  u8_num = (rx_size / 4 - 2) / 8;
  u8_mod_num = (rx_size / 4 - 2) % 8;
  
  if(!nec_header_if(p_temp++)) 
  {
    return;
  }
  rmt_learning->item_parsed_num++;
  for(uint8_t i = 0; i < u8_num; i++)
  {
    for(uint8_t j = 0; j < 8; j++)
    {
      if(nec_bit_one_if(p_temp))
      {
        rmt_learning->item_data[i] |= (1 << j);
        rmt_learning->item_parsed_num++;
      }
      else if(nec_bit_zero_if(p_temp))
      {
        rmt_learning->item_data[i] |= (0 << j);
        rmt_learning->item_parsed_num++;
      }  
      else
      {
        return;
      }
      p_temp++;
    }
  }
  if(u8_mod_num != 0)
  {
    for(uint8_t j = 0; j < u8_mod_num; j++)
    {
      if(nec_bit_one_if(p_temp++))
      {
        rmt_learning->item_data[u8_num] |= (1 << j);
        rmt_learning->item_parsed_num++;
      }
      else if(nec_bit_zero_if(p_temp))
      {
        rmt_learning->item_data[u8_num] |= (0 << j);
        rmt_learning->item_parsed_num++;
      }  
      else
      {
        return;
      }
    }
  }
}

STATIC void ir_learn_test_t(rmt_item32_t *item, int rx_size)
{
  ESP_LOGD(TAG, "***********%d\n",rx_size);
  memcpy(s_test_data, item, rx_size);
  s_test_size = rx_size;
  s_test_flag = true;
  ESP_LOGD(TAG, "**********send done\n");

  memset(s_t_data, 0 , 50);
 
  uint8_t u8_num = 0;
  uint8_t u8_mod_num = 0;
  rmt_item32_t *p_temp = item;
  u8_num = (rx_size / 4 - 2) / 8;
  u8_mod_num = (rx_size / 4 - 2) % 8;
  ESP_LOGD(TAG, "***********%d, %d, %d\n", rx_size, u8_num, u8_mod_num);
  if(!nec_header_if(p_temp++)) 
  {
    ESP_LOGD(TAG, "not head\n");
  }
  for(uint8_t i = 0; i < u8_num; i++)
  {
    for(uint8_t j = 0; j < 8; j++)
    {
      if(nec_bit_one_if(p_temp))
      {
        s_t_data[i] |= (1 << j);
      }
      else if(nec_bit_zero_if(p_temp))
      {
        s_t_data[i] |= (0 << j);
      }   
      else
      {
        ESP_LOGD(TAG, "not one and zero \n");
      }
      p_temp++;
    }
  }
 
  if(u8_mod_num != 0)
  {
    for(uint8_t j = 0; j < u8_mod_num; j++)
    {
      if(nec_bit_one_if(p_temp++))
      {
        s_t_data[u8_num] |= (1 << j);
      }
      else if(nec_bit_zero_if(p_temp))
      {
        s_t_data[u8_num] |= (0 << j);
      }  
      else
      {
        ESP_LOGD(TAG, "***not one and zero");
      }
    }
  }
}
#endif
/* drive functions end (NEC) */

/* define private drive function start */
STATIC void codey_rmt_board_tx_config_t(void)
{
  if(s_codey_rmt_tx_init_flag == false)
  {
    rmt_config_t rmt_tx;
    rmt_tx.channel = CODEY_RMT_BOARD_TX_CHANNEL;
    rmt_tx.gpio_num = CODEY_RMT_BOARD_TX_GPIO;
    rmt_tx.mem_block_num = 1;
    rmt_tx.clk_div = RMT_CLK_DIV;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_duty_percent = 50;
    rmt_tx.tx_config.carrier_freq_hz = 38000;
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = RMT_TX_CARRIER_EN;
    rmt_tx.tx_config.idle_level = 0;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.rmt_mode = 0;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }  
}

STATIC void codey_rmt_board_rx_config_t(void)
{
  if(s_codey_rmt_rx_init_flag == false)
  {
    rmt_config_t rmt_rx;
    rmt_rx.channel = CODEY_RMT_BOARD_RX_CHANNEL;
    rmt_rx.gpio_num = CODEY_RMT_BOARD_RX_GPIO ;
    rmt_rx.clk_div = RMT_CLK_DIV;
    rmt_rx.mem_block_num = 1;
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.rx_config.filter_en = true;
    rmt_rx.rx_config.filter_ticks_thresh = 100;
    rmt_rx.rx_config.idle_threshold = RMT_ITEM32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
    rmt_config(&rmt_rx);
    rmt_driver_install(rmt_rx.channel, 1000, 0);
  
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

STATIC void codey_rmt_board_receive_data_buffer_init_t(void)
{
  s_codey_rmt_board_receive_data_buffer.index = 0;
  s_codey_rmt_board_receive_data_buffer.data_receive_num = 0;
  s_codey_rmt_board_receive_data_buffer.buffer_over_flag = false;
}

STATIC void codey_rmt_board_receive_data_buffer_item_create_t(uint16_t addr, uint16_t cmd, codey_rmt_board_receive_item_t *item)
{

  if(item != NULL)
  {
    item->address.addr_a = addr;
    item->command.cmd_a = cmd;
  }
}

STATIC void codey_rmt_board_receive_data_buffer_push_t(uint8_t size, codey_rmt_board_receive_item_t *const item)
{
  uint8_t index = s_codey_rmt_board_receive_data_buffer.index;
  if(size == 0)
  {
    return;
  }
  else if(item != NULL)
  {
    s_codey_rmt_board_receive_data_buffer.data_latest = *(item+size - 1);  //get the latest data
    for(uint8_t i = 0; i < size; i++)
    {
      s_codey_rmt_board_receive_data_buffer.data_history[index] = item[i];
      index = (index + 1) % RMT_REC_DATA_BUFFER_LEN;
    }
    
    s_codey_rmt_board_receive_data_buffer.index = index;  //fftust:the next buffer index to be writed 
    s_codey_rmt_board_receive_data_buffer.data_receive_num += size;
    if(s_codey_rmt_board_receive_data_buffer.data_receive_num > RMT_REC_DATA_BUFFER_LEN)
    {
      s_codey_rmt_board_receive_data_buffer.buffer_over_flag = true;
      ESP_LOGI(TAG, "rmt buffer overflow");
    }
  }
}

STATIC void codey_rmt_board_receive_data_buffer_pop_t(codey_rmt_board_receive_item_t *const item)
{
  uint8_t index = 0;
  uint8_t data_num = 0;
  data_num = s_codey_rmt_board_receive_data_buffer.data_receive_num;
 
  if(data_num == 0)
  {
    return;
  }

  if(item != NULL)
  {
    index = (s_codey_rmt_board_receive_data_buffer.index == 0) ? (RMT_REC_DATA_BUFFER_LEN - 1) : (s_codey_rmt_board_receive_data_buffer.index - 1); //fftust: the buffer index of the latest data have been writed 
    data_num = (data_num > RMT_REC_DATA_BUFFER_LEN) ? RMT_REC_DATA_BUFFER_LEN : data_num;
    
    for(uint8_t i = 0; i < data_num; i++)
    {
      item[i] = s_codey_rmt_board_receive_data_buffer.data_history[index];
   
      if(index > 0)
      {
        index--;
      }
      else if(index == 0)
      {
        index = RMT_REC_DATA_BUFFER_LEN - 1;
      }
    }
    s_codey_rmt_board_receive_data_buffer.data_receive_num = 0;
    s_codey_rmt_board_receive_data_buffer.index = 0;
    s_codey_rmt_board_receive_data_buffer.buffer_over_flag = false;

  }
}

/* this function will not clear the rmt data buffer */
STATIC void codey_rmt_board_receive_data_buffer_get_t(codey_rmt_board_receive_item_t *const item)
{
  uint8_t index = 0;
  uint8_t data_num = 0;
  data_num = s_codey_rmt_board_receive_data_buffer.data_receive_num;
 
  if(data_num == 0)
  {
    return;
  }

  if(item != NULL)
  {
    index = (s_codey_rmt_board_receive_data_buffer.index == 0) ? (RMT_REC_DATA_BUFFER_LEN - 1) : (s_codey_rmt_board_receive_data_buffer.index - 1); 
    data_num = (data_num > RMT_REC_DATA_BUFFER_LEN) ? RMT_REC_DATA_BUFFER_LEN : data_num;
    
    for(uint8_t i = 0; i < data_num; i++)
    {
      item[i] = s_codey_rmt_board_receive_data_buffer.data_history[index];
   
      if(index > 0)
      {
        index--;
      }
      else if(index == 0)
      {
        index = RMT_REC_DATA_BUFFER_LEN - 1;
      }
    }
  }
}

STATIC void codey_rmt_board_receive_data_buffer_clear_t(void)
{
  s_codey_rmt_board_receive_data_buffer.data_receive_num = 0;
  s_codey_rmt_board_receive_data_buffer.index = 0;
  s_codey_rmt_board_receive_data_buffer.buffer_over_flag = false;
}

STATIC bool codey_rmt_board_received_a_string(void)
{
  if(s_codey_rmt_board_receive_data_buffer.data_receive_num < 2)
  {
    return false;
  }
  if(s_codey_rmt_board_receive_data_buffer.data_history[s_codey_rmt_board_receive_data_buffer.index - 1].command.cmd_b[0] == CHAR_STRING_END)
  {
    return true;
  }
  else
  {
    return false;
  }
}

STATIC void codey_rmt_board_receive_get_latest_string_t(codey_rmt_board_receive_item_t *const item, uint8_t *string_len)
{
  uint8_t index = 0;
  uint8_t data_num = 0;
  uint8_t len_count = 0;
  data_num = s_codey_rmt_board_receive_data_buffer.data_receive_num;

  (*string_len) = data_num;
  if(data_num == 0)
  {
    return;
  }
  
  if(item != NULL)
  {
    index = (s_codey_rmt_board_receive_data_buffer.index == 0) ? (RMT_REC_DATA_BUFFER_LEN - 1) : (s_codey_rmt_board_receive_data_buffer.index - 1); //fftust: the buffer index of the latest data have been writed 
    data_num = (data_num > RMT_REC_DATA_BUFFER_LEN) ? RMT_REC_DATA_BUFFER_LEN : data_num;
    
    for(len_count = 0; len_count < data_num; len_count++)
    {
     /* only return the latest string */
      if((len_count != 0 ) && s_codey_rmt_board_receive_data_buffer.data_history[index].command.cmd_b[0] == CHAR_STRING_END)
      {
        (*string_len) = len_count;
        break;
      }

      item[len_count] = s_codey_rmt_board_receive_data_buffer.data_history[index];
   
      if(index > 0)
      {
        index--;
      }
      else if(index == 0)
      {
        index = 127;
      }
    }
  }
}

STATIC codey_rmt_board_rectask_status_t codey_rmt_board_get_rx_task_status_t(void)
{
  return s_rectask_status;
}

STATIC void codey_rmt_board_set_rx_task_status_t(codey_rmt_board_rectask_status_t sta)
{
  s_rectask_status = sta;
}

STATIC void codey_rmt_board_rx_task_create_t(uint16_t waittime)
{
  if(s_rmt_rec_task_handle != NULL) 
  {
    return;
  }
  s_codey_rmt_rx_timewait = waittime;
  xTaskCreatePinnedToCore(codey_rmt_board_nec_rx_task, "rmt_nec_rx_task", 2048 * 2, &s_codey_rmt_rx_timewait, 2, &s_rmt_rec_task_handle, 0);
  codey_rmt_board_set_rx_task_status_t(CODEY_RMT_BOARD_RECTASK_CREATED);
}

STATIC void codey_rmt_board_rx_task_deleted_t(void)
{
  codey_rmt_board_set_rx_task_status_t(CODEY_RMT_BOARD_RECTASK_NEED_DELETE);
  while(s_rmt_rec_task_handle) //only if s_rmt_rec_task_handle is NULL indicate that task has been deleted
  {
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }

  codey_rmt_board_set_rx_task_status_t(CODEY_RMT_BOARD_RECTASK_NEED_CREATE);
}

STATIC void codey_rmt_board_nec_rx_task(void *pvParameter)
{
  static uint32_t start_time = 0;
  codey_rmt_board_receive_item_t codey_item[10];
  int channel = CODEY_RMT_BOARD_RX_CHANNEL;
  
  RingbufHandle_t rb = NULL;
  
  uint16_t timewait_t = *(uint16_t *)pvParameter;
  uint16_t timewait = (timewait_t == 0) ? RMT_REC_WAIT_TICK : timewait_t;
  
  rmt_get_ringbuf_handle(channel, &rb);
  rmt_rx_start(channel, 1);
  start_time = millis();
  while(rb)
  {
    size_t rx_size = 0;
    rx_size = 0;
    rmt_item32_t *item = (rmt_item32_t*)xRingbufferReceive(rb, &rx_size, timewait / portTICK_PERIOD_MS); 
    if(millis() - start_time >= IR_REC_BUFFER_CLEAR_INTERVAL)
    { 
      codey_rmt_board_receive_data_buffer_clear_t();
      start_time = millis();
    }
#if 0
    if(rx_size > 100)
    {
      ir_learn_test_t(item, rx_size);
    }
#endif

    if(item)  //if timewait is 0,then the receive task will not be deleted
    {
      uint16_t rmt_addr;
      uint16_t rmt_cmd;
      int offset = 0;
      /* if received data, reset the timer */
      start_time = millis();
      while(1)
      { 
        int res = nec_parse_items(item + offset, rx_size / 4 - offset, &rmt_addr, &rmt_cmd);
        /* received the hold signal  */
        if(s_ir_button_hold_flag == true)
        {
          ESP_LOGI(NEC_TAG, "RMT RCV A HOLD SIGNAL");
        }  
        else if(res > 0)
        {
          offset += res + 1;
          ESP_LOGI(NEC_TAG, "RMT RCV --- addr: 0x%04x cmd: 0x%04x", rmt_addr, rmt_cmd);
          codey_rmt_board_receive_data_buffer_item_create_t(rmt_addr, rmt_cmd, codey_item);
          codey_rmt_board_receive_data_buffer_push_t(1, codey_item);
        } 
        else 
        {
          break;
        } 
      }
      vRingbufferReturnItem(rb, (void *)item);
    }
    else
    {
      s_ir_button_hold_flag = false;
    }
    
    if(codey_rmt_board_get_rx_task_status_t() == CODEY_RMT_BOARD_RECTASK_NEED_DELETE)
    {
      break;
    }
  }
  ESP_LOGI(NEC_TAG, "RMT RCV --- END");
  s_rmt_rec_task_handle = NULL;  //show that this task had been deleted completed
  rmt_rx_stop(channel);
  rmt_memory_rw_rst(channel);
  vTaskDelete(NULL);
}

/******************************************************************************
 DEFINE MICROPYTHON FUNCTIONS
 ******************************************************************************/
STATIC mp_obj_t codey_rmt_board_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *all_args)
{
  // parse args
  mp_map_t kw_args;
  mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

  // setup the object
  codey_rmt_board_obj_t *self = &s_codey_rmt_board_obj;
  self->base.type = &codey_rmt_board_type;
  
  return self;
}

STATIC mp_obj_t codey_rmt_board_tx_init(mp_obj_t self_in)
{
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_rmt_board_tx_init_obj, codey_rmt_board_tx_init);

STATIC mp_obj_t codey_rmt_board_rx_init(mp_obj_t self_in, mp_obj_t arg1)
{
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(codey_rmt_board_rx_init_obj, codey_rmt_board_rx_init);

STATIC mp_obj_t codey_rmt_board_tx_deinit(mp_obj_t self_in)
{
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_rmt_board_tx_deinit_obj, codey_rmt_board_tx_deinit);

STATIC mp_obj_t codey_rmt_board_rx_deinit(mp_obj_t self_in)
{
  return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_rmt_board_rx_deinit_obj, codey_rmt_board_rx_deinit);

STATIC mp_obj_t codey_rmt_board_get_receive_value(mp_obj_t self_in)
{
  uint8_t item_num = 0;
  uint8_t str_len = 0;
  
  item_num = codey_rmt_board_get_buffer_data_num();
  item_num = item_num > RMT_REC_DATA_BUFFER_LEN ? RMT_REC_DATA_BUFFER_LEN : item_num;
  codey_rmt_board_receive_item_t item[item_num];
  
  if(codey_rmt_board_received_a_string())
  {	
    /* don't clear the buffer */
    codey_rmt_board_receive_get_latest_string_t(item, &str_len);
    mp_obj_list_t *newlist = mp_obj_new_list(str_len, NULL);
    for(uint8_t i = 0; i < item_num; i++)
    {
      newlist->items[i] = MP_OBJ_NEW_SMALL_INT(item[i].command.cmd_b[0]);
    }
    return (mp_obj_t)(newlist);
  }
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_rmt_board_get_receive_value_obj,codey_rmt_board_get_receive_value);

STATIC mp_obj_t codey_rmt_board_get_single_char(mp_obj_t self_in)
{
  uint8_t item_num = 0;
  item_num = codey_rmt_board_get_buffer_data_num();
  item_num = item_num > RMT_REC_DATA_BUFFER_LEN ? RMT_REC_DATA_BUFFER_LEN : item_num;

  if(item_num == 0)
  {
    return mp_const_none;
  }
  else
  {
    codey_rmt_board_receive_item_t item[item_num];
    /* don't clear the buffer */
    codey_rmt_board_receive_data_buffer_get_t(item);
    return MP_OBJ_NEW_SMALL_INT(item[0].command.cmd_b[0]);
  }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_rmt_board_get_single_char_obj, codey_rmt_board_get_single_char);

STATIC mp_obj_t codey_rmt_board_get_button_hold_flag(mp_obj_t self_in)
{
  if(s_ir_button_hold_flag)
  {
    return mp_const_true;
  }
  else
  {
    return mp_const_false;
  }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(codey_rmt_board_get_button_hold_flag_obj, codey_rmt_board_get_button_hold_flag);
 

STATIC mp_obj_t codey_rmt_board_send_cmd(mp_obj_t self_in, mp_obj_t arg1, mp_obj_t arg2)
{
  uint16_t addr = mp_obj_get_int(arg1);
  size_t len = 0;
  const char *cmd_str = mp_obj_str_get_data(arg2, &len);
  ESP_LOGD(TAG, "lenth is %d", len);
  uint16_t data[50] = { 0 };

  for(uint8_t i = 0; i< len; i++)
  {
    data[i] = (uint16_t)(cmd_str[i]); 
  }
  codey_rmt_board_send_t(addr, len, data);

/* control the air conditioner, Comment out last line */
/* call this funtion with parameter arg2: 'A'/ 'B' */
#if 0 
  /* control air conditioner in company */
  if((uint8_t)cmd_str[0] == 65)
  {
    s_t_data[0] = 65;
    s_t_data[1] = 2;
    s_t_data[2] = 32;
    s_t_data[3] = 80;
    s_t_data[4] = 2;
    s_test_size = 148;
    codey_rmt_board_send_n_bits_t(s_test_size / 4 , s_t_data);

  }
  else if((uint8_t)cmd_str[0] == 66)
  {
    s_t_data[0] = 73;
    s_t_data[1] = 2;
    s_t_data[2] = 32;
    s_t_data[3] = 80;
    s_t_data[4] = 2;
    s_test_size = 148;
    codey_rmt_board_send_n_bits_t(s_test_size / 4 , s_t_data);
  }


  if(s_test_flag == true)
  {
    s_test_flag = false;
    codey_rmt_board_send_n_bits_t(s_test_size / 4 , s_t_data);
  } 
#endif

  return mp_const_true; 
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(codey_rmt_board_send_cmd_obj,codey_rmt_board_send_cmd);

STATIC void codey_rmt_board_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{

}

STATIC mp_obj_t codey_rmt_board_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args)
{
  mp_arg_check_num(n_args, n_kw, 0, 0, false);
  
  return mp_const_none;
}

STATIC const mp_map_elem_t codey_rmt_board_locals_dict_table[] =
{
  { MP_OBJ_NEW_QSTR(MP_QSTR_value),           (mp_obj_t)&codey_rmt_board_get_receive_value_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_char),        (mp_obj_t)&codey_rmt_board_get_single_char_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_send),            (mp_obj_t)&codey_rmt_board_send_cmd_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_tx_init),         (mp_obj_t)&codey_rmt_board_tx_init_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_tx_deinit),       (mp_obj_t)&codey_rmt_board_tx_deinit_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_rx_init),         (mp_obj_t)&codey_rmt_board_rx_init_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_rx_deinit),       (mp_obj_t)&codey_rmt_board_rx_deinit_obj },
  { MP_OBJ_NEW_QSTR(MP_QSTR_get_hold_flag),   (mp_obj_t)&codey_rmt_board_get_button_hold_flag_obj },

};

STATIC MP_DEFINE_CONST_DICT(codey_rmt_board_locals_dict, codey_rmt_board_locals_dict_table);

const mp_obj_type_t codey_rmt_board_type =
{
  { &mp_type_type },
  .name = MP_QSTR_rmt_board,
  .print = codey_rmt_board_print,
  .make_new = codey_rmt_board_make_new,
  .call = codey_rmt_board_call,
  .locals_dict = (mp_obj_t)&codey_rmt_board_locals_dict,
};
