#include <pl011.h>
#include <macro.S>

	.global uart_enable
	.global uart_print

/*
 * x0 -  the uart base address
 * x1 -  uart clock hz
 * x2 -  baud rate
 */
uart_enable:
	ldr  x0, =0x09000000
	ldr  x1, =19200000
	ldr  x2, =115200
	/* Disable uart before programming */
	ldr	w3, [x0, #UARTCR]
	mov	w4, #PL011_UARTCR_UARTEN
	bic	w3, w3, w4
	str	w3, [x0, #UARTCR]
	/* Program the baudrate */
	/* Divisor =  (Uart clock * 4) / baudrate */
	lsl	w1, w1, #2
	udiv	w2, w1, w2
	/* IBRD = Divisor >> 6 */
	lsr	w1, w2, #6
	/* Write the IBRD */
	str	w1, [x0, #UARTIBRD]
	/* FBRD = Divisor & 0x3F */
	and	w1, w2, #0x3f
	/* Write the FBRD */
	str	w1, [x0, #UARTFBRD]
	mov	w1, #PL011_LINE_CONTROL
	str	w1, [x0, #UARTLCR_H]
	/* Clear any pending errors */
	str	wzr, [x0, #UARTECR]
	/* Enable tx, rx, and uart overall */
	mov	w1, #(PL011_UARTCR_RXE | PL011_UARTCR_TXE | PL011_UARTCR_UARTEN)
	str	w1, [x0, #UARTCR]
	mov	w0, #1
	ret


/*
 * x0 - the output strings address
 * x1 - the pl011 base address
 */
uart_print:
	mov     x1, #0x09000000
	ldrb    w2, [x0], #1
	strb    w2, [x1]
	cmp     x2, #0xA
	b.ne uart_print
	ret



.section .rodata, "a"
	.global welcome
	.global debug_spx_str
	.global debug_sp0_str
welcome:
	.ascii "staring kopernik...\n"
debug_spx_str:
	.ascii "SynchronousExceptionSPx ...\n"
debug_sp0_str:
	.ascii "SynchronousExceptionSP0 ...\n"
