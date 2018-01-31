# 概要
## 1. 该文件为神经元库依赖文件， 所有的神经元模块将在此文件中以固定格式描述，然后调用block_lib_generate_new.py文件生成所需的16进制文件：neurons_engine_lib.bin
## 2. 目前模块描述分为两种， 一种是普通模块：支持最多5条控制命令和5条读值命令， 如直流电机等  另一种是特殊模块： 支持最多256条控制命令和256条读值命令；
## 3. 在参数处理上， 两种类型的模块命令参数的描述最多占用5个字节， 具体说明如下：
##    a) 参数的每个字节低四位代表参数类型， 最多支持16种类型的数据， 如0x01代表TYPE_BYTE_8
##    b) 参数的每个字节高四位代表该类型个数或其他， 最多支持10个， 如0xa1代表10个TYPE_BYTE_8类型
##    c) 当参数的高四位值大于10时， 代表其他特殊的命令形式， 如0xb1代表连续n个TYPE_BYTE_8类型， n在前面参数中指定，最多接收127个字符， 具体如下

##    0xb0 表示连续n位， n在前一个参数指定，(注: 一定要是前一个参数指定), 低四位指定连续参数的类型， 如0xb1代表连续n个TYPE_BYTE_8类型
##         当需要连续接收n个字符时， 需要指定要分配的最大内存， 因此指定该位后一位为要分配的最大数据个数（非字节），0 - 255， 这个数据提供的意思在于下位机根据此数据分配内存
## 4. 增加普通模块时， 按大的类别划分， 在相对应的区域按固定格式增加新的模块， 同时修改该区域最前面的 模块数量宏， 如 NEU_MOVEMENT_MODULE_NUM， 同时在大的类别中添加该模块信息变量， 如
##    在NEURONS_MOVEMENT_LIB中添加新的该类别模块； 特殊模块只在NEURONS_SPECIAL_LIB 按格式添加该特殊模块即可；

TYPE_BYTE_8   = 0x01
TYPE_byte_16  = 0x02
TYPE_SHORT_16 = 0x03
TYPE_short_24 = 0x04
TYPE_long_40  = 0x05
TYPE_float_40 = 0x06

TYPE_ALTERABLE_LEN = 0xb0
##################################################################################################
## 通用类 0x61, (0x01, 0x02, 0x03)

### 设置命令反馈 0x01
NEU_COMMON_SET_CMD_FEEDBACK = {"name": "set_feedback", "type" : 0x61, "sub_type": 0x01, \
                                "respond_info" : (), \
                                "command_info" : ((0x00, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00),) \
                               }

### 设置指示灯颜色 0x02
NEU_COMMON_SET_INDICATOR_LED = {"name": "set_indicate_led", "type" : 0x61, "sub_type": 0x02, \
                                "respond_info" : (), \
                                "command_info" : ((0x00, TYPE_SHORT_16, TYPE_SHORT_16, TYPE_SHORT_16, 0x00, 0x00),) \
                               }
### 上位机查找对应模块 0x03
NEU_COMMON_FIND_BLOCK = {"name": "find_block", "type" : 0x61, "sub_type": 0x03, \
                         "respond_info" : (), \
                         "command_info" : ((0x00, 0x00, 0x00, 0x00, 0x00, 0x00),) \
                        }

## ***************************************************************************************************************************************
## 运动类 0x62 , (0x01, 0x02, 0x03, 0x04)
## 每当在该类别下增加一个新的模块时， 将下面的变量加1
NEU_MOVEMENT_MODULE_NUM = 4
### 单直流电机 0x01MOTOR
NEU_MOVEMENT_SINGLE_DC_MOTOR = {"name": "single_dc_motor", "type" : 0x62, "sub_type": 0x01, \
                                "respond_info" : (), \
                                "command_info" : ((0x01, TYPE_byte_16, 0x00, 0x00, 0x00, 0x00),) \
                                }


