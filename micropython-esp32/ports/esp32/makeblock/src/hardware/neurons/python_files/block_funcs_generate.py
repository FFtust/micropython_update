cur_block_list = [[1, 61 , 1, 1], [2, 61, 2, 1]]
#[id, type, sub_type, index]
class neurons_funs_generate(object):
    def __init__(self, index, block_type, block_sub_type = None, cmd_data = []):
        self.index = index
        self.block_type = block_type
        self.block_sub_type = block_sub_type
        self.cmd_data = cmd_data
        self.cmd_send_buffer = []
        self.device = 0

    def get_block_id(self):
        for i in range(len(cur_block_list)):
            if self.block_sub_type != None:
                if cur_block_list[i][1 : 4] == [self.block_type,self.block_sub_type, self.index]:
                    return cur_block_list[i][0]
                else:
                    return None
            else:
                if cur_block_list[i][1:3] == [self.block_type,self.block_sub_type, self.index]:
                    return cur_block_list[i][0]
                else:
                    return None

    def send_command_to_buffer(self, *cmd_data):
        self.device_id = self.get_block_id()
        if self.device_id != None:
            self.cmd_send_buffer.append(self.device_id)
            self.cmd_send_buffer.append(self.block_type)
            self.cmd_send_buffer.append(self.block_sub_type)
            for val in cmd_data:
                self.cmd_send_buffer.append(val)
        print(self.cmd_send_buffer)



test_fun = neurons_funs_generate(1, 61, 1).send_command_to_buffer
