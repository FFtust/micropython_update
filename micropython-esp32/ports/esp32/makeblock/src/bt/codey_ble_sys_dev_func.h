/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     Heard for codey_ble_sys_dev_func.c.
 * @file      codey_ble_sys_dev_func.h
 * @author    Leo lu
 * @version    V1.0.0
 * @date      2017/04/27
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
 * This file setup ble to device and start ble device with a makeblock profile.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * Leo lu             2017/08/10         1.0.0              Initial version
 * </pre>
 *
 */

#ifndef _CODEY_BLE_SYS_DEV_FUNC_
#define _CODEY_BLE_SYS_DEV_FUNC_

typedef void (*ble_connected_indicate_func)(void);

extern int codey_ble_enter_sys_dev_mode(void);
extern int codey_ble_dev_get_char(uint8_t *c);
extern void codey_ble_dev_get_data(uint8_t *out_buf, uint16_t *len);
extern int codey_ble_dev_put_data(uint8_t *data, uint16_t len);
extern void codey_ble_register_connected_func(ble_connected_indicate_func func);
#endif /* _CODEY_BLE_SYS_DEV_FUNC_ */

