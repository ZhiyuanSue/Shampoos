# 关于大端和小端存储
我们实现的arch，x86，arm以及riscv和loongarch，均为默认小端存储的
当然其中arm等架构也可以更改其大小端存储的行为，但是为了进行统一，均默认为小端存储，在代码中不再进行大端存储适配。
由于DTB本身是大端存储的，所以，在common/endianness.h中，增加了大小端转换的函数。