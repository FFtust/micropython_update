from makeblock import rgbled_board
import time
from codey_global_board import *

class codey_rgbled(rgbled_board):
    # rgb_led_board stop() ,set_color()
    def fade(self):
        self.set_color(0, 0, 0)
    
    def color(self, color_type):
        if type_check(color_type, str):
            if color_type[0] == '#':
                r = int(color_type[1 : 3], 16)
                g = int(color_type[3 : 5], 16)
                b = int(color_type[5 : 7], 16) 
                self.set_color(r, g, b)
            
    def set_rgb(self, r, g, b):
        self.set_color(r, g, b)

    def blink(self, num = 3):
        for i in range(num):
            self.set_color(50, 50, 50)
            time.sleep(0.5)
            self.set_color(0, 0, 0)
            time.sleep(0.5)
            
    def mhelp(self):
        print("makeblock:please call show_char() how_painting() how_set_time")

   




