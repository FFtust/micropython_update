/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_uarts_data_deal.c
 * @file    codey_uarts_data_deal.c.
 * @author  fftust
 * @version V1.0.0
 * @date    2017/08/16
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
 * This file is a drive uarts module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/08/16      1.0.0              build the new.
 * </pre>
 *
 */

#ifndef _CODEY_UARTS_DATA_DEAL_H_
#define _CODEY_UARTS_DATA_DEAL_H_

extern void codey_uart0_init(uint32_t baud_rate);
extern void codey_uart1_init(uint32_t baud_rate);
extern void codey_uart_change_baud_rate(uart_port_t port, uint32_t new_rate);

extern void codey_neurons_uart_rec_start(void);
extern int  codey_uart0_buffer_get_char(uint8_t *c);
extern int  codey_uart0_send_chars(uint8_t *buffer, uint16_t size);
extern int  codey_uart1_buffer_get_char(uint8_t *c);
extern int  codey_uart1_send_chars(uint8_t *buffer, uint16_t size);
extern void codey_give_data_recv_sem(void);
extern void codey_give_data_recv_sem_from_isr(void);
extern void codey_give_data_recv_sem_init(void);
extern bool codey_take_data_recv_sem(uint32_t);

#endif /* _CODEY_UARTS_DATA_DEAL_H_ */
