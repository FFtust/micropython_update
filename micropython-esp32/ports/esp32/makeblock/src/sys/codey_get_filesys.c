/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief       Get file system data
 * @file        codey_get_filesys.c
 * @author      Mark Yan
 * @version V1.0.0
 * @date        2017/12/18
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
 * Mark Yan           2017/12/18      1.0.0              Build the new.
 * </pre>
 *
 */
 
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "codey_utils.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "codey_comm_protocol.h"
#include "codey_get_filesys.h"
#include "codey_utils.h"
#include "mb_fatfs/diskio.h"
#include "mb_fatfs/drivers/sflash_diskio.h"


/******************************************************************************
 DEFINE MACROS 
 ******************************************************************************/ 
#undef    TAG
#define   TAG                           ("GET_FILESYS")
                                            
/******************************************************************************
 DEFINE TYPES & CONSTANTS
 ******************************************************************************/  

/******************************************************************************
 DECLARE PRIVATE DATAS
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
void codey_get_filesys(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  uint8_t *buffer;
  uint8_t data_check_sum;
  uint16_t addr;

  if(!data || !len || !output_len || !output_buf)
  {
    return;
  }

  uint16_t read_block_size = 256;
  buffer = heap_caps_malloc(read_block_size, MALLOC_CAP_8BIT);
  memset(buffer, 0, read_block_size);
  addr = codey_read_short(data,0);
  output_buf[0] = data[0];
  output_buf[1] = data[1];
  *output_len = 2;
 
  data_check_sum = (0x0c+(output_buf[0] + output_buf[1])) & 0xff;
  uint32_t sflash_block_addr = SFLASH_START_ADDR + addr * read_block_size;

  uint8_t ret = spi_flash_read(sflash_block_addr, buffer, read_block_size);

  if(ret != RES_OK)
  {
    printf("read error! the ret is %d\n", ret);
    heap_caps_free(buffer);
    return;
  }

  for(int16_t i=0;i<read_block_size;i++)
  {
    output_buf[i+2] = *(buffer+i);
    data_check_sum = (data_check_sum + output_buf[i+2]) & 0xff;
  }
  *output_len += read_block_size;
  heap_caps_free(buffer);
}

