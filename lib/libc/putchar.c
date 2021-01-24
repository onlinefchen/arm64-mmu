#include <stdio.h>
#include <debug.h>

int putchar(int c)
{
	int res;
	if (uart_putchar((unsigned char)c) >= 0)
		res = c;
	else
		res = EOF;

	return res;
}
