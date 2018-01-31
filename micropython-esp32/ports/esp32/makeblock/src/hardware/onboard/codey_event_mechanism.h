/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_event_mechanism.c
 * @file    codey_event_mechanism.h
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

#ifndef _CODEY_EVENT_H_
#define _CODEY_EVENT_H_

typedef enum
{
  EVE_BUTTON1_PRESS = 1,
  EVE_BUTTON1_RELEASE = 2,
  EVE_BUTTON2_PRESS = 3,
  EVE_BUTTON2_RELEASE = 4,
  EVE_BUTTON3_PRESS = 5,
  EVE_BUTTON3_RELEASE = 6,
  /* about sound sensor */
  EVE_SOUND_OVER = 7,
  EVE_SOUND_UNDER = 8,
  /* about light sensor */
  EVE_LIGHT_OVER = 9,
  EVE_LIGHT_UNDER = 10,
  /* about message */
  /* hold 12-15 for later usage */
  EVE_MESSAGE = 11,
  /* about gyro */ 
  EVE_BOARD_SHAKE = 16,
  EVE_BOARD_TILT_LEFT = 17,
  EVE_BOARD_TILT_RIGHT = 18,
  EVE_BOARD_TILT_FORWARD = 19,
  EVE_BOARD_TILT_BACK = 20,
  EVE_BOARD_SCREEN_UP = 21,
  EVE_BOARD_SCREEN_DOWN = 22,
  EVE_BOARD_FREE_FALL = 23,

  EVE_CODEY_LAUNCH = 24,
 
  EVENT_TYPE_MAX = 25
}codey_event_type_t;

extern void    codey_eve_trigger_t(codey_event_type_t eve_type, void *para);
extern void    codey_eve_deinit_t(void);
extern void    codey_eve_init_t(void);
extern bool    codey_eve_get_start_flag_t(void);
extern void    codey_eve_set_triggerd_flag_t(codey_event_type_t eve_type, bool sta);
extern bool    codey_eve_get_triggerd_flag_t(codey_event_type_t eve_type);

extern const mp_obj_type_t codey_event_type;

#endif /* _CODEY_EVENT_H_ */


