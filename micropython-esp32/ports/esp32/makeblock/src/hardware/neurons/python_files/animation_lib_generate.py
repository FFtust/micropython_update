## ***************************************************************************************************************************************
FLASH_LIB_FILE_NAME = "neurons_engine_lib.bin"
## common function
def block_lib_info_list_fill(original_data, len_d, val):
    s_len = len(original_data)
    index = s_len
    while (index < len_d):
        original_data.append(val)
        index = index + 1
    return original_data


## about head 
class file_head_ope(object):
    def __init__(self):
        self.blocks_index = []
        self.special_blocks_index = []
        self.type_begin = NEURONS_LIB[0]["TYPE_ID"]
        self.blocks_index_offset = HEAD_LENGTH

    def create_blocks_index(self):
        for item in NEURONS_LIB:
            self.blocks_index.append([item["TYPE_ID"], item["BLOCK_NUM"], self.blocks_index_offset])
            self.blocks_index_offset += item["BLOCK_NUM"] * HEAD_SINGLE_LIB_BYTES_NUM

        for item in NEURONS_SPECIAL_LIB:
            self.special_blocks_index.append([item[1]["type"], item[1]["sub_type"], 
                                             item[0]["respond_num"], item[0]["command_num"], self.blocks_index_offset])
            self.blocks_index_offset += (item[0]["respond_num"] + item[0]["command_num"]) * 30 + 2  

        print("blocks_index is:",self.blocks_index)
        print("special_blocks_index is:",self.special_blocks_index)
        return self.blocks_index, self.special_blocks_index

    def file_write_head(self):
        write_num = 0
        f = open(FLASH_LIB_FILE_NAME, "wb")
        # write version data 32bytes
        lib_version_l = block_lib_info_list_fill(list(HEAD_VERSION_RECORD[-1]), HEAD_VERSION_STR_LENGTH, 0x00)
        for da in lib_version_l:
            if type(da) == str:
                f.write(pack('B', ord(da)))
                write_num += 1
            elif type(da) == int:
                f.write(pack('B', da))
                write_num += 1

        # append single_byte_num 4bytes
        lib_single_lib_num_l = HEAD_SINGLE_LIB_BYTES_NUM
        f = open(FLASH_LIB_FILE_NAME, "ab")
        f.write(pack('L', lib_single_lib_num_l))
        write_num += 4

        # append type_num 4bytes
        lib_type_num = HEAD_TYPE_NUM_RECORD[-1]
        f.write(pack('L', lib_type_num))
        write_num += 4

        # append type_num 4bytes
        special_block_num = HEAD_SPECIAL_BLOCK_NUM_RECORD[-1]
        f.write(pack('L', special_block_num))
        write_num += 4

        lib_info_l , special_lib_info= self.create_blocks_index()
        # append special block index
        for type_struct in special_lib_info:
            if type(type_struct) == int:
                break
            f.write(pack('B', type_struct[0]))
            write_num += 1
            f.write(pack('B', type_struct[1]))
            write_num += 1
            f.write(pack('B', type_struct[2]))
            write_num += 1
            f.write(pack('B', type_struct[3]))
            write_num += 1
            f.write(pack('H', type_struct[4]))
            write_num += 2

        for i in range((SPECIAL_BLOCKS_NUM_MAX - special_block_num) * 6):
            f.write(pack('B', BLOCK_LIB_RES_CMD_OVER_FLAG_VALUE))
        write_num += (SPECIAL_BLOCKS_NUM_MAX - special_block_num) * 6

        # write info max 160 - 32 -4 - 4 - 4 - 48 = 68, max item 68 // 4 = 17
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
        print("write num is", write_num)
        for i in range(HEAD_LENGTH - write_num):
            pass 
            f.write(pack('B', BLOCK_LIB_RES_CMD_OVER_FLAG_VALUE))
        f.close()


## about body
class blocks_lib(file_head_ope):
    def __init__(self):
        file_head_ope.__init__(self)
        '''
        self.type_index = []
        for item in self.create_blocks_index():
            self.type_index.append(item[-1])
        print("type_index list is:", self.type_index)
        '''
    def get_block_write_offset(self, block_info):
        block_type = block_info["type"]
        block_sub_type = block_info["sub_type"]

        offset = self.blocks_index[block_type - self.type_begin][2] + \
                 (block_sub_type - 0x01) * 62
        print("general block offset is", offset)

        return offset

    def get_special_block_write_offset(self, block_info):
        for item in self.special_blocks_index:
            if item[0] == block_info["type"] and item[1] == block_info["sub_type"]:
                offset = item[4]
                print("special offset is", offset)
                return offset

    def write_single_block_info(self, f, block_info, offset):
        f.seek(offset)
        print("write index is ", offset)
        f.write(pack('B', block_info["type"]))
        f.write(pack('B', block_info["sub_type"]))

        if(block_info["respond_info"] != ()):
            for item in block_info["respond_info"]:
                for da in item:
                    f.write(pack('B', da))
        for i in range(6 * (5 - len(block_info["respond_info"]))):
            f.write(pack('B', 0xff))

        if(block_info["command_info"] != ()):
            for item in block_info["command_info"]:
                for da in item:
                    f.write(pack('B', da))
        for i in range(6 * (5 - len(block_info["command_info"]))):
            f.write(pack('B', 0xff))

    def write_all_block_info(self):
        f = open(FLASH_LIB_FILE_NAME, "ab") 
        for item_type in NEURONS_LIB:
            block_type = item_type["TYPE_ID"]
            for item_sub_type in item_type["BLOCK_INFO"]:
                block_sub_type = item_sub_type["sub_type"]
                off = self.get_block_write_offset(item_sub_type)
                self.write_single_block_info(f, item_sub_type, off)
        f.close()

    def write_special_block_info(self):
        f = open(FLASH_LIB_FILE_NAME, "ab") 
        for item in NEURONS_SPECIAL_LIB:
            block = item[1]
            off = self.get_special_block_write_offset(block)
            self.write_single_block_info(f, block, off)
        f.close()        

   
## ***************************************************************************************************************************************
### test
from struct import *
from block_lib_data import *
file_write = blocks_lib()

file_write.file_write_head()
file_write.write_all_block_info()
file_write.write_special_block_info()
