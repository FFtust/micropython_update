/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_battery_check.c
 * @file    codey_battery_check.h
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
 * This file is a drive hall_sensor module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust             2017/05/31     1.0.0              build the new.
 * </pre>
 *
 */

#ifndef _CODEY_BATTERY_CHECK_H_
#define _CODEY_BATTERY_CHECK_H_ 

#define CODEY_BATTERY_CHECK_CHANNEL ADC1_CHANNEL_5 //GPIO 33
#define CODEY_POWER_CONTROL_IO (15)
#define CODEY_POWER_KEY_IO (34)

#define CODEY_POWER_ON_SET_STATUS (1)
#define CODEY_POWER_OFF_SET_STATUS (0)

#define CODEY_POWER_CHECK_PRESSED_STATUS (0)
#define CODEY_POWER_CHECK_RELEASED_STATUS (1)

#define CODEY_POWER_SWITCH_CONTINUE_TIME_MS (100)
#define CODEY_POWER_OFF_START_TIME_MS (2000)

extern const mp_obj_type_t codey_battery_check_type;

extern void    codey_battery_check_config_t(void);
extern void    codey_board_power_on_set_t(void);
extern void    codey_board_power_off_set_t(void);
extern bool    codey_board_is_power_key_pressed_t(void);
extern void    codey_board_power_status_check_t(void);
extern float   codey_battery_check_get_value_t(void);
extern float   codey_battery_check_get_voltage_t(void);
extern uint8_t codey_battery_check_get_capacity(void);
extern void    codey_battery_low_capacity_check(void);

#endif /* _CODEY_BATTERY_CHECK_H_ */



