TODO list:

1、设置好IDT表，这需要设置好相应的gdt表中的段，type的设置，dpl，cpl，rpl在64位模式下的使用情况。
2、考虑8259A存在与不存在使用APIC的情况。
3、完善CPUID的部分，不要每次都要读取一遍了。