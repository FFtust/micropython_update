import codey
import rocky
import time

def car_forward_test():
	print( "Forward using forward" )
	for speed in list( range( 0, 101, 20 ) ):
		print( "Forward speed: %d"% speed )
		rocky.forward( speed, 1 )

	for speed in list( range( 0, 101, 20 ) ):
		print( "Forward speed: %d"% -speed )
		rocky.forward( -speed, 1 )

def car_backward_test():
	print( "back using back" )
	for speed in list( range( 0, 101, 20 ) ):
		print( "back speed: %d"% speed )
		rocky.back( speed, 1 )

	for speed in list( range( 0, 101, 20 ) ):
		print( "back speed: %d"% -speed )
		rocky.back( -speed, 1 )

def car_left_turn_test():
	print( "left using forward" )
	for speed in list( range( 0, 101, 20 ) ):
		print( "left speed: %d"% speed )
		rocky.left( speed, 1 )

	for speed in list( range( 0, 101, 20 ) ):
		print( "left speed: %d"% -speed )
		rocky.left( -speed, 1 )

def car_right_turn_test():
	print( "right using forward" )
	for speed in list( range( 0, 101, 20 ) ):
		print( "right speed: %d"% speed )
		rocky.right( speed, 1 )

	for speed in list( range( 0, 101, 20 ) ):
		print( "right speed: %d"% -speed )
		rocky.right( -speed, 1 )

def car_drive_test():
	print( "right left + power" )
	for speed in list( range( 0, 101, 20 ) ):
		print( "right speed: %d"% speed )
		rocky.drive( speed, speed )
		time.sleep(1)
	print( "right left - power" )
	for speed in list( range( 0, 101, 20 ) ):
		print( "right speed: %d"% speed )
		rocky.drive( -speed, -speed )
		time.sleep(1)
	rocky.stop()

def car_drive_continue_test():
	print( "car drive continue" )
	for speed in list( range( 0, 101 ) ):
		print( "drive speed: %d"% speed )
		rocky.drive( speed, speed )
		time.sleep(0.1)
	for speed in list( range( 0, 101 ) ):
		print( "drive speed: %d"% -speed )
		rocky.drive( -speed, -speed )
		time.sleep(0.1)
	rocky.stop()

def color_rgb_read_test():
	print( "rgb read test" )
	for i in range( 10 ):
		print( "R: %d"%rocky.red() )
		time.sleep( 1 )
		print( "G: %d"%rocky.green() )
		time.sleep( 1 )
		print( "B: %d"%rocky.blue() )
		time.sleep( 1 )

def color_env_gray_test():
	print( "env light test" )
	for i in range( 10 ):
		print( "env left gray: %d"%rocky.light_intensity( "left" ) )
		time.sleep( 1 )
		print( "env right gray: %d"%rocky.light_intensity( "right" ) )
		time.sleep( 1 )

def color_is_obstacle_ahead_test():
	print( "is_obstacle_ahead test" )
	for i in range( 10 ):
		print( "is_obstacle_ahead %d"%rocky.is_obstacle_ahead() )
		time.sleep( 1 )

def color_id_test():
	print( "color id test" )
	while( True ):
		print( "color is " + rocky.color() )
		time.sleep( 1 )

car_forward_test()
car_backward_test()
car_left_turn_test()
car_right_turn_test()
color_env_gray_test()
color_is_obstacle_ahead_test()
color_id_test()
# color_id_test()
# color_is_obstacle_ahead_test()
# color_env_gray_test()
# color_rgb_read_test()
# car_drive_continue_test()
# car_drive_test()
# car_backward_test()
# car_left_turn_test()
# car_right_turn_test()

while( True ):
	print("1")
	time.sleep( 1 )


