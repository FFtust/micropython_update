from makeblock import ledmatrix
import time
from codey_global_board import *
NUM_0 = [124, 68, 124]
NUM_1 = [0, 124, 0]
NUM_2 = [92, 84, 116]
NUM_3 = [84, 84, 124]
NUM_4 = [112, 16, 124]
NUM_5 = [116, 84, 92]
NUM_6 = [124, 84, 92]
NUM_7 = [64, 64, 124]
NUM_8 = [124, 84, 124]
NUM_9 = [116, 84, 124]
NUM_DOT = [0, 4, 0]
NUM_NEG = [32, 32, 32]
LED_NUM_DATA = [NUM_0, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9, NUM_DOT, NUM_NEG]

class codey_ledmatrix(ledmatrix):
    # ledmatrix_board : clean() , set_brightness() , init() , deinit()
    # string_show(), number_show(), animation_show(), faceplate_show(), faceplate_show_with_time(),pixel_control
    def __init__(self):
        pass

    def help(self):
        dir(codey_ledmatrix)

    def show_persent(self, val):
        cent = [128, 71, 37, 23, 232, 164, 226, 1]
        temp_face = [0] * 16

        if val < 0:
            val = 0 
        elif val > 99:
            val = 99

        str_num = str(val)
        cross_num = 0

        for da in str_num:
            for i in range(3):
                temp_face[cross_num] = LED_NUM_DATA[int(da)][i]
                cross_num += 1   
            cross_num += 1
        
        for i in range(8, 16):
            temp_face[23 - i] = cent[i - 8]

        self.faceplate_show(0, 0, *temp_face)

    



