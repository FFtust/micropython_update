
#common functions
def mb_safe(func):
    def _wrapper(*args, **kwargs):
        try:
            return func(*args, **kwargs)
        except Exception as e:
            print('===== mb_safe: ' + str(e))
            return ''
    return _wrapper

#create iot module
import uos
uos.chdir('/flash/flash')
with open("iot.py", "w") as f:
    f.write("")

import ujson
import iot
import time
import codey
import urequests
from codey_wlan_board import codey_wlan

# 云列表请求 域名
iot_list_request_domain = 'http://test.iothub.makeblock.com/'
iot_weather_request_domin = 'http://test.mweather.makeblock.com/';

def __iot_get_request_header():
    return {
        'content-type': 'application/json; charset=utf-8',
        'uid': '90',
        'devicetype': '1',
        'deviceid': '30AEA4277CF8'
    }

def __iot_get(request_url):
    if not codey.wifi_is_connected():
        print('-------> No network connection: ' + request_url)
        return ''
    print('======request_url: ' + request_url)
    res = urequests.request('GET', request_url, headers = __iot_get_request_header()).json()
    print(res)
    return res['data']

def __iot_post(request_url, post_data):
    if not codey.wifi_is_connected():
        print('-------> No network connection: ' + request_url)
        return ''
    print('======request_url: ' + request_url)
    res = urequests.request('POST', request_url, headers = __iot_get_request_header(), data = post_data).json()
    print(res)
    return res['data']

# 添加数据项
@mb_safe
def iot_list_add(name, data):
    post_data = ujson.dumps({ "listName": name, "data": data})
    return __iot_post(iot_list_request_domain + 'meos/postcloudlist', post_data)

iot.list_add = iot_list_add

# 得到 index 指向数据项
@mb_safe
def iot_list_index(name, index):
    req_type = 'index'
    req_index = 0
    if index == 'random':
        req_type = 'random'
    elif index == 'last':
        req_type = 'last'
    else:
        index = int(index)
        if index > 0:
            req_index = index - 1
        else:
            return ''
    res = __iot_get(iot_list_request_domain + 'meos/getCloudListItemByIndex?listName=' + name + '&type=' + req_type + '&index=' + str(req_index))
    print('iot_list_index ----> ' + str(res['itemData']['data']))
    return res['itemData']['data']

iot.list_index = iot_list_index

# 获取云列表长度
@mb_safe
def iot_list_length(name):
    res = __iot_get(iot_list_request_domain + 'meos/getcloudlistlen?listName=' + name)
    print('iot_list_length ----> ' + str(res['listLen']))
    return int(res['listLen'])

iot.list_length = iot_list_length

# 获取天气信息想
#   city_code: 城市编码
#   data_type: 获取数据类型
@mb_safe
def iot_weather(city_code, data_type):
    if not codey.wifi_is_connected():
        return ''
    res = urequests.request('GET', iot_weather_request_domin + 'getweather?woeid=' + str(city_code) + '&type=' + str(data_type))
    text = res.text
    print('=====iot_weather: ' + text)
    if int(data_type) <= 3:
        return int(text)
    return text
iot.weather = iot_weather
import codey
import iot

def on_message_callback():
    codey.face('00001020402012020212204020100000', 1)
    while True:
        codey.show(iot.list_length('r'))


codey.on_message(str('gg'), on_message_callback)

def on_button_callback():
    codey.say('cat.wav')

codey.on_button('A', on_button_callback)

def on_message1_callback():
    codey.color('#ff0000', 1)
    while True:
        iot.list_add('r', iot.weather('2161853', '0'))


codey.on_message(str('ww'), on_message1_callback)


codey.wifi('Maker-guest', 'makeblock')
codey.show('connecting...')
while not codey.wifi_is_connected():
    pass

codey.message(str('gg'))
codey.message(str('ww'))


