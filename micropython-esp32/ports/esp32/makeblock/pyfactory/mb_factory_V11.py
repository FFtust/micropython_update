# refer to B-V1.1-20170803-DH

import meos
import time
import _thread

# define gloabal var

## check if there is a meos car connect to meos 
IS_CAR_EXIST = False 

## button index mark
BUTTON_A = 1
BUTTON_B = 2
BUTTON_C = 3

BUTTON_PRESSED = 1
BUTTON_RELEASED = 0

BUTTON_EVENT_NONE = 0
BUTTON_EVENT_PRESSED = 1
BUTTON_EVENT_RELEASED = 2
BUTTON_EVENT_LONG_PRESSED = 3

## led matrix table define
MODE1_FACE_TABLE = [128, 190, 138, 138, 0, 62, 10, 62, 0, 62, 34, 34, 0, 62, 42, 42]
FACE_1 = [60,66,153,189,189,153,66,60,60,66,153,189,189,153,66,60]
FACE_2 = [48,48,48,48,48,48,0,0,0,0,48,48,48,48,48,48]
FACE_3 = [24,56,56,24,8,8,8,0,24,56,56,24,8,8,8,0]
FACE_4 = [32,48,24,12,24,48,32,0,32,48,24,12,24,48,32,0]
FACE_5 = [60,66,153,165,149,69,58,0,60,66,153,165,149,69,58,0] # dizziness
FACE_MATRIX = [FACE_2, FACE_1, FACE_3, FACE_4, FACE_5]


MODE1_DICE_TABLE = [0, 62, 34, 28, 0, 34, 190, 162, 128, 190, 34, 34, 0, 62, 34, 34]

DICE_START = [0, 0, 126, 129, 102, 102, 0, 102, 102, 0, 102, 102, 129, 126, 0, 0]
DICE_ONE    = [0, 0, 126, 129, 0, 0, 0, 24, 24, 0, 0, 0, 129, 126, 0, 0]
DICE_TWO    = [0, 0, 126, 129, 6, 6, 0, 0, 0, 0, 96, 96, 129, 126, 0, 0]
DICE_THREE  = [0, 0, 126, 129, 6, 6, 0, 24, 24, 0, 96, 96, 129, 126, 0, 0]
DICE_FOUR   = [0, 0, 126, 129, 102, 102, 0, 0, 0, 0, 102, 102, 129, 126, 0, 0]
DICE_FIVE   = [0, 0, 126, 129, 102, 102, 0, 24, 24, 0, 102, 102, 129, 126, 0, 0]
DICE_SIX    = DICE_START

DICE_ONE_INDEX   = 0
DICE_TWO_INDEX   = 1
DICE_THREE_INDEX = 2
DICE_FOUR_INDEX  = 3
DICE_FIVE_INDEX  = 4
DICE_SIX_INDEX   = 5
DICE_START_INDEX = 6
DICE_MATRIX = [DICE_ONE, DICE_TWO, DICE_THREE, DICE_FOUR, DICE_FIVE, DICE_SIX, DICE_START]

MODE1_TIME_TABLE = [0, 2, 62, 2, 0, 34, 62, 34, 0, 62, 60, 62, 128, 190, 162, 162]
TIME_START = [0, 0, 60, 66, 129, 137, 145, 137, 133, 66, 60, 0, 4, 126, 4, 0]
TIME_MAX   = [62, 2, 62, 2, 62, 0, 60, 10, 10, 60, 0, 34, 20, 8, 20, 34]

MODE1_MATRIX = [MODE1_FACE_TABLE, MODE1_DICE_TABLE, MODE1_TIME_TABLE]
## mode1 & submode 
MODE1_SUBMODE_MIN  = 0
MODE1_SUBMODE_FACE = 0
MODE1_SUBMODE_DICK = 1
MODE1_SUBMODE_TIME = 2
MODE1_SUBMODE_MAX  = 2



