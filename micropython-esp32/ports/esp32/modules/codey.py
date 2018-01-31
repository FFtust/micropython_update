from codey_ledmatrix_board import codey_ledmatrix
from codey_rgbled_board import codey_rgbled
from codey_wlan_board import codey_wlan
from codey_event_board import *
from codey_global_board import *

from makeblock import super_var
from makeblock import timer_t
from makeblock import reset_timer_t
import math
import time as stime
import _thread
from umqtt.simple import MQTTClient

# a temporary solution to turn off the rgb led on rocky
from rocky import stop as r_stop
from rocky import color as r_color
r_stop()
r_color("#000000")

# for rgb led
m_rgbled = codey_rgbled()
def color(col, t = None):             # col is string or int index should be discussed
    if t == None:
        m_rgbled.color(col)           # now int index
    elif type_check(t, int, float):
        if t <= 0:
            return
        m_rgbled.color(col)           # now int index
        stime.sleep(t)
        m_rgbled.color("#000000")

def rgb(r, g, b):
    if (not type_check(r, int, float)) or (not type_check(g, int, float)) or (not type_check(b, int, float)):
        return
    r = num_range_check(r, 0, 255)
    g = num_range_check(g, 0, 255)
    b = num_range_check(b, 0, 255)
    m_rgbled.set_rgb(int(r), int(g), int(b))

def red(val):
    if (not type_check(val, int, float)):
        return    
    val =int(num_range_check(val, 0, 255))
    m_rgbled.set_r_g_b(val, RGB_RED_INDEX)

def green(val):
    if (not type_check(val, int, float)):
        return    
    val =int(num_range_check(val, 0, 255))
    m_rgbled.set_r_g_b(val, RGB_GREEN_INDEX)

def blue(val):
    if (not type_check(val, int, float)):
        return    
    val =int(num_range_check(val, 0, 255))
    m_rgbled.set_r_g_b(val, RGB_BLUE_INDEX)

def color_off():
    m_rgbled.set_rgb(0, 0, 0)

#for ledmatrix
m_ledmatrix = codey_ledmatrix()
def animation(animation_name, to_stop = False):
    face_dict = {"cat": 0, "dog": 0, "test": 0, "happy": 1, "cry": 2, "dispirited": 3, "angry": 4, "fear": 5} 
    if type_check(animation_name, int):
        animation_name = num_range_check(animation_name, 0, 5)
        m_ledmatrix.animation_show(animation_name, int(to_stop))
    elif type_check(animation_name, str):
        try:
            m_ledmatrix.animation_show(face_dict[animation_name], int(to_stop))
        except KeyError:
            print("not found this animation")

def face_at(fac, p_x, p_y, with_time = None):          # face_code should be a list with 16 elements
    if (not type_check(p_x, int, float)) or (not type_check(p_y, int, float)):
        return    
    p_x = int(p_x)
    p_y = int(p_y)
    if p_x > 15 or p_x < -15 or p_y > 7 or  p_y < -7:
        clear()
        
    temp_list = [0] * 16
    if type_check(fac, list):
        for i in range(len(fac)):
            temp_list[i] = fac[i]
        m_ledmatrix.faceplate_show(p_x, p_y, *fac)

    elif type_check(fac, str):
        if len(fac) != 32:  # 16 *2
            fac = fac + '0' * (32- len(fac))

        for i in range(16):
            temp_list[i] = int(fac[i * 2: i * 2 + 2], 16)  

    if with_time == None:
        m_ledmatrix.faceplate_show((int)(p_x), (int)(p_y), *temp_list)
    elif type_check(with_time, int, float):
        m_ledmatrix.faceplate_show_with_time((int)(p_x), (int)(p_y), int(with_time * 1000), *temp_list)


def face(fac, wait = None):          # face_code should be a list with 16 elements
    if wait == None:
        face_at(fac, 0, 0)
        return 
    elif type_check(wait, int, float):
        if wait <= 0:
            return
        face_at(fac, 0, 0)
        stime.sleep(wait)
        clear()
        
