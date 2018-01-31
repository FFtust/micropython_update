import codey
import rocky
import time
version = '1_1'
codey.set_variable('cmd', "0")
codey.set_variable('v', version)

def run(l, r, duration):
    print( "l: %d r: %d, d: %f"%(l, r, duration) )
    rocky.drive(l, r)
    # rocky.forward( l )
    time.sleep(duration)

def runCmd(cmd):
  if len(cmd) < 1:
    return
  for i, j in enumerate(cmd):
    print( "index: %d"%i )
    if j == 0:
        # rocky.stop()
        codey.set_variable('cmd', "0_0_0")
    else:
        t = type(j)
        if t == str:
            if i == 0:
                if len(j) > 0:
                    codey.say(j + '.wav')
            else:
                codey.face(j)
        elif t == tuple:
            run(j[0],j[1],j[2])

while True:
    cmdRaw = codey.get_variable('cmd')
    if len(cmdRaw) == 0:
        rocky.stop()
        codey.set_variable('cmd', "0_0_0")
        continue
    cmdData = cmdRaw.split('_')
    typeA = int(cmdData[0])
    if typeA == 0:
        if len(cmdData) == 3:
            lS = int(cmdData[1])
            rS = int(cmdData[2])
            rocky.drive(lS, rS)
        else:
            rocky.stop()
    elif typeA == 1:
        runCmd(['start','00003c1e0e0400000000040e1e3c0000',(100,100,1), 0])
    elif typeA == 2:
        runCmd(['jump','1c224a524c201c00001c204c524a221c',(100,-100,1.3), 0])
    elif typeA == 3:
        runCmd(['wrong','0000363e1c3e360000363e1c3e360000',(0,90,0.1),(90,0,0.2),(0,90,0.2),(90,0,0.1),0])
    elif typeA == 4:
        runCmd(['happy','000c18181c0c000000000c1c18180c00',(90,90,0.2),(-90,-90,0.3),(90,90,0.1),0])
    elif typeA == 5:
        runCmd(['sad','00000c1e3e3c000000003c3e1e0c0000',(0,90,0.1),(-70,-70,0.3),(90,0,0.3),(-70,-70,0.3),(0,90,0.1),0])
    elif typeA == 6:
        runCmd(['wake','003c4242423c000000003c4242423c00',(-80,-80,0.4),0])
    elif typeA == -1:
        codey.set_variable('v', version)
        codey.set_variable('cmd', "0")
