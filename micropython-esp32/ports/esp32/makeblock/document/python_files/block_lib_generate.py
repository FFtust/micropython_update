from struct import *
from block_lib_data import *

FLASH_LIB_FILE_NAME = "neurons_engine_lib.bin"


# the head information */
# total 128bytes */
lib_head_dict = {"version": 32, "single_byte_num": 4, "type_num": 4, "info": 92}
LIB_HEAD_LEN = 128
LIB_VERSION = "V1.0alpha1"
LIB_SINGLE_LIB_NUM = 62
LIB_TYPE_NUM = 1 # only 0x61 exist as a test
lib_info = [[0x63, 0x0d + 1, 128], [0x65, 0x06 + 1, 128 + 62 * (0x0d + 1)]]
#           [byte1, byte2, short3]
BLOCK_LIB_BYTES_NUM = 62   #ser_id(1) + sub_id(1) + 5 * respond_item(6) + 5 * command_item(6)
BLOCK_LIB_RESPOND_ITEM_MAX = 5
BLOCK_LIB_COMMAND_ITEM_MAX = 5
BLOCK_LIB_RESPOND_ITEM_BYTES_NUM = 6 # respond_id(1) + datas_type(5)
BLOCK_LIB_COMMAND_ITEM_BYTES_NUM = 6

BLOCK_LIB_RES_CMD_ITEM_OVER_FLAG_VALUE = 0x00
BLOCK_LIB_RES_CMD_OVER_FLAG_VALUE = 0xff
'''
# common func define
def block_lib_info_list_fill(original_data, len_d, val):
    s_len = len(original_data)
    index = s_len
    while (index < len_d):
        original_data.append(val)
        index = index + 1
    return original_data

class neurons_engine_block_lib_file_generate(object):
    def __init__(self):
        self.now_write_offset = 0
        self.head_add_flag = False      
        self.total_bytes = 0
        
    def add_single_block_lib(self, lib_info):
        f = open(FLASH_LIB_FILE_NAME, "ab")
        f.seek(now_write_offset)
        for item in  lib:
            if type(item) == list:
                for item_tuple in item:
                    for da in item_tuple:
                        f.write(pack('B', da))
                for i in range(6 * (5 - len(item))):
                    f.write(pack('B', BLOCK_LIB_RES_CMD_OVER_FLAG_VALUE))
        
            elif type(item) == int:
                f.write(pack('B', item))
'''






class lib_information(object):
    def __init__(self):
        self.now_write_offset = 128

    def write_single_lib_info(self, lib_info):
        f = open(FLASH_LIB_FILE_NAME, "ab")
        write_single_lib(f, self.now_write_offset, lib_info)    
        f.close()
        self.now_write_offset += LIB_SINGLE_LIB_NUM

def bytes_fill(data, len_d):
    s_len = len(data)
    index = s_len
    while (index < len_d):
        data.append(0)
        index = index + 1
    return data

def create_head_frame():
    write_num = 0
    f = open(FLASH_LIB_FILE_NAME, "wb")
    # write version data
    lib_version_l = bytes_fill(list(LIB_VERSION), lib_head_dict["version"])
    for da in lib_version_l:
        if type(da) == str:
            f.write(pack('B', ord(da)))
            write_num += 1
        elif type(da) == int:
            f.write(pack('B', da))
            write_num += 1
    # append single_byte_num
    lib_single_lib_num_l = LIB_SINGLE_LIB_NUM
    f = open(FLASH_LIB_FILE_NAME, "ab")
    f.write(pack('L', lib_single_lib_num_l))
    write_num += 4

    # append single_byte_num
    lib_type_num = LIB_TYPE_NUM
    f.write(pack('L', lib_type_num))
    write_num += 4

    # write info
    lib_info_l = bytes_fill(lib_info, lib_head_dict["info"])
    for type_struct in lib_info_l:
        if type(type_struct) == int:
            break

        f.write(pack('B', type_struct[0]))
        write_num += 1
        f.write(pack('B', type_struct[1]))
        write_num += 1
        f.write(pack('H', type_struct[2]))
        write_num += 2

    # no data fill 0 
    for i in range(LIB_HEAD_LEN - write_num):
        f.write(pack('B', 0xff))
    f.close()


def write_single_lib(f, offset, lib):
    f.seek(offset)
    for item in  lib:
        if type(item) == list:
            for item_tuple in item:
                for da in item_tuple:
                    f.write(pack('B', da))
            for i in range(6 * (5 - len(item))):
                f.write(pack('B', 0xff))
        
        elif type(item) == int:
            f.write(pack('B', item))



    

def main(): 
    lib_ope = lib_information()
    create_head_frame()

    lib_ope.write_single_lib_info(NEU_SENSORS_TEM_LIB)
    lib_ope.write_single_lib_info(NEU_SENSORS_LIGHT_LIB)
    lib_ope.write_single_lib_info(NEU_SENSOR_ULTAR_LIB)
    lib_ope.write_single_lib_info(NEU_SENSOR_LINEFOLLOW_LIB)
    lib_ope.write_single_lib_info(NEU_SENSOR_COLOR_LIB)

    lib_ope.write_single_lib_info(NEU_DISPLAY_NUMERIC_LIB)
    lib_ope.write_single_lib_info(NEU_DISPLAY_S_RGB_LED_LIB)
    lib_ope.write_single_lib_info(NEU_DISPLAY_M_RGB_LED_LIB)
    lib_ope.write_single_lib_info(NEU_DISPLAY_RGB_MATRIX_LIB)

    f = open(FLASH_LIB_FILE_NAME, "rb")
    print(f.read())
    f.close()

if __name__ == "__main__":
    main()
