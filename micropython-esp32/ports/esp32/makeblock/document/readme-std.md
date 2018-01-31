# 新主板板载传感器API说明 #
注意：

	 1.该文件描述Cpython的接口，仅供调试使用；后面将在此API函数的额基础上再封装一层，有以下好处：

    	a.通过再封装一层Python接口，提高代码的灵活性，且容易阅读和调用，方便用户和软件端的解读；

     	b.方便维护，相比于去维护c语言程序，维护Python程序将简化很多；

     	c.可充分利用Python的优势，更多的体现使用Python的好处；
	 2.除特殊情况外（如参数过多），c语言部分现不提供参数可变特性，这样可提高c端的执行效率，再上一层的python封装将提供这个特性；

       所以调用此API必须严格匹配参数；
## 板载按钮##
说明：新主控电路板一共提供4个按钮，其id分别为1、2、3、4，对应的IO口分别为15、12、0、2；硬件部分由于电路限制，
button1和button3为上拉，初始状态为1，button1和button3为下拉，初始状态为0；程序中做了统一，使初始状态全为0；

###button_board类 函数说明###
init()

  该函数为初始化函数，每个板载传感器的使用均需要调用先调用该函数，否则其他接口程序将不执行，或者不返回任何数据；

deinit()

  该函数为禁止函数，调用该函数后,其他程序将不执行相应操作；

value(arg1,arg2)

  该函数未获取按钮值得函数  
  arg1 为按钮id，值可以为1-4，或者255,255表示得到四个按钮的值，每个值占用一位，如仅button1和button3被按下，则返回5(00000101)
  arg2 为按钮测量值是否翻转标志位，0则不翻转，1则翻转；


####使用举例：####

    import makeblock     ………………………………………………………………………导入makeblock模块，要使用该API里的函数，必须导入该模块

    makeblock.button_board().value(1,0) ………………………………将返回button1的状态，且不做翻转


也可使用另一种方式来简化函数名，使用如下（下同）：

    import makeblock 

    butt=makeblock.button_board()   …………………………………………建造button_board类的一个实例 butt，value为butt实例的一个方法

	butt.value(255,0)               …………………………………………同时得到四个按钮的状态，不翻转

## 板载表情面板##
说明：

	板载表情面板只是在原来公司表情面板的基础上修改了接口形式；使用了IO22(SDA)和IO23(SCL);

    C语言部分暂不提供字符串滚动显示，将在上层的Python层实现；


###ledmatrix_board类 函数说明###
init()
  
  该函数为初始化函数，每个板载传感器的使用均需要调用先调用该函数，否则程序将不执行，或者不返回任何数据；
   
deinit()

  该函数为禁止函数，调用该函数后,其他程序将不执行相应操作；

clean()
  该函数将清除表情面板；

set_brightness(arg1)
  
  该函数用于设置屏幕亮度，共分为8档（0-7） 
  arg1 为亮度参数 可设为0-7

char_show(arg1,arg2,arg3,arg4...argn)
  
  该函数为字符显示函数；
  arg1、arg2 分别为x(0-15)、y(0-7)的位置参数;
  arg3 为显示字符个数
  arg4-argn 共（arg3）个显示数据

time_show(arg1,arg2)
   
  该函数为时间显示函数 将显示时和分
  arg1 小时参数
  arg2 分钟参数

painting_show(arg1，arg2，[arg3...arg18])
  
  该函数为表情面板绘图函数；共需16个显示数据
  arg1、arg2 分别为x(0-15)、y(0-7)的位置参数;
  arg3...arg18 共16个参数


####使用举例####
	from makeblock import ledmatrix_board        …………直接从makeblock模块中导入该类

    lmd=ledmatrix_board()                        …………建造一个ledmatrix_board的实例

    lmd.init()                                   …………调用init函数，必须在建立实例后调用该函数，否则其他程序无法正常执行

    lmd.clean()                                  …………清除表情面板

    lmd.set_brightness(5)                        …………设置亮度为5（7为最亮）

    lmd.char_show(0,0,2,97,98)					 …………显示字符‘a’和‘b’，97和98位两个字符的ascii

    lmd.char_show(0,0,2,ord('a'),ord('b'))       …………也可使用python的转换函数ord，此行将执行和上一行同样的操作

	lmd.time_show(12，30)                        …………显示12：：30

    lmd.painting_show(0,0,0,0,2,18,34,66,64,64,64,64,66,34,18,2,0,0)  ……………………该行将显示一个笑脸   

	lmd.deinit()                                ……………禁止表情面板的相应功能                     