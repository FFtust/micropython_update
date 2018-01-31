/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief       Header file for super varible
 * @file        codey_super_var.h
 * @author      leo
 * @version     V1.0.0
 * @date        2017/09/13
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
 * leo             2017/09/13         1.0.0              build the new.
 * </pre>
 *
 */

#ifndef _CODEY_SUPER_VAR_H_
#define _CODEY_SUPER_VAR_H_

/******************************************************************************
 INCLUDE  
 ******************************************************************************/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "codey_comm_protocol.h"

/******************************************************************************
 DEFINE MACRO 
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES 
 ******************************************************************************/ 

/******************************************************************************
 DECLARE PUBLIC FUNCTIONS AND DATAS
 ******************************************************************************/
extern const mp_obj_type_t codey_super_var_type;
extern void codey_super_var(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len);
extern void codey_super_var_clear(void);

/*
1 )
NOTE: MUST import and check the codey.py
Assumption that codey.py define these function:

# super_var
def set_variable( name, value ):
    s_var = super_var( name )
    if ( s_var ):
        return s_var.set_value( value )
    else:
        return False

def get_variable( name ):
    g_var = super_var( name )
    if ( g_var ):
        return g_var.get_value()
    else:
        return None

2 ) And use it as:

import time
import codey

while( True ):
	codey.get_variable( "a" )
	codey.set_variable( "b", 123 )
	time.sleep( 1 )

*/

#endif /* _CODEY_SUPER_VAR_H_ */