# geneal function 
## 
class button_event(object):
    def __init__(self):
        self.button_event_flag_clear()

    def button_event_check(self):    
        while True:
            self.current_button_a_value = meos.readButton(BUTTON_A) 
            self.current_button_b_value = meos.readButton(BUTTON_B) 
            self.current_button_c_value = meos.readButton(BUTTON_C)

            if self.current_button_a_value != self.last_button_a_value: 
                if self.current_button_a_value == BUTTON_PRESSED:  # button press
                    self.button_a_pressed_flag = True
                else:                                         # button release
                    self.button_a_released_flag = True
            if self.current_button_b_value != self.last_button_b_value: 
                if self.current_button_b_value == BUTTON_PRESSED:  # button press
                    self.button_b_pressed_flag = True
                else:                                         # button release
                    self.button_b_released_flag = True
            if self.current_button_c_value != self.last_button_c_value: 
                if self.current_button_c_value == BUTTON_PRESSED:  # button press
                    self.button_c_pressed_flag = True
                    self.button_c_pressed_start_time = time.ticks_ms()
                else:                                         # button release
                    self.button_c_released_flag = True
            elif self.last_button_c_value == BUTTON_PRESSED:       # check if there is a long press 
                if time.ticks_ms() - self.button_c_pressed_start_time >= 2000: # 2s
                    self.button_c_pressed_long_falg = True
                    self.button_c_pressed_flag = False

            self.last_button_a_value = self.current_button_a_value
            self.last_button_b_value = self.current_button_b_value
            self.last_button_c_value = self.current_button_c_value
            time.sleep(0.05)

    def button_event_check_start(self):
        _thread.start_new_thread(self.button_event_check, ())

    def is_button_a_pressed(self):
        tempdata = self.button_a_pressed_flag
        self.button_a_pressed_flag = False  
        return tempdata
    def is_button_a_released(self):
        tempdata = self.button_a_released_flag
        self.button_a_released_flag = False  
        return tempdata        
    def is_button_b_pressed(self):
        tempdata = self.button_b_pressed_flag
        self.button_b_pressed_flag = False  
        return tempdata
    def is_button_b_released(self):
        tempdata = self.button_b_released_flag
        self.button_b_released_flag = False  
        return tempdata  
    def is_button_c_pressed(self):
        tempdata = self.button_c_pressed_flag
        self.button_c_pressed_flag = False  
        return tempdata
    def is_button_c_released(self):
        tempdata = self.button_c_released_flag
        self.button_c_released_flag = False  
        return tempdata 
    def is_button_c_long_pressed(self):
        tempdata = self.button_c_pressed_long_falg
        if self.button_c_pressed_long_falg == True:
            self.button_c_pressed_flag = False      # clear button_c single press flag
            self.button_c_pressed_long_falg = False  
            self.button_c_pressed_start_time = time.ticks_ms()
        return tempdata  

    def get_button_event(self, button_index = None):
        ret = [0, 0, 0, 0]
        if self.is_button_a_pressed() == True:
            ret[1] = 1
        elif self.is_button_a_released() == True:
            ret[1] = 2
        else:
            ret[1] = 0

        if self.is_button_b_pressed() == True:
            ret[2] = 1
        elif self.is_button_b_released() == True:
            ret[2] = 2
        else:
            ret[2] = 0

        if self.is_button_c_pressed() == True:
            ret[3] = 1
        elif self.is_button_c_released() == True:
            ret[3] = 2
        elif self.is_button_c_long_pressed() == True:
            ret[3] = 3
        else:
            ret[3] = 0
        if button_index == None:
            return ret
        else:
            return ret[button_index]



    def button_event_flag_clear(self):
        self.last_button_a_value = BUTTON_RELEASED
        self.current_button_a_value = BUTTON_RELEASED
        self.button_a_pressed_flag = False
        self.button_a_released_flag = False

        self.last_button_b_value = BUTTON_RELEASED
        self.current_button_b_value = BUTTON_RELEASED
        self.button_b_pressed_flag = False
        self.button_b_released_flag = False

        self.last_button_c_value = BUTTON_RELEASED
        self.current_button_c_value = BUTTON_RELEASED
        self.button_c_pressed_flag = False
        self.button_c_released_flag = False
        self.button_c_pressed_start_time = 0
        self.button_c_pressed_long_falg = False           

## hello interface
def show_hello_interface():
    global OUT_HELLO_FLAG
    global button_eve 
    hello_str = "HELLO HELLO "
    time_pice_num = 0
    time_pice = 0.1 # second
    p_x = 0 

    num = len(hello_str)
    sstr = (list)(hello_str)

    for i in range(num):
        sstr[i] = ord(sstr[i])   
    sstr_show = sstr
    button_c_in_status = meos.readButton(BUTTON_C)
    while True:
        if(button_c_in_status == BUTTON_RELEASED):
            #if button_eve.is_button_c_released() == True:
            if button_eve.get_button_event(BUTTON_C) == BUTTON_EVENT_RELEASED:
                meos.cleanScreen()
                break
        elif button_c_in_status == BUTTON_PRESSED:
            #if button_eve.is_button_c_released() == True:
            if button_eve.get_button_event(BUTTON_C) == BUTTON_EVENT_RELEASED:
                button_c_in_status = BUTTON_RELEASED

        if time_pice_num == 18:
            sstr_show = (list)(sstr[3 : num])
            p_x = 0
        elif time_pice_num == 36:
            sstr_show = (list)(sstr)
            p_x = 0

        meos.showLEDChars(8, sstr_show, p_x, 0)
        p_x = p_x - 1
        if time_pice_num >= 36:
            #p_x = 0
            time_pice_num = 1

        time.sleep(time_pice)
        time_pice_num = time_pice_num + 1
