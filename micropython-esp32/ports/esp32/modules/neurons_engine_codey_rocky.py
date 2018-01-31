neu_block_string_dict = \
{
    "MOTOR": [(0x62, 0x01), {"power": 0x01}],
    "DMOTOR": [(0x62, 0x02), {"power": 0x01}],
    "SERVO": [(0x62, 0x03), {"angle": 0x01}],
    "CMOTOR": [(0x62, 0x04), {"abspos": 0x01, "relpos": 0x02, \
                             "speed": 0x03, "pwm": 0x04, \
                             "cur_pos": 0x05}],
###################################################################                             
    "TEMP": [(0x63, 0x01), {"val": 0x01}],
    "LIGHT": [(0x63, 0x02), {"val": 0x01}],                           
    "ULTRAS": [(0x63, 0x03), {"val": 0x01}],    
    "LINEFOLLOWER": [(0x63, 0x04), {"val": 0x01}],   
    "COLOR": [(0x63, 0x05), {"val": 0x01}],  
    "GYRO": [(0x63, 0x06), {"subscr": 0x01}],   ## special  format
    "HUMI": [(0x63, 0x07), {"val": 0x01}], 
    "SOIL": [(0x63, 0x08), {"val": 0x01}], 
    "RAIN": [(0x63, 0x09), {"val": 0x01}], 
    "WIND": [(0x63, 0x0a), {"val": 0x01}],
    "PM": [(0x63, 0x0b), {"pm2.5": 0x01, "pm1.0": 0x02, "pm10": 0x03}],
    "PIR": [(0x63, 0x0c), {"val": 0x01}],
    "SOUND": [(0x63, 0x0d), {"val": 0x01}],
################################################################### 
    "DAIL": [(0x64, 0x01), {"val": 0x01}],
    "BUTTON": [(0x64, 0x02), {"val": 0x01}],
    "MAKEY": [(0x64, 0x04), {"val": 0x01}],   ## special  
    "TKEY": [(0x64, 0x05), {"val": 0x01}],
    "LOCK": [(0x64, 0x06), {"val": 0x01}],
    "JOY": [(0x64, 0x7), {"val": 0x01}],
####################################################################
    "TUBE": [(0x65, 0x01), {"show": 0x01}],
    "RGBLED": [(0x65, 0x02), {"set_color": 0x01}],
    "STRIP": [(0x65, 0x03), {"set_single":0x01, "set_block":0x02}], ## special
    "FACE": [(0x65, 0x04), {"set_face": 0x01, "set_signel": 0x02, "set_colorful_face": 0x03}], 
    "OLED": [(0x65, 0x05), {"send_str": 0x01, "send_face": 0x02}],
    "ELWIRE":[(0x65, 0x06), {"set": 0x01}],
#####################################################################   
    "MP3": [(0x66, 0x01), {"play": 0x01, "delete": 0x02, \
                           "play_last": 0x03, "play_next": 0x04, \
                           "pause": 0x05, "stop": 0x06, \
                           "delete_all": 0x07, "set_mode": 0x08, \
                           "change_volume": 0x09 }],
    "BUZZER": [(0x66, 0x02), {"play": 0x01}],

######################################################################
    "ROCKY": [(0x63, 0x10), {"val_rgb": 0x01, \
                             "color": 0x02, \
                             "val_lightness": 0x03, \
                             "val_grey": 0x04, \
                             "val_light_reflect": 0x05, \
                             "val_ir_reflect": 0x06, \
                             "is_in_grey": 0x07, \
                             "is_barrier": 0x08, \
                             "set_rgb": 0x09, \
                             "val_motor_current": 0x0a, \

                             "stop": 0x0c, \
                             "forward": 0x0d, \
                             "back": 0x0e, \
                             "right": 0x0f, \
                             "left": 0x10, \
                             "drive": 0x11, \
                             "left_dif": 0x12, \
                             "right_dif": 0x13, \
                             "single_drive": 0x14}],

    "IO_EXT": [(0x67, 0x02), {"set_digital_sta": 0x01, \
                              "get_digital_sta": 0x02, \
                              "set_servo_angle": 0x03, \
                              "get_analog_val" : 0x04, \
                              "write_i2c"      : 0x06, \
                              "read_i2c"       : 0x07, \
                              "set_relay_sta":   0x08
                             }

              ] 
}

#############################################
from makeblock import neurons_engine
neu = neurons_engine()
def find_block_by_name(name):
    try:
        ret_block = neu_block_string_dict[name]
    except KeyError:
        print("not found the block", name)
        ret_block =  None
    finally:
        return ret_block

def find_func_by_name(block_info, func_name):    
    try:
        ret_func = block_info[1][func_name]
    except KeyError:
        print("not found the func", func_name)
        ret_func =  None
    finally:
        return ret_func
        
def neu_send(block_str, func_str = None, index = 1, para = []):
    block_info = find_block_by_name(block_str)
    if block_info != None:
        func_id = find_func_by_name(block_info, func_str)
        if func_id != None:
            para.insert(0, func_id)
            neu.send(block_info[0][0], block_info[0][1], index, para)

def neu_read(block_str, func_str = None, index = 1, para = []):
    block_info = find_block_by_name(block_str)
    if block_info != None:
        func_id = find_func_by_name(block_info, func_str)
        if func_id != None:
            para.insert(0, func_id)
            #neu.send(block_info[0][0], block_info[0][1], index, para)
            value = neu.read(block_info[0][0], block_info[0][1], index, para)
            if(value == None):
                return None
            if(len(value) == 1):
                return value[0]
            else:
                return value
