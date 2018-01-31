/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   The basis of the function for makeblock.
 * @file    codey_ringbuf.c
 * @author  Mark Yan
 * @version V1.0.0
 * @date    2017/03/30
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
 * This file include some system function.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  Mark Yan           2017/03/30      1.0.0              build the new.
 * </pre>
 *
 */
 
#include <stdint.h>
#include <stdio.h>
#include "codey_ringbuf.h"

/*****************************************************************
 DEFINE MACROS
******************************************************************/

/*****************************************************************
 DEFINE TYPES & CONSTANS
******************************************************************/

/*****************************************************************
DEFINE PRIVATE DATAS
******************************************************************/

/*****************************************************************
DECLARE PRIVATE FUNCTIONS
******************************************************************/

/*****************************************************************
 DEFINE PUBLIC FUNCTIONS
******************************************************************/
uint8_t codey_ring_buf_get(codey_ring_buf_t *r)
{
  uint8_t ret;
  
  ret = r->buf[r->iget++];
  r->iget &= r->size;
  return ret;
}

void codey_ring_buf_put(codey_ring_buf_t *r, uint8_t val) 
{
  r->buf[r->iput++] = val;
  r->iput &= r->size;
}

