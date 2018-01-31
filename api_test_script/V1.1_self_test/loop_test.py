import codey 

MAIN_LOOP_TEST = 1
THREAD_LOOP_TEST = 0

if MAIN_LOOP_TEST:
	while True:
		a = 0
		a = 'a'
		a = 9
		codey.show(codey.light_strength())

elif THREAD_LOOP_TEST:
	def on_button_callback():
	    while True:
	        codey.show(codey.gyro('roll'))


	codey.on_button('A', on_button_callback)
else:
	def on_button_callback():
	    while True:
	        codey.show(codey.gyro('roll'))


	codey.on_button('A', on_button_callback)
	while True:
		a = 0
		a = 'a'
		a = 9
		codey.show(codey.light_strength())
