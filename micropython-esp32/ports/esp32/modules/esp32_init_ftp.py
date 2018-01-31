from time import sleep
from codey_wlan_board import codey_wlan

e_wifi_enable = True

# for wifi config
if e_wifi_enable == True:
    e_wi = codey_wlan()
    e_wi.wifi_enable()
    e_wi.wifi_add_mac(True)
    e_wi.wifi_ap_config("makeblock_fftust","12345678")
    e_wi.wifi_mode_config(e_wi.STA_AP)
    e_wi.wifi_start()
    del e_wi

import gc
gc.collect()
