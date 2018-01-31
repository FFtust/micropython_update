import time
import random

icr_pid = { "set_pos":0, "act_pos":0, "err":0, "err_last":0, "err_last_last":0, "Kp":0, "Ki":0, "Kd":0, "a0":0, "a1":0, "a2":0, "max_irc":100 }

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
	p = icr_pid_s["Kp"] * ( icr_pid_s["err"] - icr_pid_s["err_last"] ) / 1000
	i = icr_pid_s["Ki"] * icr_pid_s["err"] / 1000
	d = icr_pid_s["Kd"] * ( icr_pid_s["err"] - 2*icr_pid_s["err_last"] + icr_pid_s["err_last_last"] ) / 1000
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

def icr_pid_test():
	global icr_pid
	cur = 0
	target = 2000
	count = 0
	icr_pid_init( icr_pid, 50, 10, 20, 100 )
	while ( True ):
		cur += random.randint( -50, 50 )
		cur = cur + icr_pid_loop( icr_pid,  (target - cur) )
		print( "%d"%(cur) )
		count += 1
		# time.sleep( 0.1 )


icr_pid_test()
