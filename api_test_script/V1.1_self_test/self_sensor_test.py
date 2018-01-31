import codey
import time 
import rocky

SEN_BUTTON = 1
SEN_LIGHT = 2
SEN_SOUND = 3
SEN_DAIL = 4
SEN_GYRO = 5
SEN_IR_REC = 6
SEN_RGB = 7
SEN_LEDM = 8
SEN_IR_SEND = 9
SEN_ROCKY_SEND = 10
SEN_ROCKY_READ = 11
SEN_SPEAKER = 12
SEN_ID_MAX = 12
sensor_id = 0

SENSOR_VALUE = 1
SENSOR_INFO = 2
info_show_id = 0

def button_test():
    print("A is pressed:", codey.is_button('A'))
    print("B is pressed:", codey.is_button('B'))
    print("C is pressed:", codey.is_button('C'))

def light_test():
    print("light value is:", codey.light_strength())

def sound_test():
    print("sound value is:", codey.sound_strength())

def dail_test():
    print("dail value is:", codey.dail())

def gyro_test():
    print("acc_x is:", codey.gyro_a('x'))
    print("acc_y is:", codey.gyro_a('y'))
    print("acc_z is:", codey.gyro_a('z'))
    print("pitch is:", codey.gyro('pitch'))
    print("roll is:", codey.gyro('roll'))
    print("yaw is:", codey.gyro('yaw'))
    if codey.is_tilt("forward"):
        print("codey tilt is forward")
    if codey.is_tilt("backward"):
        print("codey tilt is backward")
    if codey.is_tilt("left"):
        print("codey tilt is left")
    if codey.is_tilt("right"):
        print("codey tilt is right")

def irrec_test():
    print("ir send abc")
    codey.ir_send("abc")
    print("irrec value is:", codey.ir_receive())

def rgb_test():
    print("red on")
    codey.color("#ff0000", 1)
    print("green on")
    codey.color("#00ff00", 1)
    print("blue on")
    codey.color("#0000ff", 1)
    print("single red on")
    codey.red(255)
    time.sleep(1)
    print("single green on")
    codey.green(255)
    time.sleep(1)
    print("single blue on")
    codey.blue(255)
    time.sleep(1)
    print("single red off")
    codey.red(0)
    time.sleep(1) 
    print("single green off")
    codey.green(0)
    time.sleep(1)
    print("single blue off")
    codey.blue(0)       
    time.sleep(1)

def ledm_test():
    print("all led on")
    codey.face("ffffffffffffffffffffffffffffffff", 3)
    print("all led off")
    print('(0, 0) on')
    codey.pixel(0, 0)
    time.sleep(1)
    print("(0, 0) off")
    codey.pixel_off(0, 0)
    time.sleep(1)
    print("(0, 0) toggle")
    codey.pixel_toggle(0, 0)
    time.sleep(1)

def irsend_test():
    print("ir send abc")
    codey.ir_send("abc")

def rocky_send():
    print("rocky forward")
    rocky.forward(100, 2)
    print("rocky back")
    rocky.backward(100, 2)   
    print("rocky left")    
    rocky.turn_left(100, 2)
    print("rocky right")
    rocky.turn_right(100, 2)
    print("rocky stop")
    print("rocky set rgb color white")
    rocky.color("#ffffff")
    time.sleep(1)
    print("rocky set rgb color red")
    rocky.color("#ff0000")

def rocky_read():
    print("rocky color is:", rocky.get_color()) # not a demand 
    print("red value is:", rocky.red())
    print("green value is:", rocky.green())
    print("blue value is:", rocky.blue())    
    print("rocky grey is:", rocky.grey())
    print("rocky obstacle is:", rocky.is_obstacle_ahead())
    print("rocky light is:", rocky.light_strength())
    print("rocky light reflect is:", rocky.reflection_strength()) 
    print("rocky ir reflect is:", rocky.ir_reflection_strength()) 
    print("rocky left motor current is ", rocky.motor_current('left'))
    rocky.forward(100)
    time.sleep(1)
    print("rocky left motor current is ", rocky.motor_current('left'))
    rocky.stop()
    print("")

def speaker_test():
    print("speaker say cat")
    codey.say('meow.wav', True)
    print("play node")
    codey.play('C5', 1)
    print("pause")
    print("pre time value is ", codey.time())
    codey.pause(1)
    print("later time value is ", codey.time())
    print("play freequence")
    codey.play_freq(523, 1)

