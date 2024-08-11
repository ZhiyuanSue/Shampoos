# 8254时钟
这是在使用8259A的情况下，使用8254的时钟。
连接到IRQ0

关于结构
8位控制寄存器
16位初值寄存器CR
16位减一寄存器CE
16位输出锁存寄存器OL

设定初值，复制到减一寄存器
然后OL其实可以读出来，不过好像没啥用

对于计数器，首先需要弄明白的是他所工作的几种模式
方式0：计数结束发送一次中断
方式1：单稳脉冲（同样单次但是并不是设定之后立马开始，而是触发门控信号才开始（不懂））
方式2：循环方式的频率发生器，这应该是我们想要的当成时钟中断的方式
方式3：方波发生器
方式4：软件触发选通信号
方式5：硬件触发选通信号

在正常PC中，他的端口号为40H-42H为timer0-2，然后43H为控制字寄存器。
反正对于每个计数器，都是先写控制字，然后往每个8位的端口中写入低8位，再写入高8位
整体而言不算复杂