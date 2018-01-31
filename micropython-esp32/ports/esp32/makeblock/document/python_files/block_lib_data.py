#NEU_RGB = ({(0x65, 0x02): "NEU_RGB"}, [], {"set_color": (0x01, NEU_SHORT, NEU_SHORT, NEU_SHORT)})

#NEU_BUTTON = ({(0x64, 0x02): "NEU_BUTTON"}, [0x01, NEU_BYTE], {'is_pressed':(0x01,)})

#NEU_SENSORS_LIB = {"NEU_RGB" : NEU_RGB, "NEU_BUTTON" : NEU_BUTTON} 
################################
TYPE_BYTE_8 = 0x01
TYPE_byte_16 = 0x02
TYPE_SHORT_16 = 0x03
TYPE_short_24 = 0x04
TYPE_long_40 = 0x05
TYPE_float_40 = 0x06

# every lib has 4 items; type -- sub_type -- respond -- command --
NEU_SENSORS_TEM_LIB = [0x63, 0x01, [(0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00)], [(0x01, 0x00, 0x00, 0x00, 0x00, 0x00)]]
NEU_SENSORS_LIGHT_LIB = [0x63, 0x02, [(0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00)], [(0x01, 0x00, 0x00, 0x00, 0x00, 0x00)]]
NEU_SENSOR_ULTAR_LIB = [0x63, 0x03, [(0x01, TYPE_float_40, 0x00, 0x00, 0x00, 0x00)], [(0x01, 0x00, 0x00, 0x00, 0x00, 0x00)]]
NEU_SENSOR_LINEFOLLOW_LIB = [0x63, 0x04, [(0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00)], [(0x01, 0x00, 0x00, 0x00, 0x00, 0x00)]]
NEU_SENSOR_COLOR_LIB = [0x63, 0x05, [(0x01, TYPE_SHORT_16, TYPE_SHORT_16, TYPE_SHORT_16, 0x00, 0x00)], [(0x01, 0x00, 0x00, 0x00, 0x00, 0x00)]]

# controlers test
NEU_DISPLAY_NUMERIC_LIB = [0x65, 0x01, [], [(0x01, TYPE_float_40, 0x00, 0x00, 0x00, 0x00)]]
NEU_DISPLAY_S_RGB_LED_LIB = [0x65, 0x02, [], [(0x01, TYPE_SHORT_16, TYPE_SHORT_16, 0x00, 0x00, 0x00)]]
## the second command not support now 
NEU_DISPLAY_M_RGB_LED_LIB = [0x65, 0x03, [], [(0x01, TYPE_BYTE_8, TYPE_SHORT_16, TYPE_SHORT_16, 0x00, 0x00)]]
NEU_DISPLAY_RGB_MATRIX_LIB = [0x65, 0x04, [], [(0x01, TYPE_long_40, TYPE_long_40, TYPE_SHORT_16, TYPE_SHORT_16, TYPE_SHORT_16), \
                                               (0x02, TYPE_BYTE_8, TYPE_SHORT_16, TYPE_SHORT_16, TYPE_SHORT_16, 0x00)]]

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
                                        "command_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00),) \
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
                             "command_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00),\
                                               (0x02, 0x30 | TYPE_BYTE_8, 0xf0 | TYPE_BYTE_8, 0x00, 0x00, 0x00),\
                                              ) \
                              }
### 彩色8*8led面板 0x04
NEU_DISPLAY_RGB_LED_MATRIX_8_8 = {"name": "rgb_led_matrix", "type" : 0x65, "sub_type": 0x04, \
                                  "respond_info" : (), \
                                  "command_info" : ((0x01, 0x20 | TYPE_long_40, 0x30 | TYPE_SHORT_16, 0x00, 0x00, 0x00), \
                                                    (0x02, TYPE_BYTE_8, 0x30 | TYPE_SHORT_16, 0x00, 0x00, 0x00), \
                                                    (0x03, 0x20 | TYPE_BYTE_8, 0xf0 | TYPE_BYTE_8, 0x00, 0x00, 0x00), \
                                                    (0x04, 0x20 | TYPE_BYTE_8, 0xf0 | TYPE_BYTE_8, 0x00, 0x00, 0x00), \
                                                    (0x05, 0x20 | TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00)) \
                                 }
