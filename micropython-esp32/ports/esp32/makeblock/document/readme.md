# 需要阅读和修改的文件 #

1. mods/makeblock文件夹内的文件属于和mbot通信时所必须的一些模块的代码文件，和makeblock module相关的内容最好都整理在这个文件夹内

2. application.mk  这个Makefile文件用于新增文件的编译配置。

3. mptask.c 这个文件是esp32内核的主体程序，命令解析，python解析器，以及文件传输的代码的入口都在这个地方

4. mpconfigport.h   新增python模块时，需要在这里进行配置。


# 需要注意的修改规则 #

1. 文件头的一些简要介绍
2. 函数的命名方式要统一
3. 在函数文件中不要用table键来替代空格键，所有缩进都要按照两个空格的规则。
4. 每个功能做完都需要及时自测，避免代码误入歧途，浪费时间
5. 养成良好的编程习惯

# 控制协议 #

/*

	ff 55 len idx action device dat1 dat2 dat3
	0  1  2   3   4      5      6    7    8
*/

进入原始命名后，通过以下命令来切换到python repl模式


ff 55 04 01 02 50 05    切换到 PYEXEC_MODE_FRIENDLY_REPL

ff 55 04 01 02 50 06    切换到 PYEXEC_MODE_RAW_REPL

# 电机控制协议 #
 
ff 55 06 00 02 0a 0a 64 00

/*

    head   len  idx  action  device   port  speed
    ff 55  06   00   02      0a       0a    64 00
*/

控制port10(电机接口2)的电机转动100

# python的一些语句 #

import makeblock

dc = makeblock.dcmotor()

dc.run(10,100)        //端口10的电机设置100的速度

line = makeblock.linefollower()

line.value()


# 巡线传感器 #

ff 55 04 00 01 11 02
/*

    head   len  idx  action  device   port  speed
    ff 55  04   00   01      11       02
*/

返回值是 ff 55 00 02 00 00 40 40 0d 0a

/*

    head   idx  type  value         end
    ff 55  00   02    00 00 40 40   0d 0a
*/


# 后期修改需要注意的 #

1. communication_channel_init 这个函数后期应该根据硬件状态自动调用和注销

2. 有一个超时判断需要补上