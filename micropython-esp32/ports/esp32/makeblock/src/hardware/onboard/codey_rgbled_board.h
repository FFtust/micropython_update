/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_rgbled_board.c.
 * @file    codey_rgbled_board.h
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
 * This file is a drive  rgbled_board module.
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

#ifndef _CODEY_RGBLED_BOARD_H_
#define _CODEY_RGBLED_BOARD_H_

#define CODEY_RGBLED_TIMER          LEDC_TIMER_0
#define CODEY_RGBLED_MODE           LEDC_HIGH_SPEED_MODE
#define CODEY_RGBLED_CH0_GPIO       (14)  // red
#define CODEY_RGBLED_CH0_CHANNEL    LEDC_CHANNEL_0
#define CODEY_RGBLED_CH1_GPIO       (21)  // green
#define CODEY_RGBLED_CH1_CHANNEL    LEDC_CHANNEL_1
#define CODEY_RGBLED_CH2_GPIO       (4)  // blue
#define CODEY_RGBLED_CH2_CHANNEL    LEDC_CHANNEL_2

extern const mp_obj_type_t codey_rgbled_board_type;

extern void    codey_rgbled_board_config_t(void); 
extern void    codey_rgbled_board_set_color_t(uint8_t rgb_r, uint8_t rgb_g, uint8_t rgb_b);
extern uint8_t codey_rgbled_board_get_color_intensity_t(uint8_t rgb);
extern void    codey_rgbled_board_stop_t(void);
extern void    codey_rgbled_board_reload_t(void);

#endif /* _CODEY_RGBLED_BOARD_H_ */


