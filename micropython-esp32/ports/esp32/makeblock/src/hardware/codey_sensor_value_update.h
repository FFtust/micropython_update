/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_sensor_value_update.c.
 * @file    codey_sensor_value_update.h
 * @author  fftust
 * @version V1.0.0
 * @date    2017/05/31
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
 * This file is a drive sensor value update module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/05/31      1.0.0              build the new.
 * </pre>
 *
 */

#ifndef _CODEY_SENSOR_VALUE_UPDATE_H_
#define _CODEY_SENSOR_VALUE_UPDATE_H_

extern void codey_sensors_init_t(void);
extern void codey_sensors_update_t(void);
extern void codey_sensors_update_task_t(void *parameter);
extern uint8_t codey_get_button_A_t(void);
extern uint8_t codey_get_button_B_t(void);
extern uint8_t codey_get_button_C_t(void);
extern float codey_get_timer_value_t(void);
extern void codey_reset_timer_t(void);
extern uint32_t codey_timer_value_t(void);
extern uint8_t codey_button_value_t(void);
extern float codey_light_sensor_value_t(void);
extern float codey_sound_sensor_value_t(void);
extern float codey_gyro_sensor_value_t(uint8_t index);
extern float codey_dail_sensor_value_t(void);

#endif /* _CODEY_SENSOR_VALUE_UPDATE_H_ */

