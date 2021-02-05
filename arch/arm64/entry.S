/*
 * entry porint nothing to describe
 * go go sliver
 */

#include <cpu.h>
#include <arch.h>

.section .init.entry, "ax"
.global entry
.global debug_print
entry:
	/* Linux aarch64 image header format */
	b 	real_entry
	.word 	0
	.quad 	0x1000      /* text_offset */
	.quad 	_image_size  /* image_size */
	.quad 	0           /* flags */
	.quad 	0           /* res2 */
	.quad 	0           /* res3 */
	.quad 	0           /* res4 */
	.word 	0x644d5241  /* magic */
	.word 	0

real_entry:
	mrs     x0, sctlr_el2
	bic     x0, x0, #SCTLR_EE
	msr     sctlr_el2, x0
	isb

	bl 	uart_enable
	adr 	x0, welcome
	bl 	uart_print /* asm print welcome message */

	/* exception handlers */
	adrp 	x0, exceptions
	add 	x0, x0, :lo12:exceptions
	msr	vbar_el2, x0

	/* serror enable */
	msr	daifclr, #DAIFSET_ABT

	/* enable icache and alignment check */
	mov	x1, #(SCTLR_I | SCTLR_A | SCTLR_SA)
	mrs	x0, sctlr_el2
	orr	x0, x0, x1
	msr	sctlr_el2, x0
	isb

	bl 	zero_bss
	/* prepare c stack */
	bl 	prepare_for_c
	bl 	main


prepare_for_c:
	/* set the cpu id */
	adrp 	x28, cpu_stack
	add 	x28, x28, :lo12:cpu_stack

	/* Use SPx (instead of SP0). */
	msr 	spsel, #1

	/* Prepare the stack. */
	mov 	sp, x28
	ret


zero_bss:
        /* Zero out the bss section. */
	adrp x0, _bss_start
	add x0, x0, :lo12:_bss_start
	adrp x1, _bss_end
	add x1, x1, :lo12:_bss_end
zero_16:
	cmp x0, x1
	b.eq end
	stp xzr, xzr, [x0], #16
	b zero_16
end:
	ret


debug_print:
	adr 	x0, welcome
	bl 	uart_print /* asm print welcome message */
	ret
