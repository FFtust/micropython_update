/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Ring buffer
 * @file    codey_ringbuf.h
 * @author  Leo lu
 * @version V1.0.0
 * @date    2017/05/17
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
 * This file is a header for ring function.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *   Leo lu          2017/05/17       1.0.0              build the new.
 * </pre>
 *
 */
  
#ifndef _CODEY_RINGBUF_H_
#define _CODEY_RINGBUF_H_
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/*****************************************************************
 DEFINE MACROS
******************************************************************/
#define RING_BUF_INIT(r, b, s)    {\
                                      (r)->buf = b;\
                                      (r)->size = s - 1;\
                                      (r)->iget = (r)->iput = 0;\
                                    }

#define RING_BUF_IS_FULL(r)       ((((r)->iput + 1) & (r)->size) == (r)->iget)
#define RING_BUF_IS_EMPTY(r)      ((r)->iput == (r)->iget ? 1 : 0)

/*****************************************************************
 DEFINE TYPES & CONSTANS
******************************************************************/
typedef struct codey_ring_buf {
  uint32_t size;
  uint32_t iput;
  uint32_t iget;
  uint8_t *buf;
} codey_ring_buf_t;

/*****************************************************************
 DECLAREE PUBLIC FUNCTIONS
******************************************************************/
/*
*/
uint8_t codey_ring_buf_get(codey_ring_buf_t *r);

/*
*/
void codey_ring_buf_put(codey_ring_buf_t *r, uint8_t val);

#endif /* _CODEY_RINGBUF_H_ */