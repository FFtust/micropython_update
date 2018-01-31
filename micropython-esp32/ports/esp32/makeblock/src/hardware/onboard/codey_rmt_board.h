/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_rmt_board.c
 * @file    codey_rmt_board.h
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/24
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
 * This file is a drive  rmt_board module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust              2017/03/24    1.0.0            build the new.
 * </pre>
 *
 */

#ifndef _CODEY_RMT_BOARD_H_
#define _CODEY_RMT_BOARD_H_

/******************************************************************************
 DECLARE CONSTANTS
 ******************************************************************************/
/* for esp32 rmt config */
#if CODEY_V107
#define CODEY_RMT_BOARD_RX_CHANNEL    (0)
#define CODEY_RMT_BOARD_RX_GPIO       (5)

#define CODEY_RMT_BOARD_TX_CHANNEL    (1)
#define CODEY_RMT_BOARD_TX_GPIO       (26)
#else
#define CODEY_RMT_BOARD_RX_CHANNEL    (0)
#define CODEY_RMT_BOARD_RX_GPIO       (26)

#define CODEY_RMT_BOARD_TX_CHANNEL    (1)
#define CODEY_RMT_BOARD_TX_GPIO       (5)

#endif

#define RMT_RX_ACTIVE_LEVEL    (0)  
#define RMT_TX_CARRIER_EN      (1)   

#define RMT_TX_CHANNEL         (1)
#define RMT_TX_GPIO_NUM        (5)

#define RMT_RX_CHANNEL         (0)     /*!< RMT channel for transmitter */
#define RMT_RX_GPIO_NUM        (26)     /*!< GPIO number for transmitter signal */
#define RMT_CLK_DIV            (100)    /*!< RMT counter clock divider */
#define RMT_TICK_10_US         (80000000 / RMT_CLK_DIV / 100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */

/* for nec config */
#define NEC_HEADER_HIGH_US     (9000)                         /*!< NEC protocol header: positive 9ms */
#define NEC_HEADER_LOW_US      (4500)                         /*!< NEC protocol header: negative 4.5ms*/
#define NEC_REPETITION_HIGH_US (9000)                         /*!< NEC protocol repetition : positive 9ms */
#define NEC_REPETITION_LOW_US  (2250)                         /*!< NEC protocol repetition: negative 2.25ms*/

#define NEC_BIT_ONE_HIGH_US    (560)                          /*!< NEC protocol data bit 1: positive 0.56ms */
#define NEC_BIT_ONE_LOW_US     (2250 - NEC_BIT_ONE_HIGH_US)   /*!< NEC protocol data bit 1: negative 1.69ms */
#define NEC_BIT_ZERO_HIGH_US   (560)                          /*!< NEC protocol data bit 0: positive 0.56ms */
#define NEC_BIT_ZERO_LOW_US    (1120 - NEC_BIT_ZERO_HIGH_US)  /*!< NEC protocol data bit 0: negative 0.56ms */
#define NEC_BIT_END            (560)                          /*!< NEC protocol end: positive 0.56ms */
#define NEC_BIT_MARGIN         (150)                          /*!< NEC parse margin time */

#define NEC_ITEM_DURATION(d)   ((d & 0x7fff) * 10 / RMT_TICK_10_US)  /*!< Parse duration time from memory register value */
#define NEC_DATA_ITEM_NUM      (34)     /*!< NEC code item number: header + 32bit data + end */
#define RMT_TX_DATA_NUM        (10)     /*!< NEC tx test data number */
#define RMT_ITEM32_tIMEOUT_US  (9500)   /*!< RMT receiver timeout value(us) */

#define RMT_REC_DATA_BUFFER_LEN      (128)

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/
typedef enum
{
  CODEY_RMT_BOARD_RECTASK_NEED_CREATE,
  CODEY_RMT_BOARD_RECTASK_CREATED,
  CODEY_RMT_BOARD_RECTASK_NEED_DELETE,
  CODEY_RMT_BOARD_RECTASK_MAX
}codey_rmt_board_rectask_status_t;

typedef struct
{
  union
  {
    uint16_t addr_a; 
    uint8_t  addr_b[2];
  }address;

  union
  {
    uint16_t cmd_a;
    uint8_t  cmd_b[2];
  }command;

}codey_rmt_board_receive_item_t;

typedef struct
{
  uint8_t index;
  bool    buffer_over_flag;
  uint8_t data_receive_num;
  codey_rmt_board_receive_item_t data_latest;
  codey_rmt_board_receive_item_t data_history[RMT_REC_DATA_BUFFER_LEN];
}codey_rmt_board_receive_data_buffer_t;

typedef struct
{
  uint8_t item_data[50];
  uint16_t item_num;
  uint16_t item_parsed_num;
}codey_rmt_board_learning_t;

extern const mp_obj_type_t codey_rmt_board_type;

extern void codey_rmt_board_rx_create_t(uint16_t rec_timewait);
extern void codey_rmt_board_rx_remove_t(void);
extern bool codey_rmt_board_tx_init_t(void);
extern bool codey_rmt_board_rx_init_t(void);
extern bool codey_rmt_board_get_buffer_over_status(void);
extern uint8_t codey_rmt_board_get_buffer_data_num(void);
extern void codey_rmt_board_send_t(uint16_t address, uint16_t cmd_num, uint16_t * const command);

#endif /* _CODEY_RMT_BOARD_H_ */