def show(var):
    if type_check(var, bool):
        if var == True:
            m_ledmatrix.string_show('true', 1)
        elif var == False:
            m_ledmatrix.string_show('false', 1)

    elif type_check(var, str):
        m_ledmatrix.string_show(var, 1) # 1: to_stop, 0: not to_stop
    
    elif type_check(var, int):
        m_ledmatrix.number_show(str(var))

    elif type_check(var, float): # python str(0.000009) equal 9.00000e-06
        if var < 0.001 and var > 0: 
            m_ledmatrix.number_show("0.0")
        elif var < 0 and var > -0.01:
            m_ledmatrix.number_show("-0.0")
        else:
            var = str(var)
            if len(var) > 5:
                var = var[0 : 5]
            z_num = 0
            if '.' in var:
                while True:
                    if var[-1 - z_num] == '0':
                        z_num += 1
                    else:
                        break
            if z_num == 0:
                m_ledmatrix.number_show(var)
            elif var[-1 - z_num] == '.':
                m_ledmatrix.number_show(var[0 : len(var) - z_num + 1])
            else:
                m_ledmatrix.number_show(var[0 : len(var) - z_num])

def pixel(pos_x, pos_y):
    if (not type_check(pos_x, int, float)) or (not type_check(pos_y, int, float)):
        return
    pos_x = int(pos_x)
    pos_y = int(pos_y)
    if pos_x < 0 or pos_x > 15 or pos_y < 0 or pos_y > 7:
        return  
    m_ledmatrix.pixel_control((int)(pos_x), (int)(pos_y), 1)

def pixel_off(pos_x, pos_y):
    pos_x = int(pos_x)
    pos_y = int(pos_y)
    if (not type_check(pos_x, int, float)) or (not type_check(pos_y, int, float)):
        return
    if pos_x < 0 or pos_x > 15 or pos_y < 0 or pos_y > 7:
        return    
    m_ledmatrix.pixel_control((int)(pos_x), (int)(pos_y), 0)

def clear():
    m_ledmatrix.clean()

# not a product demand
def screen_overturn():
    m_ledmatrix.screen_invert()

def pixel_toggle(pos_x, pos_y):
    if (not type_check(pos_x, int, float)) or (not type_check(pos_y, int, float)):
        return
    pos_x = int(pos_x)
    pos_y = int(pos_y)
    if pos_x < 0 or pos_x > 15 or pos_y < 0 or pos_y > 7:
        return      
    m_ledmatrix.pixel_invert(int(pos_x), int(pos_y))

# not a product demand
def is_shown(pos_x, pos_y):
    if (not type_check(pos_x, int, float)) or (not type_check(pos_y, int, float)):
        return
    pos_x = int(pos_x)
    pos_y = int(pos_y)
    if pos_x < 0 or pos_x > 15 or pos_y < 0 or pos_y > 7:
        return    
    return m_ledmatrix.get_pixel(int(pos_x), int(pos_y))

# not a product demand
def percent(val):
    if not type_check(val, int, float):
        return
    val = num_range_check(val, 0, 100)
    m_ledmatrix.show_persent(val)

# for speaker
m_mus = music()
def mute():                 
    m_mus.stop_all()  

def set_volume(vol): 
    if not type_check(vol, int, float):
        return
    vol = num_range_check(vol, 0, 100)
    m_mus.set_volume(vol)

def get_volume():   
    return round(m_mus.get_volume(), 1)

def change_volume(val):
    if not type_check(val, int, float):
        return
    val = num_range_check(val, -100, 100)
    m_mus.change_volume(val)

def say(sound_name, wait = False, off_t = 0.05): 
    if sound_name[-4 : ] != ".wav":
        sound_name_t = sound_name + ".wav"
    else:
        sound_name_t = sound_name
    if not wait:
        m_mus.play(sound_name_t)
        if off_t != None:
            stime.sleep(off_t) 
    else:
        m_mus.play_to_stop(sound_name_t)

# the parameter of this function should be with music node
def play(note_type, beat, wait = None):    
    if type_check(note_type, str) and type_check(beat, int, float):
        try:
            freq = node_table[note_type]
        except KeyError:
            print("not found the node", note_type)
            return
        m_mus.play_note_to_stop(int(freq), int(beat * BEAT_TO_SECOND * 1000)) # to ms

def play_note(note_num, beat):
    if type_check(note_num, int, float) and type_check(beat, int, float):
        if note_num <= 0 or beat <= 0:
            return
        note_num = num_range_check(note_num, 0, 127) 
        freq = MIDI_NOTE_NUM0 * pow(NOTE_FREQUENCE_RATIO, note_num)
        m_mus.play_note_to_stop(int(freq), int(beat * BEAT_TO_SECOND * 1000)) # to ms

