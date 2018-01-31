import codey

test_num = 0

def on_button2_callback():
    global test_num
    test_num = 3
    codey.color("#ff0000", 0.2)

codey.on_button('C', on_button2_callback)

def on_button1_callback():
    global test_num
    test_num = 1
    codey.color("#ff0000", 0.2)

codey.on_button('A', on_button1_callback)

def on_message_callback():
    global test_num
    test_num = 4

codey.on_message(str('hello'), on_message_callback)

def on_button_callback():
    global test_num
    test_num = 2
    codey.color("#ff0000", 0.2)

codey.on_button('B', on_button_callback)

def on_light_under_callback():
    global test_num
    test_num = 5
    codey.color("#ff0000", 0.2)
codey.on_light_under(10, on_light_under_callback)

def start_cb():
    global test_num
    test_num = 0
    codey.message(str('hello'))
    print("button and message test begin")
    while True:
        if test_num == 1:
            print("button_A event succeed")
            test_num = 0
        elif test_num == 2:
            print("button_B event succeed")
            test_num = 0
        elif test_num == 3:
            print("button_C event succeed")
            test_num = 0
        elif test_num == 4:
            print("message event succeed")
            test_num = 0
        if test_num == 5:
            print("light event succeed")
            test_num = 0

codey.on_start(start_cb)