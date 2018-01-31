import gc
import uos
from flashbdev import bdev

# Begin makeblock 20171109 by fftust
neurons_file_name = 'neurons_engine_lib.bin'

def neurons_file_check():
    uos.chdir('/flash/lib')
    lib_file_list = uos.listdir()
    if not neurons_file_name in lib_file_list:
        print("neurons lib not exist, write one")
        from neurons_lib_string import *
        write_neurons_lib()
    else:
        from neurons_lib_string import *
        current_version = get_current_neurons_lib_version()
        static_version = get_static_neurons_lib_version()
        if current_version == None:
            print("neurons lib version is not valid, write the version: ", static_version)
            write_neurons_lib()
        else:
            if int(static_version) > int(current_version):
                print("neurons lib version is old, write the version: ", static_version)
                write_neurons_lib()
    uos.chdir('/flash')  

try:
    if bdev:
        #uos.mount(bdev, '/')
        vfs = uos.VfsFat(bdev)
        uos.mount(vfs, '/flash')

except OSError:
    import inisetup
    vfs = inisetup.setup()

# check the neurons existed or not
neurons_file_check()

# End makeblock 20171109 by fftust
gc.collect()
