import codey

def on_shake_callback():
    codey.face('00001020402012020212204020100000', 1)

codey.on_shake(on_shake_callback)

def on_tilt_callback():
    codey.color('#ff0000', 1)

codey.on_tilt('forward', on_tilt_callback)

def on_tilt1_callback():
    codey.color('#7fff00', 1)

codey.on_tilt('backward', on_tilt1_callback)

def on_tilt2_callback():
    codey.color('#3f00ff', 1)

codey.on_tilt('left', on_tilt2_callback)

def on_tilt3_callback():
    codey.color('#ffffff', 1)

codey.on_tilt('right', on_tilt3_callback)
