# x86_64
x86_64下
首先看中断向量表
对于CPU的中断，他包括了32个CPU自己定义好的中断，然后剩下的中断则是用于外设的。但是也可以用int n触发
这部分可以看手册的第六章。

外设的中断源
我这边试图写四种情况下
## no APIC
这时候只能用8259A和8254（用于时钟），但是正常情况需要disable掉APIC，才会只使用8259A
https://blog.csdn.net/weixin_46716100/article/details/122205489
我找了一篇文章来介绍使用8259A
网上相关文章也很多
简单来说，就是使用ICW1-4四个命令字按照顺序写到奇端口和偶端口，这部分初始化会设置好优先级，中断号等一堆东西

OCW1-3命令字用于控制屏蔽中断，以及结束中断等

## APIC

### Local APIC
首先使用CPUID，EAX=01，返回的EDX中bit9置位表示支持local APIC

对于Local APIC，他放在1个4K的block中
查看10.4了解其布局
初试物理地址都在0xFEE00000，他必须被映射到一个strong uncachable的区域（UC），见11.3

可选的一个动作是，把这个地址空间，映射到另一个区域
见10.4.5

访问所有的32bit的寄存器，64bit或者256bit的寄存器，必须使用128bit的align

有个IA32_APIC_BASE MSR寄存器，见10.4.3，来启用或者禁用APIC
这个寄存器低12位中，bit11表示APIC是否启用，bit10表示启用x2APIC模式，bit8表示处理器是BSP
然后bit12-maxphyaddr是表示APIC base

APIC ID,在多核中，APIC ID还被当做CPU ID，这个值还可以通过CPUID命令，EAX=1，返回的EBX中的bit31-24来决定，即使写了APIC寄存器空间的APIC ID，CPUID命令也会正确返回通过启动决定的CPUID
这个寄存器放在FEE0 0020H

#### INIT Reset和多核启动的Wait for init状态
这个不细说了

#### Local APIC VERSION register
bit 0-7表示vition，0XH都是82489，10-15H都是integrated APIC
bit 16-23为MAX LVT entry
bit 24 为EOI，翻译是说，用户软件是否可以通过设置Spurious Interrupt Vector rigister的bit 12禁用broadcast EOI消息

#### LVT寄存器
可以见10.5.1
包括CMCI、Timer（需要额外的寄存器支持）、Thermal、Performance、LINT0、LINT1、Error几个寄存器
最低8位是Vector，表示对应的core里面的中断。
其他的位也值得关注，看Figure10-8

#### ESR寄存器
几个位表明出错的原因

#### Timer寄存器
包括
divide configure寄存器
initial count寄存器
current count寄存器
LVT timer寄存器

首先是否支持还是需要看CPUID
CPUID.06H.EAX.ARAT[bit2]

APIC的频率是CPU的bus clock，然后divide了divide configure寄存器配置的值

##### TSC模式
这是基于CPU的时钟而不是CPU的总线，相对更加精确
需要CPUID.01H.ECX.TSC_Deadline支持

而这还涉及到一个IA32_TSC_DEADLINE寄存器（MSR地址为6E0H）而不是使用initial count和current count
反正就是倒计时一次

TSC模式需要做的事情就是
1、检测是否支持TSC
2、在LVT Timer寄存器中设置为TSC的模式
3、在IA32_TSC_DEADLINE寄存器写入东西
4、会触发中断
5、重复3开始的步骤

#### IPI
##### 发送
ICR寄存器
Interrupt Command Register

IPI本身可以向其他核心发消息，转发自己来不及处理的中断信号，向自己发消息，以及发送SIPI开始多核启动，注意，这个SIPI消息本身不会在失败的时候重发（其他的消息都会）

ICR寄存器本身是64位的寄存器（在xAPIC中是ICR_LOW和ICR_HIGH，两个寄存器），除了delivery status位是只读的之外都是可读写的

vector位

Delivery Mode，其中特别的是SIPI（0b110）

Destination shorthand，四个模式，00表示使用destination位指明的，01表示自己，10表示包括自己在内的所有，11表示除了自己之外的所有
其他还有一些位
（需要注意不是所有的组合都有效）对于后三种模式，下面的LDR和DFR就没啥用了

决定IPI的除了ICR寄存器之外，还包括
Local destination register（LDR）以及Destination format register（DFR）

物理dst模式
直接使用APIC ID作为目的地

逻辑dst模式
使用一个8bit的message destination address（MDA），这个MDA
在LDR的高8位写入了logical APIC ID
对于DFR寄存器，只有高4位有效并且要么全都是1表示flat模式，要么全都是0表示cluster模式，剩余全都是1

flat模式会让MDA和LDR做and，只要不为0就接受
cluster模式（太复杂了，看不明白）

Lowest Priority Delivery模式
大概意思是，会发一圈，但是最终只有优先级最低的那个接受了这个IPI

需要使用到APR寄存器（Arbitrary Priority Register）

