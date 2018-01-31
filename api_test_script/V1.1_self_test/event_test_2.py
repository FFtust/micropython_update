import codey

test_num = 0

def on_tilt_callback():
    global test_num
    test_num = 1
    codey.color("#ff000", 0.5)

codey.on_tilt('forward', on_tilt_callback)

def on_sound_over_callback():
    global test_num
    test_num = 5
    codey.color("#000ff0", 0.5)

codey.on_sound_over(20, on_sound_over_callback)

def on_tilt1_callback():
    global test_num
    test_num = 2
    codey.color("#0000ff", 0.5)

codey.on_tilt('backward', on_tilt1_callback)

def on_tilt2_callback():
    global test_num
    test_num = 3
    codey.color("#ffffff", 0.5)

codey.on_tilt('left', on_tilt2_callback)

def on_shake_callback():
    global test_num
    test_num = 6

codey.on_shake(on_shake_callback)

def on_tilt3_callback():
    global test_num
    test_num = 4
    codey.color("#223344", 0.5)
    
codey.on_tilt('right', on_tilt3_callback)

def start_cb():
    global test_num
    test_num = 0
    codey.message(str('hello'))
    print("button and message test begin")
    while True:
        if test_num == 1:
            print("tilt forward event succeed")
            test_num = 0
        elif test_num == 2:
            print("tilt back event succeed")
            test_num = 0
        elif test_num == 3:
            print("tilt left event succeed")
            test_num = 0
        elif test_num == 4:
            print("tilt right event succeed")
            test_num = 0
        if test_num == 5:
            print("sound event succeed")
            test_num = 0
        if test_num == 6:
            print("shake event succeed")
            test_num = 0

codey.on_start(start_cb)