### 双直流电机 0x02
NEU_MOVEMENT_DOUBLE_DC_MOTOR = {"name": "double_dc_motor", "type" : 0x62, "sub_type": 0x02, \
                                "respond_info" : (), \
                                "command_info" : ((0x01, TYPE_byte_16, TYPE_byte_16, 0x00, 0x00, 0x00),) \
                               }

### 9g舵机 0x03
NEU_MOVEMENT_9G_SERVO = {"name": "9g_servo", "type" : 0x62, "sub_type": 0x03, \
                         "respond_info" : (), \
                         "command_info" : ((0x01, TYPE_SHORT_16, 0x00, 0x00, 0x00, 0x00),) \
                        }

### 编码电机 0x04
NEU_MOVEMENT_SINGLE_CODE_MOTOR = {"name": "single_code_motor", "type" : 0x62, "sub_type": 0x04, \
                                  "respond_info" : ((0x05, TYPE_short_24, TYPE_long_40, 0x00, 0x00, 0x00), ), \
                                  "command_info" : ((0x01, TYPE_short_24, TYPE_long_40, 0x00, 0x00, 0x00), (0x02, TYPE_short_24, TYPE_long_40, 0x00, 0x00, 0x00), \
                                                    (0x03, TYPE_short_24, 0x00, 0x00, 0x00, 0x00), (0x04, TYPE_byte_16, 0x00, 0x00, 0x00, 0x00), \
                                                    (0x05, 0x00, 0x00, 0x00, 0x00, 0x00)) \
                                 }
## ***************************************************************************************************************************************
## 传感类 0x63, (0x01 -- 0x0d)
## 每当在该类别下增加一个新的模块时， 将下面的变量加1
NEU_SENSORS_MODULE_NUMBER = 13
### 温度传感器 0x01
NEU_SENSORS_TEMPERATURE = {"name": "temperature", "type" : 0x63, "sub_type" : 0x01,  \
                           "respond_info" : ((0x01, TYPE_float_40, 0x00, 0x00, 0x00, 0x00), ), \
                           "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00), )  \
                          } 

### 光线强度传感器 0x02
NEU_SENSORS_LIGHT_SENSOR = {"name":"light_sendor", "type" : 0x63, "sub_type" : 0x02,  \
                            "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), ), \
                            "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00), ) \
                           }
### 超声波传感器 0x03
NEU_SENSORS_ULTRASONIC_SENSOR = {"name": "ultrasonic", "type" : 0x63, "sub_type": 0x03, \
                                 "respond_info" : ((0x01, TYPE_float_40, 0x00, 0x00, 0x00, 0x00), ), \
                                 "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00), ) \
                                }
### 巡线传感器 0x04
NEU_SENSORS_LINEFOLLOWER_SENSOR = {"name": "linefollower", "type" : 0x63, "sub_type": 0x04, \
                                   "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), ), \
                                   "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00),) \
                                  }
### 颜色传感器 0x05
NEU_SENSORS_COLOR_SENSOR = {"name": "color_sensor", "type" : 0x63, "sub_type": 0x05, \
                            "respond_info" : ((0x01, TYPE_SHORT_16, TYPE_SHORT_16, TYPE_SHORT_16, 0x00, 0x00),), \
                            "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00),) \
                           }
### 加速度和陀螺仪传感器 0x06
NEU_SENSORS_GYRO_ACCELERATION_SENSOR = {"name": "gyro_acceleration_sensor", "type" : 0x63, "sub_type": 0x06, \
                                        "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), ), \
                                        "command_info" : ((0x01, TYPE_BYTE_8, TYPE_BYTE_8, TYPE_long_40, 0x00, 0x00),) \
                                       }
### 温湿度传感器 0x07
NEU_SENSORS_TEMPERATURE_HUMIDITY_SENSOR = {"name": "tem_humidity_sensor", "type" : 0x63, "sub_type": 0x07, \
                                           "respond_info" : ((0x01, TYPE_byte_16, TYPE_BYTE_8, 0x00, 0x00, 0x00), ), \
                                           "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00), ) \
                                          }
### 土壤湿度传感器 0x08
NEU_SENSORS_SOIL_HUMIDITY_SENSOR = {"name": "soil_humidity_sensor", "type" : 0x63, "sub_type": 0x08, \
                                    "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), ), \
                                    "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00), ) \
                                   }
### 雨滴传感器 0x09
NEU_SENSORS_RAINDROP_SENSOR = {"name": "raindrop_sensor", "type" : 0x63, "sub_type": 0x09, \
                                "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), ), \
                                "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00), ) \
                              }
