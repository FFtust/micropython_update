/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for neurons_engine_port.c
 * @file    neurons_engine_port.h
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

#ifndef _NEURONS_ENGINE_PORT_H_
#define _NEURONS_ENGINE_PORT_H_

/******************************************************************************
 DEFINE MACRO 
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES 
 ******************************************************************************/ 

/******************************************************************************
 DECLARE PUBLIC FUNCTIONS AND DATAS
 ******************************************************************************/
extern const mp_obj_type_t neurons_engine_port_type;
extern void neurons_engine_read_check_t(uint8_t dev_id, uint8_t block_id, uint8_t sub_id, uint8_t read_cmd_id);
extern void neurons_engine_read_control_init_t();
extern void neurons_engine_read_control_deinit_t();

#endif /* _NEURONS_ENGINE_PORT_H_*/



