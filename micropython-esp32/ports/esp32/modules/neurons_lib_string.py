from struct import * 
import re
from codey_global_board import *

lib_str = "\
636f 6465 795f 6e65 7500 0000 0000 0000 \
3030 3200 0000 0000 0000 0000 0000 0000 \
3e00 0000 0600 0000 0200 0000 6310 1414 \
9e08 6601 0009 9009 ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff 6204 a000 \
630d 9801 6408 be04 6506 ae06 6602 2208 \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
6201 ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
0102 0000 0000 ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff 6202 \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff 0102 \
0200 0000 ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff 6203 ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff 0103 0000 \
0000 ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff 6204 0504 0500 \
0000 ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff 0104 0500 0000 \
0204 0500 0000 0304 0000 0000 0402 0000 \
0000 0500 0000 0000 6301 0106 0000 0000 \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff 0100 0000 0000 ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff 6302 0101 0000 0000 ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff 0100 0000 0000 ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff 6303 0106 0000 0000 ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff 0100 0000 0000 ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff 6304 0101 0000 0000 ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff 0100 0000 0000 ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
6305 0103 0303 0000 ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
0100 0000 0000 ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff 6306 \
0101 0000 0000 ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff 0101 \
0105 0000 ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff 6307 0102 \
0100 0000 ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff 0100 0000 \
0000 ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff 6308 0101 0000 \
0000 ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff 0100 0000 0000 \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff 6309 0101 0000 0000 \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff 0100 0000 0000 ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff 630a 0101 0000 0000 ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff 0100 0000 0000 ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff 630b 0103 0000 0000 0203 0000 \
0000 0303 0000 0000 0403 0000 0000 ffff \
ffff ffff 0100 0000 0000 0200 0000 0000 \
0300 0000 0000 0401 0000 0000 ffff ffff \
ffff 630c 0101 0000 0000 ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff 0100 0000 0000 ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
630d 0101 0000 0000 ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
0100 0000 0000 ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff 6401"

lib_str_ext = "\
0101 0000 0000 ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff 0100 \
0000 0000 ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff 6402 0101 \
0000 0000 ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff 0100 0000 \
0000 ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff 6403 ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff 0101 0000 0000 \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff 6404 0101 0000 0000 \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff 6405 0101 0000 0000 ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff 0100 0000 0000 ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff 6406 0101 0000 0000 ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff 0100 0000 0000 ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff 6407 0102 0200 0000 ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff 0100 0000 0000 ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
6408 ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
0100 0000 0000 ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff 6501 \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff 0106 \
0000 0000 ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff 6502 ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff 0103 0303 \
0000 ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff 6503 ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff 0101 0000 0000 \
0231 f100 0000 ffff ffff ffff ffff ffff \
ffff ffff ffff ffff 6504 ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff 0125 3300 0000 0201 \
3300 0000 0321 f100 0000 0421 f100 0000 \
0521 0000 0000 6505 ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff 0121 03f1 0000 0221 0000 \
0000 0321 03f1 0000 ffff ffff ffff ffff \
ffff ffff 6506 ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff ffff 0101 0000 0000 ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff 6601 ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
ffff 0101 0000 0000 0201 0000 0000 0300 \
0000 0000 0400 0000 0000 0500 0000 0000 \
6602 ffff ffff ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff ffff \
0104 0100 0000 ffff ffff ffff ffff ffff \
ffff ffff ffff ffff ffff ffff ffff 6310 \
0133 0000 0000 0201 0000 0000 0301 0000 \
0000 0401 0000 0000 0501 0000 0000 0601 \
0000 0000 0701 0000 0000 0801 0000 0000 \
0900 0000 0000 0a23 0000 0000 0b00 0000 \
0000 0c00 0000 0000 0d00 0000 0000 0e00 \
0000 0000 0f00 0000 0000 1000 0000 0000 \
1100 0000 0000 1200 0000 0000 1300 0000 \
0000 1400 0000 0000 0100 0000 0000 0200 \
0000 0000 0300 0000 0000 0400 0000 0000 \
0500 0000 0000 0600 0000 0000 0721 0000 \
0000 0800 0000 0000 0931 0000 0000 0a00 \
0000 0000 0b00 0000 0000 0c00 0000 0000 \
0d01 0000 0000 0e01 0000 0000 0f01 0000 \
0000 1001 0000 0000 1141 0000 0000 1201 \
0000 0000 1301 0000 0000 1431 0000 0000 \
6601 0101 0000 0000 0201 0000 0000 0300 \
0000 0000 0400 0000 0000 0500 0000 0000 \
0600 0000 0000 0700 0000 0000 0801 0000 \
0000 0901 0000 0000" \

# every line contains 40 bytes, version info in second line
neurons_static_version_str = lib_str[40 : 80 - 1]
neurons_lib_head_size = 32
neurons_lib_head_magic_len = 16
neurons_lib_version_len = 16
def write_neurons_lib():
    global lib_str
    global lib_str_ext
    with open("neurons_engine_lib.bin", "w") as f:
        str_data = lib_str.split(' ')
        del lib_str
        for item in str_data:
            f.write(pack('B', int(item[0 : 2], 16)))
            f.write(pack('B', int(item[2 : 4], 16)))
        del str_data
        
        str_data = lib_str_ext.split(' ')
        del lib_str_ext
        for item in str_data:
            f.write(pack('B', int(item[0 : 2], 16)))
            f.write(pack('B', int(item[2 : 4], 16)))
        del str_data


def get_current_neurons_lib_version():
    with open("neurons_engine_lib.bin", "r") as f:
        ver = f.read(neurons_lib_head_size)
        # print("neurons now version is: ", ver[16 : 32]) # the name of version just like "001"
        try:
            ver_value = re.search(r"\d+\.?\d*", ver).group(0)
            print_dbg("current version is", ver_value)
            return ver_value
        except:
            print("current version string is not match")
            return None
    	
def get_static_neurons_lib_version():
    # we insure that the static version format is valid
    ver_str = neurons_static_version_str.split(' ')
    print_dbg("ver_value is", ver_str)
    ver_value = ''
    for item in ver_str:
        ver_value += chr(int(item[0 : 2], 16))
        ver_value += chr(int(item[2 : 4], 16)) 
    ver_value = re.search(r"\d+\.?\d*", ver_value).group(0)
    print_dbg("static version is", ver_value)
    return ver_value

def lib_str_del():
	global lib_str
	del lib_str