### 风速传感器 0x0a
NEU_SENSORS_WIND_SPEED_SENSOR = {"name": "wind_speed_sensor", "type" : 0x63, "sub_type": 0x0a, \
                                 "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), ), \
                                 "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00), ) \
                                }
### 颗粒物浓度传感器 0x0b
NEU_SENSORS_PARTICLE_CONCENTRATION_SENSOR = {"name": "particle_concentration_sensor", "type" : 0x63, "sub_type": 0x0b, \
                                             "respond_info" : ((0x01, TYPE_SHORT_16, 0x00, 0x00, 0x00, 0x00), (0x02, TYPE_SHORT_16, 0x00, 0x00, 0x00, 0x00), \
                                                               (0x03, TYPE_SHORT_16, 0x00, 0x00, 0x00, 0x00), (0x04, TYPE_SHORT_16, 0x00, 0x00, 0x00, 0x00)), \
                                             "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00), (0x02, 0x00, 0x00, 0x00, 0x00, 0x00), (0x03, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                                               (0x04, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00)) \
                                            }
### 人体红外传感器 0x0c
NEU_SENSORS_HUMEN_IR_SENSOR = {"name": "humen_ir_sensor", "type" : 0x63, "sub_type": 0x0c, \
                               "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), ), \
                               "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00),) \
                              }
### 声音传感器模块 0x0d
NEU_SENSORS_SOUND_SENSOR = {"name": "single_dc_motor", "type" : 0x63, "sub_type": 0x0d, \
                            "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00),), \
                            "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00),) \
                           }
## ***************************************************************************************************************************************
## 控制类 0x64, (0x01 -- 0x08)
## 每当在该类别下增加一个新的模块时， 将下面的变量加1
NEU_CONTROLERS_MODULE_NUMBER = 8
### 电位器 0x01
NEU_CONTROLERS_DAIL = {"name": "dail", "type" : 0x64, "sub_type": 0x01, \
                       "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00),), \
                       "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00),) \
                      }
### 按键 0x02
NEU_CONTROLERS_BUTTON = {"name": "button", "type" : 0x64, "sub_type": 0x02, \
                         "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00),), \
                         "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00),) \
                        }
### 旋转编码器 0x03
NEU_CONTROLERS_ROTARY_ENCODER = {"name": "rotary_encoder", "type" : 0x64, "sub_type": 0x03, \
                                 "respond_info" : (), \
                                 "command_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00),) \
                                }
### makey makey 0x04
NEU_CONTROLERS_MAKEY_MAKEY = {"name": "makey_makey", "type" : 0x64, "sub_type": 0x04, \
                              "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00),), \
                              "command_info" : () \
                             }
### 触摸按键 0x05 
NEU_CONTROLERS_TOUCH_KEY = {"name": "touch_key", "type" : 0x64, "sub_type": 0x05, \
                            "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00),), \
                            "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00),) \
                           }
### 自锁开关 0x06
NEU_CONTROLERS_SELF_LOCKING_BUTTON = {"name": "self_locking_key", "type" : 0x64, "sub_type": 0x06, \
                                      "respond_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00),), \
                                      "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00),) \
                                     }
