# 说明 #
1.将makeblock文件夹里所有文件全部放在外一层
2.加入了新的文件 esp32_hal.c 和esp32_hal.h文件
3.在modmakeblock.c文件中注释掉了 include "machuart.h" 该文件在实际操作中并未使用，且和uart.h文件存在冲突
4.在mpconfigport.c 文件中注册makeblock





# 新主板问题收集#
1.五向开关 管脚定义错误；


2.0,2,12,15,5 管脚需设定固定初始状态 否则很可能造成系统无法启动


3.esp-idf 未定义ADC2——api  自己添加了 ADC2 在adc.h 和Rtc_Moudle中做了修改和注释；（后根据官方建议做了重新修改，运行均无问题）

4.I2c总线出现问题；（更新espidf后没有错误，应该是官方api出现问题）


# 重点说明 #
1.fatfs首地址在sflash_diskio.h里定义；建议定义在0x200000以后；

2.目前_boot.py在程序中未执行，在fatfs初始化函数中由fatfs的初始化，和_boot.py有重合，目前若同时运行这两部分，程序会出错，原因还未找到；

3.ftp的程序执行流程简述如下：
  在mb_ftp_task.c 文件中初始化wifi，同时建立ftp命令解析任务，循环执行mb_ftp_run();在mb_ftp_task.c暂时未使用停止ftp服务等功能，后续将优化；

4.后续思路是将makebloxk相关文件使用一套python程序封装起来，方便接口定义，灵活性也更强；