# for no car 

## main execute function in no car mode 
### face change function 
def face_change_function():
    global button_eve
    index = 1
    light_flag_low = 0
    face_change_flag = 0
    sound_flag_high = 0

    meos.showLEDPainting(FACE_2)

    while True:
        light_value = meos.readLightSensor()
        sound_value = meos.readSoundSensor()
        
        if light_value < 50:
            light_flag_low = 1

        if light_flag_low == 1 and light_value > 250:
            face_change_flag = 1
         
        if sound_value > 50:
            meos.showLEDPainting(FACE_MATRIX[4])
            meos.playSoundEffect("Pickup_Coin7_8k_8bit.wav",True)
            time.sleep(2)
            meos.showLEDPainting(FACE_MATRIX[0])

        if face_change_flag == 1:
            meos.showLEDPainting(FACE_MATRIX[index]) 
            if(index % 2 == 0):
                meos.playSoundEffect("Jump8_8k_8.wav",True)
            else:
                meos.playSoundEffect("Pickup_Coin7_8k_8bit.wav",True)
            index = (index + 1) % 4
            light_flag_low = 0
            face_change_flag = 0

        #if button_eve.is_button_c_long_pressed() == True:
        if button_eve.get_button_event(BUTTON_C) == BUTTON_EVENT_LONG_PRESSED:
            break 

        time.sleep(0.1)

### dice function
def dice_function():
    meos.showLEDPainting(DICE_MATRIX[DICE_START_INDEX])
    while True:
        acc_xyz = meos.readGyroRawData(1)
        if acc_xyz[0] > 30000 or acc_xyz[1] > 30000 or acc_xyz[2] > 30000:
            t = time.ticks_ms()
            t = t % 6
            meos.showLEDPainting(DICE_MATRIX[t])
            meos.playSoundEffect("Jump8_8k_8.wav",True)
        time.sleep(0.1)
        #if button_eve.is_button_c_long_pressed() == True:
        if button_eve.get_button_event(BUTTON_C) == BUTTON_EVENT_LONG_PRESSED:
            break

