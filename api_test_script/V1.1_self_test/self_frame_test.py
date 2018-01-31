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
SEN_ID_MAX = 11
sensor_id = 0

SENSOR_VALUE = 1
SENSOR_INFO = 2
info_show_id = 0

def button_test():
    print("A is pressed:", codey.is_button('A'))
    print("B is pressed:", codey.is_button('B'))
    print("C is pressed:", codey.is_button('C'))

def light_test():
    print("light value is:", codey.light_sensor())

def sound_test():
    print("sound value is:", codey.sound_sensor())

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

def ledm_test():
    print("all led on")
    codey.face("ffffffffffffffffffffffffffffffff", 3)
    print("all led off")

def irsend_test():
    print("ir send abc")
    codey.ir_send("abc")

def rocky_send():
    print("rocky forward")
    rocky.forward(100, 2)
    print("rocky back")
    rocky.back(100, 2)   
    print("rocky left")    
    rocky.left(100, 2)
    print("rocky right")
    rocky.right(100, 2)
    print("rocky stop")

def rocky_read():
    print("rocky color is:", rocky.color())
    print("rocky grey is:", rocky.grey('left'))
    print("rocky obstacle is:", rocky.is_obstacle_ahead())
    print("rocky light is:", rocky.light_intensity('left'))

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

def sensor_test():
    global sensor_id
    global info_show_id

    if codey.is_button('A'):
        while codey.is_button('A'):
            pass
        if sensor_id > 0:
            sensor_id -= 1
        else:
            sensor_id = SEN_ID_MAX
        sensor_show_info()

    if codey.is_button('B'):
        while codey.is_button('B'):
            pass
        if sensor_id < SEN_ID_MAX:
            sensor_id += 1 
        else:
            sensor_id = 1
        sensor_show_info()

    if codey.is_button('C'):
        while codey.is_button('C'):
            pass
        sensor_control()

    time.sleep(0.1)
    

while True:
    sensor_test()