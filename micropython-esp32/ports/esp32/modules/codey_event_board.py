import _thread
import time
from makeblock import codey_eve
from codey_global_board import *

event_sema_list = [0] * THREAD_MAX_NUM
events_info = []
events_stack_size = 1024 * 4 + 512
m_eve = codey_eve()

def event_set_sema(event_id):
    event_sema_list[event_id] = _thread.allocate_lock()
    event_sema_list[event_id].acquire(0)

class codey_event(object):
    def __init__(self, event_index, cb = None, para = 0):
        global event_sema_list
        self.eve_type = event_index
        self.cb = cb
        self.parameter = para
        self.eve_id = -1
        self.eve_id = m_eve.register(self.eve_type, self.parameter)
        if self.eve_id >= 0:
            event_set_sema(self.eve_id)
        else:
            print_dbg("event register failed")

    def event_cb_task(self):
        try:
            while True:
                if self.eve_id != -1:
                    if event_sema_list[self.eve_id].acquire(1) == True:
                        # Call user callback function
                        if self.cb != None:
                            self.cb()
                        m_eve.set_occured_flag(self.eve_id, False)
                        event_sema_list[self.eve_id].acquire(0)
                    else:
                        continue
                else:
                    print_dbg("eve_id is null")
                    time.sleep(1000)
        # if error occured in the callback, the sema RAM will be freed, 
        # but other function will still use this sema, then a fatal system error happend
        # get the exception and make this task never out is a temporary solution
        except KeyError:
            # when error occured, set the item in event_sema_list to None, 
            # idicating that this callback had been destroyed
            event_sema_list[self.eve_id] = None
            print(str(e))
            print("key error on user callback occured")
        finally:
            event_sema_list[self.eve_id] = None
            print("event:", self.eve_id, "error occured:")
            print("free the memory of this callback")
   
    def event_execute_cb(self):
        global events_stack_size
        _thread.stack_size(events_stack_size)
        _thread.start_new_thread(self.event_cb_task, ())
  
    def event_listening_start(self):
        self.event_execute_cb()

def startEvent(event_type, user_cb, user_para = 0):
    # for button enents
    event = codey_event(event_type, user_cb, user_para)
    event.event_listening_start() 

def event_register_add(event_type, user_cb, user_para = 0):
    global events_info
    events_info.append([event_type, user_cb, user_para])

def codey_sys_start():
    global events_info
    global events_stack_size
    events_num = len(events_info)
    # set thread stack size here, not the default stack size
    if events_num != 0 and (not THREAD_STACK_SIZE_IS_DEFAULT):
        events_stack_size = THREAD_TOTAL_STATCK_SIZE // events_num
    for item in events_info:
        startEvent(item[0], item[1], item[2])
        time.sleep(0.05)




            

            
    
