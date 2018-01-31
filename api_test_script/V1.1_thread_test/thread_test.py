import time
import _thread
import codey
def fun_timer():
	while( True ):
		codey.rgb(200,0,0)
		print('Hello Timer in a threading!')
		time.sleep( 1 )

_thread.start_new_thread( fun_timer, () )

while True:
	print( "In main tick" )
	time.sleep( 1 )