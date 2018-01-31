import neurons_engine_codey_rocky
from codey_global_board import *
from makeblock import gyro_board
import time
import math

color_dict = {"white": 0, "pink": 1, "red": 2, \
              "orange": 3, "yellow": 4, "green": 5, \
              "cyan":6 , "blue": 7, "purple": 8, \
              "black": 9, "gold": 10 \
                }
neu = neurons_engine_codey_rocky

# now this varibale just for rotate function
rocky_stop_flag = False
def stop(index = 1):
    global rocky_stop_flag
    rocky_stop_flag = True
    neu.neu_send("ROCKY","forward", index, [0]) # the command "stop" don't work now,
                                                # so make "forward(0) instead" 
def forward(speed, t = None, index = 1):
    if type_check(speed, int, float) and type_check(index, int, float) and (type_check(t, int, float) or t == None):
        if type_check(t, int, float):
           if t <= 0:
               return
        speed = num_range_check(speed, -100, 100)
        if speed < 0:
            neu.neu_send("ROCKY","back", index, [-speed])
        else:
            neu.neu_send("ROCKY","forward", index, [speed])
        if t != None:
            time.sleep(t)
            stop()

def backward(speed, t = None, index = 1):
    if type_check(speed, int, float) and type_check(index, int, float) and (type_check(t, int, float) or t == None):
        if type_check(t, int, float):
           if t <= 0:
               return
        speed = num_range_check(speed, -100, 100)
        if speed < 0:
            neu.neu_send("ROCKY","forward", index, [-speed])
        else:
            neu.neu_send("ROCKY","back", index, [speed])
        if t != None:
            time.sleep(t)
            stop()

def turn_left(speed, t = None, index = 1):
    if type_check(speed, int, float) and type_check(index, int, float) and (type_check(t, int, float) or t == None):
        if type_check(t, int, float):
           if t <= 0:
               return
        speed = num_range_check(speed, -100, 100)
        if speed < 0:
            neu.neu_send("ROCKY", "right", index, [-speed])
        else:
            neu.neu_send("ROCKY","left", index, [speed])
        if t != None:
            time.sleep(t)
            stop()

def turn_right(speed, t = None, index = 1):
    if type_check(speed, int, float) and type_check(index, int, float) and (type_check(t, int, float) or t == None):
        if type_check(t, int, float):
           if t <= 0:
               return
        speed = int(num_range_check(speed, -100, 100))
        if speed < 0:
            neu.neu_send("ROCKY", "left", int(index), [-speed])
        else:
            neu.neu_send("ROCKY","right", int(index), [speed])
        if t != None:
            time.sleep(t)
            stop()

def drive(left_power, right_power, index = 1): # not support now
    if (not type_check(left_power, int, float)) or (not type_check(right_power, int, float)):
        return
    if left_power < 0:
        left_dir = 1
        left_power = -left_power
    else:
        left_dir = 0

    if right_power < 0:
        right_dir = 0
        right_power = -right_power
    else:
        right_dir = 1
    left_power = num_range_check(left_power, -100, 100)
    right_power = num_range_check(right_power, -100, 100)    
    neu.neu_send("ROCKY","drive", index, [left_dir, left_power, right_dir, right_power])

def value_rgb(color, index = 1):
    # neu.neu_send("ROCKY", "val_rgb", index, [])
    #time.sleep(0.1)
    ret = neu.neu_read("ROCKY", "val_rgb", index, [])
    if(ret == None):
        return 0
    if color == "red":
        return ret[0]
    if color == "green":
        return ret[1]
    if color == "red":
        return ret[2]

def red(index = 1):
    #neu.neu_send("ROCKY", "val_rgb", index, [])
    #time.sleep(0.1)
    ret = neu.neu_read("ROCKY", "val_rgb", index, [])
    if(ret == None):
        print("rgb command received none")
        return 0
    else:
        return ret[0]

def green(index = 1):
    #neu.neu_send("ROCKY", "val_rgb", index, [])
    #time.sleep(0.1)
    ret = neu.neu_read("ROCKY", "val_rgb", index, [])
    if(ret == None):
        return 0
    else:
        return ret[1]

def blue(index = 1):
    #neu.neu_send("ROCKY", "val_rgb", index, [])
    #time.sleep(0.1)
    ret = neu.neu_read("ROCKY", "val_rgb", index, [])
    if(ret == None):
        return 0
    else:
        return ret[2]

# not a demand
def get_color(index = 1):
    #neu.neu_send("ROCKY", "color", 1, [])
    #time.sleep(0.01)
    ret = neu.neu_read("ROCKY", "color", 1, [])
    for item in color_dict:
        if color_dict[item] == ret:
            return item
    return ""
