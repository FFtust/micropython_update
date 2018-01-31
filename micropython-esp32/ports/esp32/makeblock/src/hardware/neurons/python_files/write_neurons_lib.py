import uos
import neurons_lib_string

uos.chdir('/flash/lib')
lib_length = len(neurons_lib_string.lib_str)

if(ucos.stat('/flash/lib/neurons_engine_lib')):
    print("neurons engine existed")
else:
    print("neurons engine not existed")