### OLED显示屏 0x05
NEU_DISPLAY_OLED = {"name": "oled", "type" : 0x65, "sub_type": 0x05, \
                    "respond_info" : (), \
                    "command_info" : ((0x01, 0x20 | TYPE_BYTE_8, TYPE_SHORT_16, 0xf0 | TYPE_BYTE_8, 0x00, 0x00), \
                                      (0x02, 0x20 | TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                      (0x03, 0x20 | TYPE_BYTE_8, TYPE_SHORT_16, 0xf0 | TYPE_BYTE_8, 0x00, 0x00), \
                                     )\
                   }
### 冷光管 0x06
NEU_DISPLAY_COLD_LIGHT_PIPE = {"name": "cold_light_pipe", "type" : 0x65, "sub_type": 0x06, \
                               "respond_info" : (), \
                               "command_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00),) \
                              }
## **************************************************************************************************************************************
## 声音类 0x66, (0x01, 0x02)

### MP3模块
NEU_SOUND_MP3 = {"name": "mp3", "type" : 0x66, "sub_type": 0x01, \
                 "respond_info" : (), \
                 "command_info" : ((0x01, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                   (0x02, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                   (0x03, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                   (0x04, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                   (0x05, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                   #(0x06, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                   #(0x07, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                   #(0x08, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                   #(0x09, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                               ) \
                }
### 蜂鸣器模块
NEU_SOUND_BUZZER = {"name": "buzzer", "type" : 0x66, "sub_type": 0x02, \
                    "respond_info" : (), \
                    "command_info" : ((0x01, TYPE_short_24, TYPE_BYTE_8, 0x00, 0x00, 0x00),) \
                   }
## ***************************************************************************************************************************************
# 其他类0x67

####说明
# 特殊传感器有 陀螺仪、 颗粒浓度检测计、 彩色灯带、led面板、OLED显示屏、rokey
### rokey
NEU_ROKEY_S = {"name": "buzzer", "type" : 0x63, "sub_type": 0x10, \
               "respond_info" : ((0x01, 0x30 | TYPE_short_24, 0x00, 0x00, 0x00, 0x00), \
                                 (0x02, TYPE_BYTE_8, TYPE_BYTE_8, 0x00, 0x00, 0x00), \
                                 (0x03, TYPE_SHORT_16, 0x00, 0x00, 0x00, 0x00), \
                                 (0x04, TYPE_SHORT_16, 0x00, 0x00, 0x00, 0x00), \
                                 (0x05, TYPE_SHORT_16, 0x00, 0x00, 0x00, 0x00), \
                                 (0x07, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                 (0x08, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                 (0x09, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                 (0x0a, TYPE_SHORT_16, TYPE_SHORT_16, 0x00, 0x00, 0x00) \
                               ), \
              "command_info" : ((0x01, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x02, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x03, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x04, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x05, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x06, TYPE_BYTE_8, 0x20 | TYPE_SHORT_16, 0x00, 0x00, 0x00), \
                                (0x07, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x09, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x0a, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x0b, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x11, 0x00, 0x00, 0x00, 0x00, 0x00), \
                                (0x12, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x13, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x14, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x15, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x16, 0x40 | TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x17, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00), \
                                (0x18, TYPE_BYTE_8, 0x00, 0x00, 0x00, 0x00) \
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

## ***************************************************************************************************************************************
NEURONS_MOVEMENT_LIB = {"TYPE_ID" : 0x62, "BLOCK_NUM" : 4, "BLOCK_INFO" : \
                        [NEU_MOVEMENT_SINGLE_DC_MOTOR, NEU_MOVEMENT_DOUBLE_DC_MOTOR, NEU_MOVEMENT_9G_SERVO, NEU_MOVEMENT_SINGLE_CODE_MOTOR]}

NEURONS_SENSORS_LIB = {"TYPE_ID" : 0x63, "BLOCK_NUM" : 13, "BLOCK_INFO" : \
                       [NEU_SENSORS_TEMPERATURE, NEU_SENSORS_LIGHT_SENSOR, NEU_SENSORS_ULTRASONIC_SENSOR, NEU_SENSORS_LINEFOLLOWER_SENSOR, \
                       NEU_SENSORS_COLOR_SENSOR, NEU_SENSORS_GYRO_ACCELERATION_SENSOR, NEU_SENSORS_TEMPERATURE_HUMIDITY_SENSOR, NEU_SENSORS_SOIL_HUMIDITY_SENSOR, \
                       NEU_SENSORS_RAINDROP_SENSOR, NEU_SENSORS_WIND_SPEED_SENSOR, NEU_SENSORS_PARTICLE_CONCENTRATION_SENSOR, NEU_SENSORS_HUMEN_IR_SENSOR, \
                       NEU_SENSORS_SOUND_SENSOR]}

NEURONS_CONTROLERS_LIB = {"TYPE_ID" : 0x64, "BLOCK_NUM" : 8, "BLOCK_INFO" : \
                         [NEU_CONTROLERS_DAIL, NEU_CONTROLERS_BUTTON, NEU_CONTROLERS_ROTARY_ENCODER, NEU_CONTROLERS_MAKEY_MAKEY, \
                          NEU_CONTROLERS_TOUCH_KEY, NEU_CONTROLERS_SELF_LOCKING_BUTTON, NEU_CONTROLERS_ROCKER, NEU_CONTROLERS_LIGHT_TOUCH_KEY]}

NEURONS_DISPLAY_LIB = {"TYPE_ID" : 0x65, "BLOCK_NUM" : 6, "BLOCK_INFO" : \
                      [NEU_DISPLAY_4_BIT_7SEG_TUBE, NEU_DISPLAY_COLORFUL_RGB_LED, NEU_DISPLAY_RGB_LED_STRIP, NEU_DISPLAY_RGB_LED_MATRIX_8_8, \
                       NEU_DISPLAY_OLED, NEU_DISPLAY_COLD_LIGHT_PIPE]}

NEURONS_SOUND_LIB = {"TYPE_ID" : 0x66, "BLOCK_NUM" : 2, "BLOCK_INFO" : \
                     [NEU_SOUND_MP3, NEU_SOUND_BUZZER]}


NEURONS_LIB = [NEURONS_MOVEMENT_LIB, NEURONS_SENSORS_LIB, NEURONS_CONTROLERS_LIB, NEURONS_DISPLAY_LIB, NEURONS_SOUND_LIB]

### special blocks that can not be described by common struct , include the MP3 bllock、rokey block 
NEURONS_SPECIAL_LIB = [[{"respond_num": 10, "command_num": 18}, NEU_ROKEY_S], [{"respond_num": 0, "command_num": 9} ,NEU_SOUND_MP3_S]]
## ***************************************************************************************************************************************
## 文件头信息
HEAD_LENGTH  = 160
HEAD_VERSION_STR_LENGTH = 32

HEAD_VERSION_RECORD = ["V1.0alpha1"] # we can only append this list, not cover
HEAD_SINGLE_LIB_BYTES_NUM = 62
HEAD_TYPE_NUM_RECORD = [6]          # we can only append this list, not cover
HEAD_SPECIAL_BLOCK_NUM_RECORD = [2] # we can only append this list, not cover
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