# the unit of t: second
def play_freq(freq, t, wait = None):    
    if type_check(freq, int, float) and type_check(t, int, float):
        if freq <= 0 or t <= 0:
            return
        m_mus.play_note_to_stop(int(freq), int(t * 1000)) 

def pause(num):
    if not type_check(num, int, float):
        return
    else:
        num = num_range_check(num, 0)
        stime.sleep(num * BEAT_TO_SECOND)

# for sound sensor
m_soundsensor = sound_sensor_board()
def sound_strength():
    tem = m_soundsensor.value() * VAL_TO_SENSOR
    return round(sound_value_range(tem), 1)

# for light sensor
m_lightsensor = light_sensor_board()
def light_strength():
    return round(m_lightsensor.value() * VAL_TO_SENSOR, 1)


# for potentionmeter
m_potentionmeter = potentionmeter_board()
def dail():
    return round(m_potentionmeter.value() * VAL_TO_SENSOR, 1)

# for buttons  & button_id should be 1-3
m_button = button_board()
def is_button(button_id):
    if button_id != "A" and button_id != "B" and button_id != "C":
        return None 
    dic = {'A': 1, 'B': 2, 'C': 3}
    if m_button.value(dic[button_id]) == 1: # if m_button.get_value(dic[button_id]) == 1:
        return True
    else:
        return False

# for gyro 
m_gyro = gyro_board()
def gyro(axis):
    axis_index = 0
    if type_check(axis, str):
        if axis == 'pitch':
            axis_index = 1
        elif axis == 'yaw':
            axis_index = 2
        elif axis == 'roll':
            axis_index = 3
    elif type_check(axis, int):
        axis_index = axis

    # to 0 - 360
    ret = m_gyro.value(axis_index)
    if axis_index == 2:
        while ret < 0:
            ret = ret + 360
        while ret > 360:
            ret = ret % 360 
    return round(int(ret))

def is_shaked():
    if m_gyro.is_shaked():
        return True
    else:
        return False

def is_tilt(dir):
    dic = {"forward" : 3, "backward" : 4, "left" : 1, "right" : 2}
    if not type_check(dir, str):
        return False
    ret = m_gyro.tilt()
    try:
        if ret & (1 << dic[dir]):
            return True
        else:
            return False
    except KeyError:
        return False

def gyro_a(axis = 0):
    axis_index = 0
    if type_check(axis, str):
        if axis == 'x':
            axis_index = 1
        elif axis == 'y':
            axis_index = 2
        elif axis == 'z':
            axis_index = 3
    elif type_check(axis, int):
        axis_index = num_range_check(axis, 1, 3)
    temp_value = m_gyro.raw_data(1)

    return int(temp_value[axis_index - 1] * 0.98) / 10  # make the value only one number after point

# for infrared sensor
m_ir = rmt_board()
def ir_receive():
    ir_list_t = m_ir.value()
    if ir_list_t == None:
        return None

    if type_check(ir_list_t, list):
        ir_len_t = len(ir_list_t)
        ir_str_t = ''
        if ir_len_t == 0:
            return None
        elif ir_list_t[0] != ord('\n'):
            return None
        elif ir_len_t > 1:
            for i in range(1, ir_len_t):
                if ir_list_t[i] == ord('\n'):
                    break    
                else:
                    ir_str_t = chr(ir_list_t[i]) + ir_str_t

            return ir_str_t 
        else:
            return None
    elif type_check(ir_list_t, int):
        return ir_list_t

def ir_get_char():
    ir_char = m_ir.get_char()

    return ir_char

def ir_send(sstr):
    if type(sstr) != str: 
        sstr = str(sstr) + '\n'
    else:
        if(sstr[-1] != '\n'):
            sstr += '\n'
    m_ir.send(0, sstr)

# for time counter
def time():
    return int(timer_t() * 10) / 10

def reset_time():
    reset_timer_t()

# for message advance
m_mess = message_board()
def message(sstr, wait = False):    
    stime.sleep(0.02)  
    m_mess.message_advance(sstr)

# for event mechanism    
def on_start(callback):
    event_register_add(CODEY_LAUNCH, callback)

def on_shake(callback):              
    event_register_add(BOARD_SHAKE, callback)

