import codey
import rocky
import time

icr_pid = { "set_pos":0, "act_pos":0, "err":0, "err_last":0, "err_last_last":0, "Kp":0, "Ki":0, "Kd":0, "a0":0, "a1":0, "a2":0 }

def icr_pid_init( icr_pid_s, Kp, Ki, Kd, max_icr ):
	icr_pid_s["Kp"] = Kp
	icr_pid_s["Ki"] = Ki
	icr_pid_s["Kd"] = Kd
	icr_pid_s["err"] = 0
	icr_pid_s["err_last"] = 0
	icr_pid_s["err_last_last"] = 0
	icr_pid_s["max_irc"] = max_icr

def icr_pid_print_param( icr_pid_s ):
	print( "Kp " + icr_pid_s["Kp"] )
	print( "Ki " + icr_pid_s["Ki"] )
	print( "Kd " + icr_pid_s["Kd"] )
	print( "err " + icr_pid_s["err"] )
	print( "err_last " + icr_pid_s["err_last"] )
	print( "err_last_last " + icr_pid_s["err_last_last"] )
	print( "max_irc " + icr_pid_s["max_irc"] )

def icr_pid_loop( icr_pid_s, err ):
	icr_pid_s["err"] = err
	p = icr_pid_s["Kp"] * ( icr_pid_s["err"] - icr_pid_s["err_last"] )
	i = icr_pid_s["Ki"] * icr_pid_s["err"]
	d = icr_pid_s["Kd"] * ( icr_pid_s["err"] - 2*icr_pid_s["err_last"] + icr_pid_s["err_last_last"] )
	icr_pid_s["err_last_last"] = icr_pid_s["err_last"]
	icr_pid_s["err_last"] = icr_pid_s["err"]
	icr = (p + i + d)
	# print( "err: %f"%(icr_pid_s["err"]) )
	# print( "cir: %f, p: %f, i: %f, d: %f"%(icr, p, i, d) )
	
	if ( icr > icr_pid_s["max_irc"] ):
		icr = icr_pid_s["max_irc"]
	elif ( icr < (-icr_pid_s["max_irc"]) ):
		icr = -icr_pid_s["max_irc"]
	return ( icr )

def trace_line_calb():
	while( True ):
		err = rocky.grey("left") - rocky.grey("right")
		# print( "err%d"%err )
		rocky.left( 100 )
		rocky.right( 100 )
		time.sleep( 0.7 )	

def trace_line():
	global icr_pid
	record_cnt = 10
	gray_1 = 0
	gray_2 = 0
	last_gray_1 = 0
	last_gray_2 = 0
	gray_diff_err_squ = []
	gray_diff_err = []
	for i in range(record_cnt):
		gray_diff_err_squ.insert( i + 1, 0 )
		gray_diff_err.insert( i + 1, 0 )
	record_idx = 0
	max_icr = 80
	trace_speed = 80
	icr_pid_init( icr_pid, 1.3, 0.1, 0.0, 100 )
	icr = 0
	on_white_cnt = 0
	while ( True ):
		err = rocky.grey("left") - rocky.grey("right")
		gray_1 = rocky.grey("left")
		gray_2 = rocky.grey("right")
		err = gray_1 - gray_2

		gray_diff_err_squ[record_idx] = abs(gray_1 - last_gray_1) * abs(gray_1 - last_gray_1) - abs(gray_2 - last_gray_2) * abs(gray_2 - last_gray_2)
		gray_diff_err[record_idx] = err
		record_idx += 1
		record_idx = record_idx%record_cnt
		last_gray_1 = gray_1
		last_gray_2 = gray_2

		# print( "err %d"%err )
		out = icr_pid_loop( icr_pid,  err )
		if ( icr >= max_icr ):
			if ( out < 0 ):
				icr += out
		elif ( icr <= -max_icr ):
			if ( out > 0 ):
				icr += out
		else:
			icr += out

		left_speed = trace_speed + icr
		right_speed = trace_speed - icr

		# print( "L: %d,     R: %d"%(left_speed, right_speed) )
		rocky.drive( left_speed, right_speed )
		time.sleep( 0.01 )

# left +, right -
# trace_line_calb()
trace_line()
# rocky.stop()