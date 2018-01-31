/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_8bit_voice_board.c
 * @file    codey_8bit_voice_board.h
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

#ifndef _CODEY_8BIT_VOICE_BOARD_H_
#define _CODEY_8BIT_VOICE_BOARD_H_

extern const mp_obj_type_t codey_8bit_voice_board_type;
extern void codey_8bit_voice_board_config_t(void);
extern void codey_8bit_voice_board_play_note_directly_t(uint32_t freq, uint32_t time_ms, uint8_t vol);

#endif /* CODEY_8BIT_VOICE_BOARD_H_ */