def on_sound_over(threshold, callback):
    if type(threshold) == str:
        return
    threshold = int(threshold)
    if threshold < 0:
        threshold = 0
    event_register_add(SOUND_OVER, callback, threshold)

def on_light_under(threshold, callback):
    if type(threshold) == str:
        return
    threshold = int(threshold)
    if threshold < 0:
        threshold = 0
    event_register_add(LIGHT_BELOW, callback, threshold)

def on_message(msgstr, callback):
    if type(msgstr) != str:
        msgstr = str(msgstr)
    event_register_add(MESSAGE, callback, msgstr)

def on_button(button_id, callback):
    if button_id == 'A':
        event_register_add(BUTTON1_PRESS, callback)
    if button_id == 'B':
        event_register_add(BUTTON2_PRESS, callback)
    if button_id == 'C':
        event_register_add(BUTTON3_PRESS, callback)

def on_tilt(dir_str, callback):
    if dir_str == "forward":
        event_register_add(BOARD_TILT_FORWARD, callback)
    elif dir_str == "backward":
        event_register_add(BOARD_TILT_BACK, callback)
    elif dir_str == "left":
        event_register_add(BOARD_TILT_LEFT, callback)
    elif dir_str == "right":
        event_register_add(BOARD_TILT_RIGHT, callback)
        
# super_var
def set_variable(name, value):
    s_var = super_var(name) 
    if (s_var):
        return s_var.set_value(value) 
    else:
        return False

def get_variable(name):
    g_var = super_var(name)
    if (g_var):
        return g_var.get_value()
    else:
        return None
#--------------------- IOT wifi interface ----------------
m_wifi = codey_wlan()
wifi_connect_time = 3 # s
connecting_ssid = None
connecting_pass = None
m_wifi.wifi_enable()
wifi_sema = _thread.allocate_lock()
def wifi(ssid, password):
    global connecting_ssid
    global connecting_pass
    wifi_sema.acquire(1)
    if (ssid != connecting_ssid) or (password != connecting_pass):
        connecting_ssid = ssid
        connecting_pass = password
        m_wifi.wifi_sta_config(ssid, password)
        m_wifi.wifi_mode_config(m_wifi.STA)
        m_wifi.wifi_start()
        wait_count = 0
        while not wifi_is_connected():
            stime.sleep(0.1)
            wait_count += 1
            if(wait_count > wifi_connect_time * 10):
                break
    else:
        if not wifi_is_connected():
            m_wifi.wifi_connect()
            wait_count = 0
            while not wifi_is_connected():
                stime.sleep(0.1)
                wait_count += 1
                if(wait_count > wifi_connect_time * 10):
                    break
    wifi_sema.release()

def wifi_close():
    global connecting_ssid
    global connecting_pass
    if wifi_is_connected():
        wifi_sema.acquire(1)
        m_wifi.wifi_disconnect()
        wifi_sema.release()

def wifi_is_connected():
    # sleep below is necessary now
    stime.sleep(0.05)
    return m_wifi.wifi_sta_is_conn()

## battery capacity check 
from makeblock import battery_check
m_bat = battery_check()
def get_battery_voltage():
    return m_bat.battery_vol()

def charge():
    return m_bat.battery_cap()

#------------------ IOT mqtt interface --------------------
subscribe_thread = False
subscribe_client = None
def mqtt_pub(server, topic, msg):
    c = MQTTClient("cokey_mqtt_client", server)
    if (True != c.connect()):
        return False
    if (True != c.publish(topic, msg)):
        return False
    c.disconnect()
    return True

def mqtt_sub_loop():
    global subscribe_client
    try:
        while True:
            subscribe_client.check_msg()
            stime.sleep(1)
    finally:
        subscribe_client.disconnect()
    subscribe_thread = False

def mqtt_sub(server, topic, callback):
    global subscribe_thread
    global subscribe_client
    c = MQTTClient("cokey_mqtt_client", server)
    c.set_callback(callback)
    if (True != c.connect()):
        return False
    if (True != c.subscribe(topic)):
        return False
    subscribe_client = c
    if (not subscribe_thread):
        _thread.start_new_thread(mqtt_sub_loop, ())
        subscribe_thread = True
    return True

# init the random module
import random
random.seed(int(m_potentionmeter.value() + m_soundsensor.value() + m_lightsensor.value()))

import gc
gc.collect()