def sensor_show_info():
    print("**********")
    if sensor_id == SEN_BUTTON:
        print("button test")
    if sensor_id == SEN_LIGHT:
        print("light test")
    if sensor_id == SEN_SOUND:
        print("sound test")
    if sensor_id == SEN_DAIL:
        print("dail test")
    if sensor_id == SEN_GYRO:
        print("gyro test")
    if sensor_id == SEN_IR_REC:
        print("ir rec test")
    if sensor_id == SEN_RGB:
        print("rgb test")  
    if sensor_id == SEN_LEDM:
        print("led matrix test")
    if sensor_id == SEN_IR_SEND:
        print("ir send test")
    if sensor_id == SEN_ROCKY_SEND:
        print("neurons send test")
    if sensor_id == SEN_ROCKY_READ:
        print("neurons read test")
    if sensor_id == SEN_SPEAKER:
        print("speaker test")
    print("**********")
def sensor_control():
    if sensor_id == SEN_BUTTON:
        button_test()
    if sensor_id == SEN_LIGHT:
        light_test()
    if sensor_id == SEN_SOUND:
        sound_test()
    if sensor_id == SEN_DAIL:
        dail_test()
    if sensor_id == SEN_GYRO:
        gyro_test()
    if sensor_id == SEN_IR_REC:
        irrec_test()
    if sensor_id == SEN_RGB:
        rgb_test()
    if sensor_id == SEN_LEDM:
        ledm_test()
    if sensor_id == SEN_IR_SEND:
        irsend_test()
    if sensor_id == SEN_ROCKY_SEND:
        rocky_send()
    if sensor_id == SEN_ROCKY_READ:
        rocky_read()
    if sensor_id == SEN_SPEAKER:
        speaker_test()

def sensor_test():
    global sensor_id
    global info_show_id

    if codey.is_button('A'):
        while codey.is_button('A'):
            time.sleep(0.05)
        if sensor_id > 0:
            sensor_id -= 1
        else:
            sensor_id = SEN_ID_MAX
        sensor_show_info()

    if codey.is_button('B'):
        while codey.is_button('B'):
            time.sleep(0.05)
        if sensor_id < SEN_ID_MAX:
            sensor_id += 1 
        else:
            sensor_id = 1
        sensor_show_info()

    if codey.is_button('C'):
        while codey.is_button('C'):
            time.sleep(0.05)
        sensor_control()

    time.sleep(0.1)
    

def call_back():
    codey.face('00001020402012020212204020100000', 1)
    while True:
        sensor_test() 

# about codey
codey.face('00001020402012020212204020100000', 1)
codey.face('00001020402012020212204020100000')
codey.show('hello world')
codey.clear()
codey.pixel(0, 0)
codey.pixel_off(0, 0)
codey.pixel_toggle(0, 0)
codey.color('#334455', 1)
codey.color('#334455')
codey.color_off()
codey.red(255)
codey.green(255)
codey.blue(0)
codey.say('cat') 
codey.say('cat.wav')
codey.say('cat', True)
codey.mute()
codey.play_note(50, 1)
codey.pause(0.25)
codey.play_freq(700, 1)
codey.change_volume(-10)
codey.set_volume(100)
codey.get_volume()
codey.message("a")
codey.is_button('A')
codey.is_shaked()
codey.is_tilt('forward')
codey.sound_strength()
codey.light_strength()
codey.dail()
codey.gyro('pitch')
codey.time()
codey.reset_time()
codey.ir_send("A")
codey.ir_receive()

print("***codey APIS all succeed***")
# about rocky
rocky.color("#334455")
rocky.forward(50, 1)
rocky.backward(50, 1)
rocky.turn_left(50, 1)
rocky.turn_right(50, )
rocky.forward(50)
rocky.backward(50)
rocky.turn_left(50)
rocky.turn_right(50)
rocky.drive(50, 50)
rocky.turn_left_angle(15)
rocky.turn_right_angle(15)
rocky.stop()

rocky.is_obstacle_ahead()
rocky.is_color('red')
rocky.red()
rocky.green()
rocky.blue()
rocky.reflection_strength()
rocky.light_strength()
rocky.ir_reflection_strength()
rocky.grey()
print("***rocky APIS all succeed***")


codey.on_button('A', call_back) 