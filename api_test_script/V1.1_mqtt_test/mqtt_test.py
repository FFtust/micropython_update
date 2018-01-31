import codey
import rocky
import time
import _thread
from umqtt.simple import MQTTClient

# Read me ###########################################################
# 1 ) Call mqtt_pub_test() to public a message to server "test.mosquitto.org" on topic "makeblock/lxy/sensor", message is " This a message from make block (id) "
# 2 ) Call mqtt_sub_test() to suscribe a topic "makeblock/lxy/sensor" on servie "test.mosquitto.org"
# 3 ) can NOT pubic & subscri at the same time

# start public
# mqtt_pub_test()

# subscipt
# mqtt_sub_test()

server = "test.mosquitto.org"
topic = "makeblock/lxy/sensor"
base_msg = "This a messge from make block"

def conn_wifi():
	codey.wifi( "Maker-guest", "makeblock" )
	while( True ):
		if ( codey.wifi_is_connected() ):
			break;
		else:
			print( "Wait for wifi connected" )
			time.sleep( 1 )

def mqtt_sub_cb( topic, msg ):
	print( "Got a msg " + "T: " +  str(topic) + "  msg: " + str(msg)  )

def mqtt_pub( server, topic, msg ):
    c = MQTTClient( "umqtt_client", server )
    c.connect()
    c.publish( topic, msg )
    c.disconnect()

def mqtt_sub( server, topic, callback ):
    c = MQTTClient( "umqtt_client", server )
    c.set_callback( callback )
    c.connect()
    c.subscribe( topic )
    try:
        while True:
            c.check_msg()
    finally:
        c.disconnect()

def mqtt_pub_test():
	global server
	global topic 
	global base_msg
	conn_wifi()
	msg_id = 1
	while( True ):
		msg = base_msg + " " + str(msg_id)
		mqtt_pub( server, topic, msg )
		print( "public: " + msg )
		msg_id += 1
		time.sleep( 1 )

def mqtt_sub_test():
	global server
	global topic 
	conn_wifi()
	mqtt_sub( server, topic, mqtt_sub_cb )

while( True ):
	mqtt_sub_test()
	# mqtt_pub_test()
	pass