# this function is baed on function color
# don't use a indepedend id
def is_color(color_str, index = 1):
    #neu.neu_send("ROCKY", "color", 1, [])
    #time.sleep(0.1)
    try:
        ret = color_dict[color_str]
    except KeyError:
        print("not found the color", color_str)
        return False
    if ret == neu.neu_read("ROCKY", "color", 1, []):
        return True
    else:
        return False

def light_strength(index = 1):
    #neu.neu_send("ROCKY", "val_lightness", 1, [])
    #time.sleep(0.1)
    ret = neu.neu_read("ROCKY", "val_lightness", 1, [])
    if ret != None:
        return ret
    else:
        return 0
    
def grey(index = 1):
    #neu.neu_send("ROCKY", "val_grey", 1, [])
    #time.sleep(0.1)
    ret = neu.neu_read("ROCKY", "val_grey", 1, [])
    if ret != None:
        return ret
    else:
        return 0

def reflection_strength(index = 1):
    #neu.neu_send("ROCKY", "val_light_reflect", 1, [])
    #time.sleep(0.1)
    ret = neu.neu_read("ROCKY", "val_light_reflect", 1, [])
    if ret != None:
        return ret
    else:
        return 0

def ir_reflection_strength(index = 1):
    #neu.neu_send("ROCKY", "val_ir_reflect", 1, [])
    #time.sleep(0.1)
    ret = neu.neu_read("ROCKY", "val_ir_reflect", 1, [])
    if ret != None:
        return ret
    else:
        return 0

def is_grey_in(low, high, index = 1):
    #neu.neu_send("ROCKY", "is_in_grey", 1, [])
    #time.sleep(0.1)
    if neu.neu_read("ROCKY", "is_in_grey", 1, []):
        return True
    else:
        return False    

def is_obstacle_ahead(index = 1):
    #neu.neu_send("ROCKY", "is_barrier", 1, [])
    #time.sleep(0.1)
    if neu.neu_read("ROCKY", "is_barrier", 1, []) == 1:
        return True
    else:
        return False
    
def set_grb(r, g, b, index = 1):
    r = num_range_check(r, 0, 255)
    g = num_range_check(g, 0, 255)
    b = num_range_check(b, 0, 255)    
    neu.neu_send("ROCKY", "set_rgb", index, [r, g, b]) 

def color(color_type):
    if type_check(color_type, str):
        if color_type[0] == '#':
            r = int(color_type[1 : 3], 16)
            g = int(color_type[3 : 5], 16)
            b = int(color_type[5 : 7], 16) 
            set_grb(r, g, b)

def fade():
    set_grb(0, 0, 0)
            
def motor_current(direc, index = 1):
    #neu.neu_send("ROCKY", "val_motor_current", 1, [])
    #time.sleep(0.1)
    ret = neu.neu_read("ROCKY", "val_motor_current", 1, [])
    if ret != None:
        if direc == 'left':
            return ret[0]
        elif direc == 'right':
            return ret[1]
        else:
            return 0
    else:
        return 0

## test rocky turn a fixed angle
m_gyro = gyro_board()
def turn_right_angle(angle, speed = 40):
    global rocky_stop_flag
    if (not type_check(angle, int, float)) or (not type_check(speed, int, float)):
        return
    if angle < 0:
        turn_left_angle(-angle, speed)
        return
    speed = math.fabs(speed)
    angle_a = 50
    now_angle = m_gyro.value(2)
    turn_angle = angle * math.cos(angle_a / 180 * 3.1415926)
    dis_angle = now_angle - turn_angle

    rocky_stop_flag = False
    while True: 
        delt = m_gyro.value(2) - dis_angle
        if delt >= 5:
            drive(speed, -speed)
        if delt < 5 and delt > 2:
            drive(speed * 0.5, -speed * 0.5)
        elif delt < 1:
            break
        if rocky_stop_flag:
            rocky_stop_flag = False
            break
    drive(0, 0)

def turn_left_angle(angle, speed = 40):
    global rocky_stop_flag
    if (not type_check(angle, int, float)) or (not type_check(speed, int, float)):
        return
    if angle < 0:
        turn_right_angle(-angle, speed)
        return
    speed = math.fabs(speed)
    angle_a = 50 
    now_angle = m_gyro.value(2)
    turn_angle = angle * math.cos(angle_a / 180 * 3.1415926)
    dis_angle = now_angle + turn_angle

    rocky_stop_flag = False
    while True: 
        delt = m_gyro.value(2) - dis_angle
        if delt <= -5:
            drive(-speed, speed)
        if delt > -5 and delt < -2:
            drive(-speed * 0.5, speed * 0.5)
        elif delt > -1:
            break
        if rocky_stop_flag:
            rocky_stop_flag = False
            break
    drive(0, 0)

# just for a temporary usage
from makeblock import neurons_engine
def color_sensor_verify():
    neurons_engine().send_special(0xf0, 0x01, 0x63, 0x10, 0x6f, 0x63, 0xf7)

import gc
gc.collect()