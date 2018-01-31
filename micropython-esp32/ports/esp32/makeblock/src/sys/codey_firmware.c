/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief    The basis of the function for makeblock.
 * @file      codey_firewarem.c
 * @author    Leo
 * @version V1.0.0
 * @date      2017/10/16
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
 *   Leo              2017/10/16      1.0.0            build the new.
 *   Leo              2017/10/17      1.0.1            1) Fix super var update string vaue bug
 *                                                     2) Add get mac id
 *                                                     3) protocol to ble channel
 *   Leo              2017/10/18      1.0.2            firmware: 23.01.002
 *                                                     1) Fix a bug in get mac, missing a byte data when return
 *                                                     2) Add "get firmware protocol"
 *                                                     
 *   Leo              2017/10/18      1.0.3            firmware: 23.01.003
 *                                                     1) Fix a bug in report sensor: do NOT report when sensor data 
 *                                                        keep the same but request report again.
 *                                                     2) Suport varible supporting list varible
 *                                                     3) Fix a bug on message evnet, message string must contain a terminal char
 *
 *   Leo              2017/10/19      1.0.4            1) Fix a bug in super_var. A object return by mp_obj_new_xxx()
 *                                                        may been free in python, but a C program can NOT be awar of 
 *                                                        that when useing a object which is nolonger exist.
 *                                                     2) Fix a bug on interrupt mode, when there is a neurons comand needed to send.
 *
 *                                                     3) Del super_var list supporting
 *
 *  leo             2017/10/20        1.0.5            1) Fix a bug: more than one thread accessing the same super variable
 *                                                        need to protect
 *                                                     2) ble tx using block sending method for connecting with android
 *                                                      
 *  leo             2017/10/23        1.0.1            1) Add interface of getting stattion connected status
 *                                                     2) Update urequests.py
 *                                                     3) Update codey_wlan_board.py
 *                                                     4) Update wifi interface in codey.py to support IOT
 *
 *  Yan             2017/10/24        1.0.5            1) soc_memory_layout.c, save a memory from ble
 *
 *  Leo             2017/10/24        1.0.6            1) reback for "soc_memory_layout.c, save a memory from ble"
 *                                                     2) fix bug in rocky.py interface
 *
 *  fftust          2017/10/25        1.0.7            1) optimize the event mechanism
 *                                                     2) fix bug: rocky motion invalid when time less than 1s, eg: 0.5
 *                                                     3) change the time to set wifi STA connected flag 
 *
 * Leo lu             2017/10/30      1.0.8            1) Fix a bug of division by zero
 *                                                     2) Fix a bug of memory leak
 *
 * Leo lu             2017/10/30      1.0.9            1) neuron command FIFO lock to just one 
 *                                                     2) Push command must wait for FIFO
 *
 * Leo lu             2017/10/30      1.0.10           1) Suppor MQTT in python
 *
 * Leo lu             2017/11/02      1.0.11           1) Fix a bug in music play to stop, adding MP_THREAD_GIL_EXIT() & MP_THREAD_GIL_ENTER
 *                                                        when wating to stop
 *
 * Leo lu             2017/11/03      1.0.12           1) I2C bus adding a bus_check to avoid bus lock by slave
 * Leo lu             2017/11/03      1.0.13           1) Add dtr request protocol interface
 * Leo lu             2017/11/03      1.0.14           1) Music play lock fatfs sem in C, NOT in python interface
 * Leo lu             2017/11/03      1.0.15           1) Fix a bug in music play death lock in play_list_sem & fatfs_sem
 * Leo lu             2017/11/03      1.0.16           1) Music do not mix, when play a new one, the last music will stop
 * Leo lu             2017/11/17      1.0.17           1) Add communication channel protocol; Adding channel tag to protocol process function
 *                                                     2) Add ready notify protocol
 * Leo lu             2017/12/14      1.0.18           1) Fix a bug in codey_neurons_deal.c, data would stay in buffer when a processing
 *                                                       take more than 100ms, as we define the max dealwith time is 100ms. Now we must
 *                                                       continue to read data from buffer untill all the data have been read out.
 *                                                     2) Fix a bug in ready notify protocol
 * Leo lu             2017/12/15      1.0.19           1) Add baudrate setting command
 * Leo lu             2017/12/19      1.0.20           1) Add power level sensor
 * Leo lu             2017/12/27      1.0.21           1) Fia a bug in play to stop, considering play buffer duration time
 * Leo lu             2017/12/27      1.0.22           1) Reuse INT data, do NOT change INT to FLOAT anymore.
                                                       2) Add super var clear interface, after python restart
 * </pre>
 *
 */

