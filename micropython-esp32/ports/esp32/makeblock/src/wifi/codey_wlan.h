/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_wlan.c.
 * @file    codey_wlan.h
 * @author  fftust
 * @version V1.0.0
 * @date    2017/04/24
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
 * This file is a drive wlan module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * fftust              2017/04/24    1.0.0            build the new.
 * </pre>
 *
 */

#ifndef _CODEY_WLAN_H_
#define _CODEY_WLAN_H_

#define WIFI_MODE_ERR (255)

extern const mp_obj_type_t codey_wlan_type;
extern void codey_wifi_mac_to_string_t(uint8_t ifx, uint8_t *macstr);

#endif /* _CODEY_WLAN_H_ */