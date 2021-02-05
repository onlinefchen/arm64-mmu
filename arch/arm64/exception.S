/* exception handle */
#include <exception.h>

	.globl	exceptions

	.section	.vectors, "ax"; .align 11

	/* -----------------------------------------------------
	 * Very simple stackless exception handlers used by BL2
	 * and BL3-1 bootloader stages. BL3-1 uses them before
	 * stacks are setup. BL2 uses them throughout.
	 * -----------------------------------------------------
	 */
	.align	7
exceptions:
	/* -----------------------------------------------------
	 * Current EL with SP0 : 0x0 - 0x180
	 * -----------------------------------------------------
	 */
SynchronousExceptionSP0:
	mov	x0, #SYNC_EXCEPTION_SP_EL0
	adr     x0, debug_sp0_str
	bl      uart_print /* asm print welcome message */
	bl 	exception_handle
	b .

	.align	7
IrqSP0:
	mov	x0, #IRQ_SP_EL0
	b .

	.align	7
FiqSP0:
	mov	x0, #FIQ_SP_EL0
	b .

	.align	7
SErrorSP0:
	mov	x0, #SERROR_SP_EL0
	b .

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x380
	 * -----------------------------------------------------
	 */
	.align	7
SynchronousExceptionSPx:
	mov	x0, #SYNC_EXCEPTION_SP_ELX
	adr     x0, debug_spx_str
        bl      uart_print /* asm print welcome message */
	bl 	exception_handle

	.align	7
IrqSPx:
	mov	x0, #IRQ_SP_ELX
	b .

	.align	7
FiqSPx:
	mov	x0, #FIQ_SP_ELX
	b .

	.align	7
SErrorSPx:
	mov	x0, #SERROR_SP_ELX
	b .

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x580
	 * -----------------------------------------------------
	 */
	.align	7
SynchronousExceptionA64:
	mov	x0, #SYNC_EXCEPTION_AARCH64
	b .

	.align	7
IrqA64:
	mov	x0, #IRQ_AARCH64
	b .

	.align	7
FiqA64:
	mov	x0, #FIQ_AARCH64
	b .

	.align	7
SErrorA64:
	mov	x0, #SERROR_AARCH64
	b .

	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x0 - 0x180
	 * -----------------------------------------------------
	 */
	.align	7
SynchronousExceptionA32:
	mov	x0, #SYNC_EXCEPTION_AARCH32
	b .

	.align	7
IrqA32:
	mov	x0, #IRQ_AARCH32
	b .

	.align	7
FiqA32:
	mov	x0, #FIQ_AARCH32
	b .

	.align	7
SErrorA32:
	mov	x0, #SERROR_AARCH32
	b .
