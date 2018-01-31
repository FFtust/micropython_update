#we can call the functions like this
# w=makeblock_wlan()
# w.wifi_enable()
# w.wifi_add_mac(True)
# w.wifi_sta("Maker-office","hulurobot423")
# w.wifi_ap_config("oooy","12345678")
# w.wifi_mode(3)
# w.wifi_start()
#to set theESP32 wifi at ap+sta mode,it will connect company's router,
#and set itself a ap,named "oooy" + mac address

from makeblock import wlan
import usocket
from codey_global_board import *
class codey_wlan(wlan):
    # mode:0 for null, 1 for sta, 2 for ap,3 for apsta
    # you can also use const variable STA 、AP、STA_AP
    def __init__(self, mode = 0):
        if mode > 3:
            mode = 3
        self.w_mode = mode
        self.w_en = False
        self.w = wlan();

        
    def wifi_enable(self):
        if self.w.enable() == True:
            self.w_en = True
            return True
        else:
            return False

    def wifi_disenable(self):
        if self.w.deinit() == True:
            self.w_en = False
            return True
        else:
            return False

    def wifi_add_mac(self, en):
        if self.w_en == True:
            self.w.apssid_add_mac(en)
        else:
            print_dbg("wifi is not enabled")
          
    def wifi_mode_config(self, mode):
        if self.w_en == True:      
            if mode > 3:
                mode = 3
            if self.w.set_mode(mode) == True:
                self.w_mode = mode
                return True
            else:
                return False
        else:
            print_dbg("wifi is not enabled")
        
    @ property
    def wifi_mode(self):
        return self.w_mode

    
    def wifi_ap_config(self, ssid, password):
        if self.w_en == True:
            self.w.set_ap(ssid, password)
        else:
            print_dbg("wifi is not enabled")
        
    @property
    def wifi_ap(self):
        pass
    
    def wifi_sta_config(self, ssid, password):
        if self.w_en == True:
            self.w.set_sta(ssid, password)
        else:
            print_dbg("wifi is not enabled")

    @property
    def wifi_sta(self):
        pass


    def wifi_sta_auto_connect(self, en):
        if self.w_en == True:
            self.w.set_auto_connect(True)
        else:
            print_dbg("wifi is not enabled")
  
    def wifi_start(self):
        if self.w_en == True:
            self.w.start(self.w_mode)
        else:
            print_dbg("wifi is not enabled")
        
    def wifi_stop(self):
        if self.w_en == True:
           self.w.stop()
        else:
            print_dbg("wifi is not enabled")

    def wifi_connect(self):
        if self.w_en == True:
            if self.w_mode == 1 or self.w_mode == 3:
                self.w.connect()
            else:
                print_dbg("only sta or apsta mode have this func")
        else:
            print_dbg("wifi is not enabled")
        
    def wifi_disconnect(self):
        if self.w_en == True:
            if self.w_mode == 1 or self.w_mode == 3:
                self.w.disconnect()
            else:
                print_dbg("only sta or apsta mode have this func")
        else:
            print_dbg("wifi is not enabled")
        
    def wifi_scan(self):
        if self.w_en == True:
            if self.w_mode == 1 or self.w_mode == 3:
                self.w.scan()
            else:
                print_dbg("only sta or apsta mode have this func")
        else:
            print_dbg("wifi is not enabled")
        
    #ifx shoule be set to :0 or 1    
    def wifi_get_mac(self, ifx):
        if self.w_en == True:
            if ifx > 1:
                ifx = 1
            elif ifx < 0:
                ifx = 0;
            
            return self.w.get_mac(ifx)
        else:
            print_dbg("wifi is not enabled")

    def wifi_sta_is_conn(self):
        return self.w.sta_is_conn()
