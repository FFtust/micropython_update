# makeblock lib import
from makeblock import button_board
from makeblock import light_sensor_board
from makeblock import sound_sensor_board
from makeblock import potentionmeter_board
from makeblock import gyro_board
from makeblock import message_board
from makeblock import music
from makeblock import rmt_board
import math
# event index#

## MAGIC BUMBER
### FOR SENSOR VALUE UNIT

VAL_TO_SENSOR = 100 / 4095  # range to 0 to 100
BEAT_TO_SECOND = 1
## about button
BUTTON1_PRESS = 1
BUTTON1_RELEASE = 2
BUTTON2_PRESS = 3
BUTTON2_RELEASE = 4
BUTTON3_PRESS = 5
BUTTON3_RELEASE = 6
## about sound sensor
SOUND_OVER = 7
SOUND_BELOW = 8
## about light sensor
LIGHT_OVER = 9
LIGHT_BELOW = 10
## about message
## hold 12-15 for later usage
MESSAGE = 11

## about gyro 
BOARD_SHAKE = 16
BOARD_TILT_LEFT = 17
BOARD_TILT_RIGHT = 18
BOARD_TILT_FORWARD = 19
BOARD_TILT_BACK = 20
BOARD_SCREEN_UP = 21
BOARD_SCREEN_DOWN = 22
BOARD_FREE_FALL = 23 

## codey start 
CODEY_LAUNCH = 24
########################################
# SENSOR INDEX 

## BUTTON
BUTTON_A = 1
BUTTON_B = 2
BUTTON_C = 3

### RGB color
RGB_RED_INDEX = 0
RGB_GREEN_INDEX = 1
RGB_BLUE_INDEX = 2

### NODE TABLE
node_table = \
{ \
  'C2': 65,
  'D2': 73,
  'E2': 82,
  'F2': 87,
  'G2': 98,
  'A2': 110,
  'B2': 123,
  'C3': 131,
  'D3': 147,
  'E3': 165,
  'F3': 175,
  'G3': 196,
  'A3': 220,
  'B3': 247,
  'C4': 262,
  'D4': 294,
  'E4': 330,
  'F4': 349,
  'G4': 392,
  'A4': 440,
  'B4': 494,
  'C5': 523,
  'D5': 587,
  'E5': 659,
  'F5': 698,
  'G5': 784,
  'A5': 880,
  'B5': 988,
  'C6': 1047,
  'D6': 1175,
  'E6': 1319,
  'F6': 1397,
  'G6': 1568,
  'A6': 1760,
  'B6': 1976,
  'C7': 2093,
  'D7': 2349,
  'E7': 2637,
  'F7': 2794,
  'G7': 3136,
  'A7': 3520,
  'B7': 3951,
  'C8': 4186,
  'D8': 4699,
}

MIDI_NOTE_NUM0 = 8.18
NOTE_FREQUENCE_RATIO = math.pow(2, (1 / 12))
########################################
# SENSOR OPERATION 
## BUTTON
BUTTON_PRESSED = 1
BUTTON_RELEASED = 0

########################################
# system cionfig 
## events thread stack size
THREAD_STACK_SIZE_IS_DEFAULT = True
THREAD_TOTAL_STATCK_SIZE = 30 * 1024 #30K
THREAD_MAX_NUM = 16 # 6 support now
########################################
#for common use
## check the type
def type_check(para, type_1, type_2 = None, type_3 = None):
    if type(para) == type_1:
        return True
    if type_2 != None:
        if type(para) == type_2:
            return True
    if type_3 != None:
        if type(para) == type_3:
            return True
    return False

# check the range of number
def num_range_check(num, min_n = None, max_n = None, to_range = True):
    if min_n == None and max_n == None:
        return num
    
    if min_n != None:
        if to_range and num < min_n:
            num = min_n

    if max_n != None:
        if to_range and num > max_n:
            num = max_n
    return num

# debug 
CODEY_DEBUG = True
def start_dbg_out():
    global CODEY_DEBUG
    CODEY_DEBUG = True

def stop_dbg_out():
    global CODEY_DEBUG
    CODEY_DEBUG = False

def print_dbg(*args):
    global CODEY_DEBUG
    if CODEY_DEBUG:
        print(*args)

stop_dbg_out()

# light and sound sensor value range
def sound_value_range(val):
    tem = val * 1.34 - 3
    tem = num_range_check(tem, 0.0, 100.0)
    return tem