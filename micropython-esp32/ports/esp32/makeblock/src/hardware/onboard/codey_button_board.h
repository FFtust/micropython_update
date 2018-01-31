/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_button_board.c
 * @file    codey_button_board.h
 * @author  fftust
 * @version V1.0.0
 * @date    2017/05/23
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
 * This file is a drive  button_board module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/05/23      1.0.0              build the new.
 * </pre>
 *
 */

#ifndef _CODEY_BUTTON_BOARD_H_
#define _CODEY_BUTTON_BOARD_H_

/******************************************************************************
 DEFINE MACRO 
 ******************************************************************************/
#define BUTTON1_IO    (2)    // down
#define BUTTON2_IO    (12)   // down
#if CODEY_V107
#define BUTTON3_IO    (27)   // up
#else
#define BUTTON3_IO    (0)    // up
#endif 

#define BUTTON_NUM    (3)    // the number of buttons on board

/******************************************************************************
 DEFINE TYPES 
 ******************************************************************************/ 

/******************************************************************************
 DECLARE PUBLIC FUNCTIONS AND DATAS
 ******************************************************************************/
extern const mp_obj_type_t codey_button_board_type;

extern void    codey_button_board_config_t(void);  
extern uint8_t codey_button_board_get_status_t(uint button_io);
extern uint8_t codey_button_board_get_all_status_t(void);
extern bool    codey_is_button_released_t(uint8_t button_id);
extern bool    codey_is_button_pressed_t(uint8_t button_id);
extern uint8_t codey_button_board_read_status_t(void);

#endif /* _CODEY_BUTTON_BOARD_H_ */


