# every block struct is a touple with 3 element,
# show the information of SENSOR_TYPE, RESPOND_PACKAGE, COMMAND_PACKAGE 
NEU_BLOCK_LIB_TYPE = 0
NEU_BLOCK_LIB_STATUS = 1
NEU_BLCOK_LIB_CMD = 2

NEU_FRAME_HEAD = 0xf0
NEU_FRAME_END = 0xf7

NEU_BYTE = 0
NEU_byte = 1
NEU_SHORT = 2
NEU_short = 3
NEU_LONG = 4
NEU_FLOAT = 5

NEU_RGB = ({(0x65, 0x02): "NEU_RGB"}, [], {"set_color": (0x01, NEU_SHORT, NEU_SHORT, NEU_SHORT)})

NEU_BUTTON = ({(0x64, 0x02): "NEU_BUTTON"}, [0x01, NEU_BYTE], {'is_pressed':(0x01,)})

NEU_SENSORS_LIB ={"NEU_RGB" : NEU_RGB, "NEU_BUTTON" : NEU_BUTTON} 

neu_cur_blcok_list = {}
def neu_cur_blcok_list_add(device_id, sensor_type, sub_type):   
    temp = [device_id, sensor_type, sub_type]
    for item in NEU_SENSORS_LIB:
        


def neu_oprate(sensor, index, fun_str, *data):
    cmd_buffer = []
    cmd_buffer.append(NEU_FRAME_HEAD)
    
    cmd_buffer.append(sensor[])