最后，当ICR寄存器的低double word（注：32bit）被写入，那么IPI就被发出去到系统总线上了。

##### 接受

对于什么情况下会接受IPI，首先得看10-17的流程图，如果type是NMI，SMI，INIT，ExtINIT等类型可以直接accept，其他的需要加以判断。

TPR寄存器（Task-Priority Register），3-0是Task Priority sub-class，只有高于7:4位指定的Task Priority class（可读写）的才会被送给cpu
（在64位模式下，可以写CR8，效果是一样的，APIC.TPR[7:4]=CR8[7:4]）

PPR寄存器（Processor-Priority Register），布局和TPR差不多。但是他是只读的，
表示CPU正在执行的中断的优先级。
7:4位是TPR[7:4]和ISRV[7:4]取最大值
如果TPR[7:4]更大，那么PPR[3:0]为TPR[3:0]
否则为0

对于优先级的定义，x86_64里面我没找到中断优先级定义的寄存器（和aarch64以及riscv64不同）

不过，由于前面的流程图里面，优先判断了那几种中断，所以那几种中断并不会被阻挡

IRR寄存器（Interrupt Request Register）
保存了已经accept但尚未给CPU的
ISR寄存器（In Service Register）
和IRR一样都是256位的，但是0-15都是保留的，因为0-15号向量是CPU保留的（所以是无效的vector号）
当CPU准备能够接受下一个中断时，清理掉IRR最高优先级的那个位，并在ISR的那一位置位

可以同时设置IRR和ISR中的对应位，也就是说有一个中断正在被service，而另一个在IRR中排队等待。这是可行的。也就是说，对于同一个中断，最多缓存两个。

中断嵌套，如果有更高的中断，并且cpu没有禁止中断，那么可以立即打断CPU而无需清理EOI，也就是达成了中断嵌套。

TMR寄存器（Trigger Mode Register）
高低电平相关的（不太懂），但是说法是，如果是level-trigger（该位置位为1）的，在EOI写入时，会向所有的IO APIC发一次广播
（0为edge-triggered）

EOI寄存器（End Of Interrupt）
在Iret指令之前，需要写入EOI（这是一个32位的寄存器），表明中断已经处理完了。
至于写入什么，我查到的资料说是写个0（手册也没指定是哪个呀）
正如TMR中写的那样，EOI写入的时候，如果设置了TMR，会发送一次APIC的广播。这一点可以在Spurious Interrupt Vector Register的bit12来禁用（但是见上面的VERSION寄存器，里面也有一个位来表明是否可以做这个禁用）
默认情况下是可以用的

手册说推荐的做法是，禁用，然后直接写产生这个中断的IOAPIC的EOI寄存器

SVR寄存器（Spurious-Interrupt Vector Register）

bit12表明是否允许EOI的broadcast
bit9为focus Processor Checking
bit8为APIC enable/disable
bit7:0为Spurious Vector
这个vector的含义是说，如果我试图提升我的TPR的等级的时候，这时候如果新来一个中断，这会被masked
于是他会发送一个这个中断

MSI（Message Signal Interrupt）机制
主要包括两个寄存器（message data register）和（message address register）
但是这是属于PCI总线相关的内容，只是他确实会发起一个MSI中断罢了


## xAPIC

## x2APIC
CPUID:01H:ECX[21]=1表示支持x2APIC
在x2APIC中，APIC ID作为MSR 802H寄存器。整个32位都被当做ID

x2APIC中，使用了MSR寄存器而不再是内存映射的方式去做。
同样需要先考虑IA32_APIC_BASE MSR寄存器，bit11和bit10，如果都为0，则禁用
如果01，无效，10，只开启xAPIC，11则开启x2APIC模式
这种模式下，使用RDMSR和WRMSR进行操作MSR寄存器

使用ECX指定地址，EAX为低32位，EDX为高32位
（说ICR寄存器是唯一会用到的64位的寄存器）

地址范围：800H到BFFH是保留给x2APIC的
正常情况下是一一对应的，但是有例外。
1、在x2APIC中没有DFR寄存器（80E0H不存在）
2、ICR寄存器，合并到了一个共同的64位ICR寄存器
3、在x2APIC下独有的Self IPI寄存器

还有保留位的存在，建议详看10-6table

以及必须只能在x2APIC模式下才能正确的使用MSR寄存器的方式访问，其他方式都会出错。

不能保证他一定是serializable的
如果需要，必须使用SFENCE以及MFENCE等操作。
不过，在MMIO模式下，他提到了需要映射到一个非cached的页面中，因此这部分可以保证某种有序性

如何开启x2APIC
初始化的情况下，xAPIC（IA32_APIC_BASE[bit11]）是启用的，然后EXTAPIC（bit10）是默认关闭的


CPUID 0x0B扩展（enumaration leaf）
首先检查CPUID最大支持的是多少，如果大于等于0B，那么检查CPUID.0BH,ECX=0,而EBX不等于0

