import codey
import rocky
import time
version = '1_1'
codey.set_variable('cmd', "0")
codey.set_variable('v', version)

def run(dir, duration):
    if dir == 0:
        rocky.drive(200, 200)
    elif dir == 1:
        rocky.drive(0, 200)
    elif dir == 2:
        rocky.drive(-200, -200)
    elif dir == 3:
        rocky.drive(200, 0)
    elif dir == 4:
        rocky.drive(0, 0)
    time.sleep(duration)

def runCmd(cmd):
  if len(cmd) < 1:
    return
  for i, j in enumerate(cmd):
    if j == 0:
        rocky.stop()
        codey.set_variable('cmd', "0_0_0")
    else:
        t = type(j)
        if t == str:
            codey.face(j)
        elif t == tuple:
            run(j[0], j[1])

while True:
    cmdRaw = codey.get_variable('cmd')
    if len(cmdRaw) == 0:
        rocky.stop()
        codey.set_variable('cmd', "0_0_0")
        continue
    cmdData = cmdRaw.split('_')
    typeA = int(cmdData[0])

	# for debug
    # typeA = -1

    if typeA == 0:
        if len(cmdData) == 3:
            lS = int(cmdData[1])
            rS = int(cmdData[2])
            rocky.drive(lS, rS)
        else:
            rocky.stop()
    elif typeA == 1:
        runCmd(['00003c1e0e0400000000040e1e3c0000', (0,3), 0])
    elif typeA == 2:
        runCmd(['1c224a524c201c00001c224a524c201c', (3,3), 0])
    elif typeA == 3:
        runCmd(['00363e1c3e3600000000363e1c3e3600', (1,0.5), (2,1), (3,0.5), 0])
    elif typeA == 4:
        runCmd(['000c18181c0c000000000c1c18180c00', (0,0.3), (2,0.3), (2,0.3), (0,0.3), 0])
    elif typeA == 5:
        runCmd(['00000c1e3e3c000000003c3e1e0c0000', (1,0.4), (2,0.3), (3,0.6), (2,0.3),(1,0.4), 0])
    elif typeA == 6:
        runCmd(['003c4242423c000000003c4242423c00', (2,0.3), 0])
    elif typeA == -1:
        codey.set_variable('v', version)
        codey.set_variable('cmd', "0")