/**********************************************************************************************************************************/
 /*
 * \par History:
 * <pre>
 * `<Version>`                `<Time>`             `<Author>`               `<Descr>`
 *  23.01.006.V1.0alpha2      2017/10/25             fftust                  1) optimize the event mechanism
 *                                                                           2) fix bug: rocky motion invalid when time less than 1s, eg: 0.5
 *                                                                           3) change the time to set wifi STA connected flag 
 *
 * 23.01.T01                  2017/10/26                                     1) optimize the stability of system
 *                                                                           2) change the threshold of sensor value
 *                                                                           3) change "printf" to "ESP_LOG" 
 *
 * 23.01.T01                 2017/10/27             leo                      1) Update event mechanism to avoiding reentry to take sem
 *
 * 23.01.T02                 2017/10/29             fftust                   1) fix bug: ledmatrix
 *                                                                           2) set restart to soft restart after update script
 * 23.01.T03                 2017/10/31             fftust                   3) add function to get lightness for rocky  
 * 23.01.T04                 2017/10/31             fftust                   1) threads safe 
 * 23.01.T05                 2017/10/31             fftust                   1) threads safe & soft reboot 
 * 23.01.T06                 2017/11/08             fftust                   1) debug
 * 23.01.T07                 2017/11/09             fftust                   1) debug: gyro send angle value by protocol, direction wrong
 *                           2017/11/09             fftust                   2) debug: animation show inversely 
 *                           2017/11/09             fftust                   3) close logs out
 *                           2017/11/09             fftust                   3) debug: set volume error
 * 23.01.T07                 2017/11/10             leo                      1) optimize uart receiving
 *                           2017/11/10             fftust                   2) close wifi and ble 
 * 23.01.T08                 2017/11/17             leo                      1) Add communication channel protocol; Adding channel tag to protocol process function
 *                                                                           2) Add ready notify protocol
 * 23.01.T09                 2017/11/23             fftust                   1) wifi sta mode optimize
 *                                                                           2) codes tidy: change "mb" to "codey"  && change "( )" to "()"
 *                           2017/11/27             fftust                   1) delete pyton library files that not used
 *                                                                           2) debug: send neurons command ti fast will block the communication task 
 *                           2017/11/27             fftust                   1) codes tidy: change file name "mb_" to "codey"
 *                           2017/11/27             fftust                   1) add product test codes(will be deleted later)
 *                                                                           2) neurons engine lib update to a temp version alpha4
 *                                                                              for product test
 *                           2017/11/27             fftust                   1) neurons engine lib update to a temp version alpha5, for new color sensor
 *                                                                           2) update rocky and the neurons library about rocky 
 *                           2017/11/28             fftust                   1) add report mode and delete readind command
 *                           2017/11/30             fftust                   1) modify the name of static variables to "s_xxx"
 *                           2017/12/06             fftust                   1) add codey rocky rotate function
 *                                                                           2) codes tidy: optimize the position of private and public
 *                                                                              variables and functions
 *                                                                           3) delete warining codes
 * 23.01.T10                 2017/12/11             fftust                   1) change the events trigger condition
 *                                                                           2) optimize file system: add test macro 
 *                           2017/12/11             fftust                   1) chang neurons engine version "V1.0alpha5" to "T01"
 *                                                                              content not changed
 * 23.01.T10                 2017/12/16             fftust                   1) update the python functions to fit T10 demand
 *                                                                           2) rocky sensor report changed: if no function be called to get sensor value 
 *                                                                              from rocky with 500ms(heart package interval), stop the report mode
 *                                                                           3) optimize the event mechanism, most in python 
 * 23.01.T10                 2017/12/18             fftust                   1) chang the way to read data from rocky, use event group to wait
 *                                                                           2) add music node table
 * 23.01.T10                 2017/12/18             fftust                   1) fix a bug about tilt checking
 *                                                                           2) add semaphore for wifi connecting and request
 *                                                                           3) change the API about playong notes
 * 23.01.T10                 2017/12/20             fftust                   1) delete the voice when script updates
 *                                                                           2) change the priority of rmt task to be the same with sensor update task
 *                                                                           3) change several api names 
 * 23.01.T10                 2017/12/21             fftust                   1) change "codey.timer" to "codey.time" 
 *                                                                              "codey.reset_timer" to "codey.reset_time"
 *                                                                              "codey.light_sensor" to "light_strength"
 *                                                                              "codey.sound_sensor" to "sound_strength"
 * 23.01.T10                 2017/12/21             fftust                   1) debug: codey time the same with time module
 *                                                                           2) disable FTP function
 * 23.01.T10                 2017/12/22             fftust                   1) change neurons heart interval from 500 to 200, just a temporary solution
 *                                                                              to avoid python executed befor neurons start
 *                                                                           2) optimize software when execute codey_script_update_pre_operation()
 * 23.01.001                 2017/12/29             fftust                   1) T10 debug: optimize dealing with inligal unlawful parameters
 *                                                                           2) modify the definition of millis to xTaskGetTickCount
 *                                                                           3) change wifi mode from sta_ap to sta
 * 23.01.001                 2018/01/03             fftust                   1) add several indication
 *                                                                           2) optimize the output of codey angle                   
 *                                                                           3) change version to 23.01.001.001
 * 23.01.001                 2018/01/08             fftust                   1) T11 debug: when error in user thread callback occured,
 *                                                                              system restart(RAM freed when error occured)
 *                                                                           2) optimize the calculation of music volume
 *                                                                           3) codey.show(): add limit to lenth of string(128)
 *                                                                           4) add function codey_give_data_recv_sem_from_isr(),
 *                                                                              take place of codey_give_data_recv_sem() in uart ISR
 * 23.01.001                 2018/0115              fftust                   1) add new music files and command "make test" -- generate codes for test
 * </pre>* </pre>                                    
 *
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "codey_sys.h"
#include "codey_firmware.h"
#include "neurons_engine_list_maintain.h"

/********************************************************************
 DEFINE MACROS
 ********************************************************************/
 
/********************************************************************
 DEFINE PRIVATE DATAS
 ********************************************************************/

/********************************************************************
 DEFINE PUBLIC DATAS
 ********************************************************************/
char g_firmware[32] = "23.01.001";

/********************************************************************
 DEFINE PUBLIC FUNCTIONS
 ********************************************************************/
const char *codey_get_firmware(void)
{
  uint8_t str_len = 0;
  char version_temp[32];
  neurons_engine_get_version_t(version_temp, &str_len);
  if(str_len < 20)
  {
    g_firmware[9] = '.';
    memcpy(&g_firmware[10], version_temp, (str_len));
  }

  return g_firmware;
}

void codey_comm_get_firmware(channel_data_tag_t chn, uint8_t *data, uint32_t len, uint8_t *output_buf, uint32_t *output_len)
{
  if(!output_len || !output_buf)
  {
    return;
  }
  strcpy((char *)output_buf, g_firmware);
  *output_len = strlen(g_firmware);

}