在x2APIC中，APIC ID占用32位，因此，如果直接用之前的CPUID就没啥用了，因此需要额外使用CPUID.0BH:EDX来存放这个32位的CPUID。

并且这个CPUID的[7:0]和之前的CPUID.01H:EBX[31:24]一样

对于x2APIC的ICR寄存器，看figure 10-28，和之前又有些不同，APCI ID字段增加了，删掉了Delivery status字段，手册说，那就再也没必要读这个ICR寄存器了

DFR寄存器被删掉了，那么LDR寄存器则被扩展到了32位，高16位表示cluster，低16位表示logical APIC ID

local APIC ID转换到logical APIC ID（取19:4位拼上3:0）

Self IPI寄存器
最低位是一个vector
和使用ICR寄存器选择自己中断自己，是一样的


## syscall and sysret
具体见column 3的5.8.8，和之前使用中断的方式不同，这个快速系统调用，需要额外考虑
这里只说64位模式。
首先是在gdt表中，需要额外考虑段的情况

IA32_STAR选择了具体的段，这里最高16位用于描述sysret的段，接下来16位用于描述syscall的段

IA32_LSTAR选择syscall的入口地址

堆栈的段是通过IA32_START加8（也就是gdt表中的下一个条目，所以至少在用户态和内核态需要增加gdt表的内容，至少包含用户的堆栈和内核的堆栈的段。）
使用IA32_FMASK来和当前的RFLAGS进行and运算，作为进入syscall的保留使用的位。（可以通过这种方式直接屏蔽中断位）

sysret的时候，则是使用高16位往后数两个的段作为sysret的段
rip则是从RCX复制到RIP（所以需要设置RCX寄存器，如果设错了，会有GP异常）
EFLAGS则是从R11复制回来的。

总之，在做x86的syscall这部分的时候，可以不考虑中断，因为这已经是另外的东西了。

# aarch64
arm下的中断，首先需要区分同步和异步
同步表示的是当前运行的指令报错了，而异步则是用于外设中断
异步还分为三类，包括IRQ和FIQ以及SError
总共有四种，SError本身是可以不处理的，因为我确实查不到更多的信息，Linux也没有处理，只是上报。

异常向量表中则只定义了16个entry（每个level），异常向量表由VBAR寄存器指定

分别是
current el with sp0
current el with spx
lower el with aarch64
lower el with aarch32

每个分别有同步异常，以及IRQ FIQ以及SError

同步异常
寄存器包括ESR，表示异常的原因
FAR错误虚拟地址
ELR寄存器则是中断返回地址。（也就是eret返回的地址）

异步的这些中断来自于中断寄存器GIC

GIC目前也有四个版本
v1-v4

对于aarch64下的syscall，会有个SVC（对于EL2有HVC，EL3有secure monitor call）
通过这种方式去搞syscall

SPSR寄存器
这里面有一堆寄存器的位需要处理
DAIF四个位表示允许屏蔽异常事件
D表示debug，A表示SError，I，IRQ，F，FIQ

SPSel，选择SP0或者SP_x


# riscv

还是同样的分为同步异常和异步异常

mcause和scause表示了异常的原因，最高位是interrupt字段，表示原因，如果是1则是异步异常，而为0则是同步异常
同步异常也是各种处理器的问题，得看exception code字段，而异步异常同样看EC字段，（不考虑M模式还是S模式的话）主要分为软件中断（比如ipi），时钟中断，外部中断三种。
（s模式的一些字段的修改会影响m模式的一些字段，只是包含在了m模式的字段当中）

mstatus中的MIE和SIE字段用于使能M模式和S模式下的中断
（和mie寄存器和sie寄存器不是一回事）

MPIE和SPIE则是用于临时保存之前的中断使能状态。
SPP和MPP是中断之前的特权模式

mtvec寄存器和stvec寄存器，在最后两位是mode，表示通过统一的入口函数还是通过中断向量表的方式

mie寄存器，对于异步异常的s和m模式下的软件中断，时钟中断，外部中断，分别进行使能，共6种
sie寄存器同理，共3种

mtval寄存器表明发生异常的虚拟地址

mip表示需要响应的外部中断的情况，总共也是6种

mideleg和medeleg（在m模式下委托中断给s模式，s态下用不到）

正常情况下的初始化流程
配置vec寄存器
配置plic等中断初始化外设中断
使能mie或sie寄存器
配置mstatus寄存器

上下文保存
x1-x31
sepc
sstatus
scause
ra
sp
等的值


## PLIC中断控制器
分为本地的中断和外部的中断，PLIC那一大堆寄存器，就是配置一堆中断号的寄存器的使能，优先级等等。


因此总体而言，对于中断处理的这部分代码，x86_64和aarch64架构，我都需要做
1，对于CPU定义好的中断的处理（这部分需要优先做）
2，控制好apic，gic等中断控制器的处理。包括这些抽象等。