### time function
def timer_function():
    global button_eve
    
    TIEMR_FSM_START_WAIT= 0
    TIMER_FSM_START = 1
    TIMER_FSM_STOP = 2
    TIMER_FSM_RESET = 3

    timer_count_100ms_max = 10 * 60 * 10

    meos.showLEDPainting(TIME_START)

    TIMER_FSM_STATUS = TIEMR_FSM_START_WAIT
    TIMER_FSM_STATUS_CHANGED_FLAG = False
    while True:
        evens = button_eve.get_button_event() 
        if TIMER_FSM_STATUS == TIEMR_FSM_START_WAIT:
            if TIMER_FSM_STATUS_CHANGED_FLAG == True:
                meos.showLEDPainting(TIME_START) 

            #if button_eve.is_button_b_released() == True:
            #if button_eve.get_button_event(BUTTON_B) == BUTTON_EVENT_RELEASED:  
            if evens[BUTTON_B] ==  BUTTON_EVENT_RELEASED:         
                timer_start_time = time.ticks_ms()  # unit: ms
                timer_current_time = timer_start_time
                timer_last_time = timer_current_time

                TIMER_FSM_STATUS = TIMER_FSM_START
                TIMER_FSM_STATUS_CHANGED_FLAG = True
            else:
                TIMER_FSM_STATUS_CHANGED_FLAG = False

        elif TIMER_FSM_STATUS == TIMER_FSM_START:
            timer_current_time = time.ticks_ms()
            if timer_current_time - timer_last_time >= 100:
                timer_count_100ms = (timer_current_time - timer_start_time) // 100
                if timer_count_100ms >= timer_count_100ms_max:
                    meos.showLEDPainting(TIME_MAX)
                else:
                    meos.showTimer(timer_count_100ms)

                timer_last_time = timer_current_time
            
            #if button_eve.is_button_b_released() == True:
            #if button_eve.get_button_event(BUTTON_B) == BUTTON_EVENT_RELEASED:    
            if evens[BUTTON_B] == BUTTON_EVENT_RELEASED:
                TIMER_FSM_STATUS = TIMER_FSM_STOP
                TIMER_FSM_STATUS_CHANGED_FLAG = True
            else:
                TIMER_FSM_STATUS_CHANGED_FLAG = False


        elif TIMER_FSM_STATUS == TIMER_FSM_STOP:
            #if button_eve.is_button_a_released():
            #if button_eve.get_button_event(BUTTON_A) == BUTTON_EVENT_RELEASED:
            if evens[BUTTON_A] == BUTTON_EVENT_RELEASED:
                TIMER_FSM_STATUS = TIMER_FSM_RESET
                TIMER_FSM_STATUS_CHANGED_FLAG = True
            else:
                TIMER_FSM_STATUS_CHANGED_FLAG = False

        elif TIMER_FSM_STATUS == TIMER_FSM_RESET:
            if TIMER_FSM_STATUS_CHANGED_FLAG == True:
                meos.showTimer(0)

            #if button_eve.is_button_b_released() == True:
            #if button_eve.get_button_event(BUTTON_B) == BUTTON_EVENT_RELEASED:
            if evens[BUTTON_B] == BUTTON_EVENT_RELEASED:
                timer_start_time = time.ticks_ms()  # unit: ms
                timer_current_time = timer_start_time
                timer_last_time = timer_current_time

                TIMER_FSM_STATUS = TIMER_FSM_START
                TIMER_FSM_STATUS_CHANGED_FLAG = True
            else:
                TIMER_FSM_STATUS_CHANGED_FLAG = False

        #if button_eve.is_button_c_long_pressed() == True:
        #if button_eve.get_button_event(BUTTON_C) == BUTTON_EVENT_LONG_PRESSED:
        if evens[BUTTON_C] == BUTTON_EVENT_LONG_PRESSED: 
            break
        time.sleep(0.1)



# no car main function
def no_car_execute_function():
    global button_eve

    show_hello_interface()

    mode1_submode = MODE1_SUBMODE_FACE   
    mode1_stage = 0 
    mode_choose_current_index = MODE1_SUBMODE_FACE
    mode_choose_changed = True
    # choose mode 
    while True:
        if mode1_stage == 0:
            eves = button_eve.get_button_event() 
            #if button_eve.is_button_a_released() == True:
            if eves[BUTTON_A] == BUTTON_EVENT_RELEASED:
                mode_choose_changed = True
                if mode_choose_current_index > MODE1_SUBMODE_MIN:
                    mode_choose_current_index = mode_choose_current_index - 1 

            #elif button_eve.is_button_b_released() == True:
            if eves[BUTTON_B] == BUTTON_EVENT_RELEASED:
                mode_choose_changed = True
                if mode_choose_current_index < MODE1_SUBMODE_MAX:
                    mode_choose_current_index = mode_choose_current_index + 1

            #elif button_eve.is_button_c_released() == True:   # confirm button 
            if eves[BUTTON_C] == BUTTON_EVENT_RELEASED:
                mode1_submode = mode_choose_current_index
                mode1_stage = 1

            #if button_eve.is_button_c_long_pressed():
            elif eves[BUTTON_C] == BUTTON_EVENT_LONG_PRESSED:
                break

            if mode_choose_changed == True:
                meos.showLEDPainting(MODE1_MATRIX[mode_choose_current_index])
                mode_choose_changed = False

        elif mode1_stage == 1:
            #in mode
            if mode1_submode == MODE1_SUBMODE_FACE:
                face_change_function()
            elif  mode1_submode == MODE1_SUBMODE_DICK:
                dice_function()
            elif mode1_submode == MODE1_SUBMODE_TIME:
                timer_function()

            break

        time.sleep(0.1)

#for car exist


## main execute function in car exist mode
def car_exist_execute_function():
    while True:
        pass
 

button_eve = button_event()
def main (): 
    global button_eve
    button_eve.button_event_check_start()

    while True:
        if 0 :    # it will be a check function later
            IS_CAR_EXIT = True
            car_exist_execute_function()
        else :
            IS_CAR_EXIT = False
            no_car_execute_function()

if __name__ == "__main__":
    main()


