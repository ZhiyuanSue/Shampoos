#include <shampoos/common.h>
#include <driver/uart/uart_16550A.h>
void cmain(){
	uart_16550A_open();
	uart_16550A_putc('\n');
	uart_16550A_putc('S');
	uart_16550A_putc('h');
	uart_16550A_putc('a');
	uart_16550A_putc('m');
	uart_16550A_putc('P');
	uart_16550A_putc('o');
	uart_16550A_putc('O');
	uart_16550A_putc('S');
	uart_16550A_putc('\n');
}