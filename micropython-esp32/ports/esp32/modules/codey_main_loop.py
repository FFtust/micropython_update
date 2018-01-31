import time
from codey_event_board import event_sema_list
from codey_event_board import m_eve
from codey_event_board import codey_sys_start
from codey_global_board import *

SEM_WAIT_FOREVER = 40000
SEM_NOT_WAIT = 0
THREAD_SCHEDULE_INTERVAL = 0.025 

def main_loop():
    global event_sema_list
    # start the system thread here, call only once
    codey_sys_start()
    m_eve.start_trigger()
    m_eve.event_trigger(CODEY_LAUNCH)
    while True:
        if m_eve.get_event_number() != 0:    
            for i in range(m_eve.get_event_number()):
                if m_eve.get_occured_flag(i) == True:
				    # when error in user callback, the memory about this function will be free,
					# when error occured, codey system will catch the error, and set event_sema_list[i] to None
                    if event_sema_list[i] != None:
                        if event_sema_list[i].locked():
                            event_sema_list[i].release()
                        else:
                            print_dbg("sema is not locked", i)   
        time.sleep(THREAD_SCHEDULE_INTERVAL)

def main_loop_hook():
    pass 

if __name__ == '__main__':
    try:
        main_loop()
    except Exception as e:
        print('main_loop error occured:' + str(e))
            






            

            
    