### 摇杆 0x07
NEU_CONTROLERS_ROCKER = {"name": "rocker", "type" : 0x64, "sub_type": 0x07, \
                         "respond_info" : ((0x01, TYPE_byte_16, TYPE_byte_16, 0x00, 0x00, 0x00),), \
                         "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00),) \
                        }
### 轻触开关 0x08
NEU_CONTROLERS_LIGHT_TOUCH_KEY = {"name": "light_touch_key", "type" : 0x64, "sub_type": 0x08, \
                                  "respond_info" : (), \
                                  "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00),) \
                                 }
## ***************************************************************************************************************************************
## 显示类 0x65, (0x01 -- 0x06)
## 每当在该类别下增加一个新的模块时， 将下面的变量加1
NEU_DISPLAY_MODULE_NUMBER = 6
### 四位七段数码管 0x01
NEU_DISPLAY_4_BIT_7SEG_TUBE = {"name": "7_seg_tube", "type" : 0x65, "sub_type": 0x01, \
                               "respond_info" : (), \
                               "command_info" : ((0x01, TYPE_float_40, 0x00, 0x00, 0x00, 0x00),) \
                              }
### 彩色RGB灯 0x02
NEU_DISPLAY_COLORFUL_RGB_LED = {"name": "rgb_led", "type" : 0x65, "sub_type": 0x02, \
                                "respond_info" : (), \
                                "command_info" : ((0x01, TYPE_SHORT_16, TYPE_SHORT_16, TYPE_SHORT_16, 0x00, 0x00),) \
                                }
### 彩色灯带 0x03
NEU_DISPLAY_RGB_LED_STRIP = {"name": "rgb_led_strip", "type" : 0x65, "sub_type": 0x03, \
                             "respond_info" : (), \
                             "command_info" : ((0x01, TYPE_BYTE_8, 0x30 | TYPE_SHORT_16, 0x00, 0x00, 0x00),\
                                               (0x02, 0x30 | TYPE_BYTE_8, TYPE_ALTERABLE_LEN | TYPE_BYTE_8, 0x00, 0x00, 0x00),\
                                              ) \
                              }
### 彩色8*8led面板 0x04
NEU_DISPLAY_RGB_LED_MATRIX_8_8 = {"name": "rgb_led_matrix", "type" : 0x65, "sub_type": 0x04, \
                                  "respond_info" : (), \
                                  "command_info" : ((0x01, 0x20 | TYPE_long_40, 0x30 | TYPE_SHORT_16, 0x00, 0x00, 0x00), \
                                                    (0x02, TYPE_BYTE_8, 0x30 | TYPE_SHORT_16, 0x00, 0x00, 0x00), \
                                                    (0x03, 0x20 | TYPE_BYTE_8, TYPE_ALTERABLE_LEN | TYPE_BYTE_8, 0x00, 0x00, 0x00), \
                                                    (0x04, 0x20 | TYPE_BYTE_8, TYPE_ALTERABLE_LEN | TYPE_BYTE_8, 0x00, 0x00, 0x00), \
                                                    (0x05, 0x20 | TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00)) \
                                 }
### OLED显示屏 0x05
NEU_DISPLAY_OLED = {"name": "oled", "type" : 0x65, "sub_type": 0x05, \
                    "respond_info" : (), \
                    "command_info" : ((0x01, 0x20 | TYPE_BYTE_8, TYPE_SHORT_16, TYPE_ALTERABLE_LEN | TYPE_BYTE_8, 0x00, 0x00), \
                                      (0x02, 0x20 | TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                     )\
                   }
### 冷光管 0x06
NEU_DISPLAY_COLD_LIGHT_PIPE = {"name": "cold_light_pipe", "type" : 0x65, "sub_type": 0x06, \
                               "respond_info" : (), \
                               "command_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00),) \
                              }
## **************************************************************************************************************************************
## 声音类 0x66, (0x01, 0x02)
## 每当在该类别下增加一个新的模块时， 将下面的变量加1
NEU_SOUND_MODULE_NUMBER = 1
### 蜂鸣器模块
NEU_SOUND_BUZZER = {"name": "buzzer", "type" : 0x66, "sub_type": 0x02, \
                    "respond_info" : (), \
                    "command_info" : ((0x01, TYPE_short_24, TYPE_BYTE_8, 0x00, 0x00, 0x00),) \
                   }
## ***************************************************************************************************************************************
# 其他类0x67


####说明
# 特殊传感器有 陀螺仪、 颗粒浓度检测计、 彩色灯带、led面板、OLED显示屏、rocky
### rokey the command 0x15 reserved for HSV 
### detail information : http://km.makeblock.com/pages/viewpage.action?pageId=13799782
###  ID对应的数据类型说明
###  0x01   获取RGB                   
###  0x02   获取颜色      
###  0x03   获取环境光强
###  0x04   获取灰度值
###  0x05   获取可见光发射强度
###  0x06   获取红外光反射强度
###  0x07   判读灰度是否在指定范围内
###  0x08   判断是否有障碍
###  0x09   设置RGB灯                      None respond
###  0x0A   获取左右电机采样电流
###  0x0B   预留

###  0x0C   小车停止                       None respond
###  0x0D   小车前进                       None respond
###  0x0E   小车后退                       None respond
###  0x0F   小车右转                       None respond
###  0x10   小车左转                       None respond
###  0x11   电机控制                       None respond
###  0x12   差速右转                       None respond
###  0x13   差速左转                       None respond
###  0x14   单电机控制                     None respond

NEU_ROCKY_S = {"name": "rokey", "type" : 0x63, "sub_type": 0x10, \
               "respond_info" : ((0x01, 0x30 | TYPE_SHORT_16, 0x00, 0x00, 0x00, 0x00), \
                                 (0x02, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                 (0x03, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                 (0x04, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                 (0x05, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                 (0x06, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                 (0x07, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                 (0x08, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                 (0x09, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                 (0x0a, 0x20 | TYPE_SHORT_16, 0x00, 0x00, 0x00, 0x00), \
                                 (0x0b, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                 (0x0c, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                 (0x0d, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                 (0x0e, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                 (0x0f, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                 (0x10, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                 (0x11, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                 (0x12, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                 (0x13, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                 (0x14, 0x00, 0x00, 0x00, 0x00, 0x00), \
                               ), \
              "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x02, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x03, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x04, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x05, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x06, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x07, 0x20 | TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x08, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x09, 0x30 | TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x0a, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x0b, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x0c, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x0d, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x0e, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x0f, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x10, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x11, 0x40 | TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x12, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x13, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x14, 0x30 | TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                               ), \
            }
### mp3
NEU_SOUND_MP3_S = {"name": "mp3", "type" : 0x66, "sub_type": 0x01, \
                  "respond_info" : (), \
                  "command_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                    (0x02, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                    (0x03, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                    (0x04, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                    (0x05, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                    (0x06, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                    (0x07, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                    (0x08, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                    (0x09, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00) \
                               ) \
                  }

### IO扩展板
NEU_IO_EXTEND_BOARD_S = {"name": "io_extend", "type" : 0x67, "sub_type": 0x02, \
                         "respond_info" : ((0x02, TYPE_BYTE_8, TYPE_BYTE_8, 0x00, 0x00, 0x00), \
                                           (0x04, TYPE_BYTE_8, TYPE_SHORT_16, 0x00, 0x00, 0x00), \
                                           (0x07, TYPE_BYTE_8, TYPE_short_24, TYPE_BYTE_8, TYPE_ALTERABLE_LEN | TYPE_byte_16, 0x20), \
                                    ), \
                         "command_info" : ((0x01, TYPE_BYTE_8, TYPE_BYTE_8, 0x00, 0x00, 0x00), \
                                           (0x02, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                           (0x03, TYPE_BYTE_8, TYPE_SHORT_16, 0x00, 0x00, 0x00), \
                                           (0x04, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                           (0x06, 0x20 | TYPE_BYTE_8, TYPE_ALTERABLE_LEN | TYPE_byte_16, 0x00, 0x00, 0x00), \
                                           (0x07, TYPE_BYTE_8, TYPE_short_24, TYPE_BYTE_8, 0x00, 0x00), \
                                           (0x08, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00) \
                                     ) \
                  }
## ***************************************************************************************************************************************
## 当增加一个新的神经元类别时， 需要重新创建一个 LIB， 并将其添加至NEURONS_LIB
NEURONS_MOVEMENT_LIB = {"TYPE_ID" : 0x62, "BLOCK_NUM" : NEU_MOVEMENT_MODULE_NUM, "BLOCK_INFO" : \
                        [NEU_MOVEMENT_SINGLE_DC_MOTOR, NEU_MOVEMENT_DOUBLE_DC_MOTOR, NEU_MOVEMENT_9G_SERVO, NEU_MOVEMENT_SINGLE_CODE_MOTOR]}

NEURONS_SENSORS_LIB = {"TYPE_ID" : 0x63, "BLOCK_NUM" : NEU_SENSORS_MODULE_NUMBER, "BLOCK_INFO" : \
                       [NEU_SENSORS_TEMPERATURE, NEU_SENSORS_LIGHT_SENSOR, NEU_SENSORS_ULTRASONIC_SENSOR, NEU_SENSORS_LINEFOLLOWER_SENSOR, \
                       NEU_SENSORS_COLOR_SENSOR, NEU_SENSORS_GYRO_ACCELERATION_SENSOR, NEU_SENSORS_TEMPERATURE_HUMIDITY_SENSOR, NEU_SENSORS_SOIL_HUMIDITY_SENSOR, \
                       NEU_SENSORS_RAINDROP_SENSOR, NEU_SENSORS_WIND_SPEED_SENSOR, NEU_SENSORS_PARTICLE_CONCENTRATION_SENSOR, NEU_SENSORS_HUMEN_IR_SENSOR, \
                       NEU_SENSORS_SOUND_SENSOR]}

NEURONS_CONTROLERS_LIB = {"TYPE_ID" : 0x64, "BLOCK_NUM" : NEU_CONTROLERS_MODULE_NUMBER, "BLOCK_INFO" : \
                         [NEU_CONTROLERS_DAIL, NEU_CONTROLERS_BUTTON, NEU_CONTROLERS_ROTARY_ENCODER, NEU_CONTROLERS_MAKEY_MAKEY, \
                          NEU_CONTROLERS_TOUCH_KEY, NEU_CONTROLERS_SELF_LOCKING_BUTTON, NEU_CONTROLERS_ROCKER, NEU_CONTROLERS_LIGHT_TOUCH_KEY]}

NEURONS_DISPLAY_LIB = {"TYPE_ID" : 0x65, "BLOCK_NUM" : NEU_DISPLAY_MODULE_NUMBER, "BLOCK_INFO" : \
                      [NEU_DISPLAY_4_BIT_7SEG_TUBE, NEU_DISPLAY_COLORFUL_RGB_LED, NEU_DISPLAY_RGB_LED_STRIP, NEU_DISPLAY_RGB_LED_MATRIX_8_8, \
                       NEU_DISPLAY_OLED, NEU_DISPLAY_COLD_LIGHT_PIPE]}

NEURONS_SOUND_LIB = {"TYPE_ID" : 0x66, "BLOCK_NUM" : NEU_SOUND_MODULE_NUMBER, "BLOCK_INFO" : \
                     [NEU_SOUND_BUZZER]}


NEURONS_LIB = [NEURONS_MOVEMENT_LIB, NEURONS_SENSORS_LIB, NEURONS_CONTROLERS_LIB, NEURONS_DISPLAY_LIB, NEURONS_SOUND_LIB]

### special blocks that can not be described by common struct , include the MP3 bllock、rokey block 
NEURONS_SPECIAL_LIB = [ \
                        [{"respond_num": len(NEU_ROCKY_S["respond_info"]), "command_num": len(NEU_ROCKY_S["command_info"])}, NEU_ROCKY_S], \
                        [{"respond_num": len(NEU_SOUND_MP3_S["respond_info"]), "command_num": len(NEU_SOUND_MP3_S["command_info"])}, NEU_SOUND_MP3_S], \
                        [{"respond_num": len(NEU_IO_EXTEND_BOARD_S["respond_info"]), "command_num": len(NEU_IO_EXTEND_BOARD_S["command_info"])}, NEU_IO_EXTEND_BOARD_S] \
                      ]
## ***************************************************************************************************************************************
## 文件头信息
HEAD_LENGTH  = 160
HEAD_MAGIC_STR_LENGTH = 16
HEAD_VERSION_STR_LENGTH = 16

## 版本说明
# V1.0alpha1： 
# 1.首版 

# V1.0alpha2
# 1.修改rocky颜色传感器的两个库文件， 之前命令填写错误， 造成读取rgb颜色以及灰度出错

# V1.0alpha3
# 1.增加读取rocky颜色光强的命令；
# 说明：由于命令的不连续性， rocky的respond命令从0x0b到0x13是无效的， 0x15预留给HSV使用
#       rocky的command命令的0x15预留给HSV使用

# V1.0alpha4_test
# 1.增加读取rocky读取电机电流命令；
#说明：该版本只作为测试使用， 产品并无此需求

# V1.0alpha5
# 1.由于rocky的颜色传感器方案改变， 因此读值命令发生重大变化， 故该版本刷新整个rocky的命令
#   包括id和命令种类

# T01
# 1. 将版本号V1.0alpha5修改为001, 后续修改将以数字 xxx的形式更新
# 说明： 001 和 V1.0alpha5 版本内容没有任何改变， 仅编号发生改变

# 001
# 1. 将版本号T01修改为001, 后续修改将以数字 xxx的形式更新
# 说明： 001 和T01 版本内容没有任何改变， 仅编号发生改, 修改原因是因为该版本将作为众筹发步版本

# 002 
# 1. 增加IO扩展模块
# 2. 对特殊模块个数以及模块类型自动统计， 不需要手动计算后更新； 但是单个类别的模块添加新的模块时人需要手动增加

# 作为神经元引擎库的校验字符串， 只有当 文件头的信息和HEAD_MAGIC_RECORD[-1]一致时，才可能是神经元引擎库文件
HEAD_MAGIC_RECORD = ["codey_neu"] # we can only append this list, not cover
HEAD_VERSION_RECORD = ["V1.0alpha1", "V1.0alpha2", "V1.0alpha3", "V1.0alpha4", "V1.0alpha5", "T01", "002"] # we can only append this list, not cover 
HEAD_SINGLE_LIB_BYTES_NUM = 62
HEAD_TYPE_NUM_RECORD = [6, len(NEURONS_LIB)]                  # we can only append this list, not cover
HEAD_SPECIAL_BLOCK_NUM_RECORD = [2, len(NEURONS_SPECIAL_LIB)] # we can only append this list, not cover
HEAD_TYPE_INFO_PER_LENGTH = 4

## 单个模块信息
BLOCK_LIB_BYTES_NUM = 62   #ser_id(1) + sub_id(1) + 5 * respond_item(6) + 5 * command_item(6)
BLOCK_LIB_RESPOND_ITEM_MAX = 5
BLOCK_LIB_COMMAND_ITEM_MAX = 5
BLOCK_LIB_RESPOND_ITEM_BYTES_NUM = 6 # respond_id(1) + datas_type(5)
BLOCK_LIB_COMMAND_ITEM_BYTES_NUM = 6 # command_id(1) + datas_type(5)

BLOCK_LIB_RES_CMD_ITEM_OVER_FLAG_VALUE = 0x00
BLOCK_LIB_RES_CMD_OVER_FLAG_VALUE = 0xff
SPECIAL_BLOCKS_NUM_MAX = 8

###########################################
# update the this lib you are exepectd to do:
# 1. add the new version to  HEAD_VERSION_RECORD
# 2. modify or add new lib for the block
# 3. modify the information of every group blocks, like NEURONS_MOVEMENT_LIB...
# 4. modify this field
#  HEAD_SINGLE_LIB_BYTES_NUM = 62
#  HEAD_TYPE_NUM_RECORD = [6]          # we can only append this list, not cover
#  HEAD_SPECIAL_BLOCK_NUM_RECORD = [2] # we can only append this list, not cover
#  HEAD_TYPE_INFO_PER_LENGTH = 4