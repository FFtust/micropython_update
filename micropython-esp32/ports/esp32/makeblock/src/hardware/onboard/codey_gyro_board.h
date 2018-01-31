/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief   Heard for codey_gyro_board.c
 * @file    codey_gyro_board.h
 * @author  fftust
 * @version V1.0.0
 * @date    2017/03/20
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
 * This file is a drive gyro_board module.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 *  fftust            2017/03/20        1.0.0            build the new.
 *  Mark Yan          2017/03/31        1.0.0            update for available version.
 * </pre>
 *
 */

#ifndef _CODEY_GYRO_BOARD_H_
#define _CODEY_GYRO_BOARD_H_

/* Exported macro ------------------------------------------------------------*/
#define I2C_ERROR                  (-1)
#define GYRO_DEFAULT_ADDRESS       (0x68)

#define GYRO_INT_PIN_CFG           (0x37)
#define GYRO_INT_ENABLE            (0x38)
#define GYRO_INT_STATUS            (0x3A)

#define I2C_SCL_IO              (18)    /*!< gpio number for I2C master clock */
#define I2C_SDA_IO              (19)    /*!< gpio number for I2C master data  */
#define I2C_NUM                 I2C_NUM_1 /*!< I2C port number for master dev */
#define I2C_TX_BUF_DISABLE      (0)  /*!< I2C master do not need buffer */
#define I2C_RX_BUF_DISABLE      (0)   /*!< I2C master do not need buffer */
#define I2C_FREQ_HZ             (100000) /*!< I2C master clock frequency */

#define WRITE_B                 I2C_MASTER_WRITE /*!< I2C master write */
#define READ_B                  I2C_MASTER_READ /*!< I2C master read */
#define ACK_CHECK_E             (0x1)    /*!< I2C master will check ack from slave*/
#define ACK_CHECK_D             (0x0)    /*!< I2C master will not check ack from slave */
#define ACK_V                   (0x0)    /*!< I2C ack value */
#define NACK_V                  (0x1)    /*!< I2C nack value */

extern const mp_obj_type_t codey_gyro_board_type;

/* this struct is just for neurons */

typedef struct 
{
  float temperature;
  float acc_x;
  float acc_y;
  float acc_z;
  float acc_earth;
  float gyr_x;
  float gyr_y;
  float gyr_z;
  float angle_pitch;
  float angle_yaw;
  float angle_roll;
  float yaw_total;
  uint8_t    shake;
}gyro_data_for_neurons_t;

/* this struct is just for neurons */
extern bool codey_gyro_board_init_t(void);
extern bool codey_gyro_board_deinit_t(void);

extern bool codey_gyro_board_get_ope_i2c_flag_t(void);
extern void codey_gyro_board_get_data_t(void);
extern gyro_data_for_neurons_t codey_gyro_board_read_data_t(void);
extern float codey_gyro_board_get_static_acc_t(void);

#endif /* _CODEY_GYRO_BOARD_H_ */

