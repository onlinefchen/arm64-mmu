#include <sizes.h>
#include <cpu.h>
#include <msr.h>
#include <exception.h>

#define ESR_EC(esr)		(((esr) >> 26) & BIT_MASK(6))

static void dump_esr()
{
	unsigned int esr = read_msr(ESR_EL2);
	const char *err;
	printf("ESR_ELn: 0x%016llx", esr);
	printf("  EC:  0x%llx", ESR_EC(esr));
	printf("  IL:  0x%llx", ESR_IL(esr));
	printf("  ISS: 0x%llx\n", ESR_ISS(esr));
	printf("  ELR: 0x%llx\n", read_msr(elr_el2));
	switch (ESR_EC(esr)) {
	case 0b000000: /* 0x00 */
		printf("Unknown reason");
		break;
	case 0b000001: /* 0x01 */
		printf("Trapped WFI or WFE instruction execution");
		break;
	case 0b000011: /* 0x03 */
		printf("Trapped MCR or MRC access with (coproc==0b1111) that "
		      "is not reported using EC 0b000000");
		break;
	case 0b000100: /* 0x04 */
		printf("Trapped MCRR or MRRC access with (coproc==0b1111) "
		      "that is not reported using EC 0b000000");
		break;
	case 0b000101: /* 0x05 */
		printf("Trapped MCR or MRC access with (coproc==0b1110)");
		break;
	case 0b000110: /* 0x06 */
		printf("Trapped LDC or STC access");
		break;
	case 0b000111: /* 0x07 */
		printf("Trapped access to SVE, Advanced SIMD, or "
		      "floating-point functionality");
		break;
	case 0b001100: /* 0x0c */
		printf("Trapped MRRC access with (coproc==0b1110)");
		break;
	case 0b001101: /* 0x0d */
		printf("Branch Target Exception");
		break;
	case 0b001110: /* 0x0e */
		printf("Illegal Execution state");
		break;
	case 0b010001: /* 0x11 */
		printf("SVC instruction execution in AArch32 state");
		break;
	case 0b011000: /* 0x18 */
		printf("Trapped MSR, MRS or System instruction execution in "
		      "AArch64 state, that is not reported using EC "
		      "0b000000, 0b000001 or 0b000111");
		break;
	case 0b011001: /* 0x19 */
		printf("Trapped access to SVE functionality");
		break;
	case 0b100000: /* 0x20 */
		printf("Instruction Abort from a lower Exception level, that "
		      "might be using AArch32 or AArch64");
		break;
	case 0b100001: /* 0x21 */
		printf("Instruction Abort taken without a change in Exception "
		      "level.");
		break;
	case 0b100010: /* 0x22 */
		printf("PC alignment fault exception.");
		break;
	case 0b100100: /* 0x24 */
		printf("Data Abort from a lower Exception level, that might "
		      "be using AArch32 or AArch64");
		break;
	case 0b100101: /* 0x25 */
		printf("Data Abort taken without a change in Exception level");
		break;
	case 0b100110: /* 0x26 */
		printf("SP alignment fault exception");
		break;
	case 0b101000: /* 0x28 */
		printf("Trapped floating-point exception taken from AArch32 "
		      "state");
		break;
	case 0b101100: /* 0x2c */
		printf("Trapped floating-point exception taken from AArch64 "
		      "state.");
		break;
	case 0b101111: /* 0x2f */
		printf("SError interrupt");
		break;
	case 0b110000: /* 0x30 */
		printf("Breakpoint exception from a lower Exception level, "
		      "that might be using AArch32 or AArch64");
		break;
	case 0b110001: /* 0x31 */
		printf("Breakpoint exception taken without a change in "
		      "Exception level");
		break;
	case 0b110010: /* 0x32 */
		printf("Software Step exception from a lower Exception level, "
		      "that might be using AArch32 or AArch64");
		break;
	case 0b110011: /* 0x33 */
		printf("Software Step exception taken without a change in "
		      "Exception level");
		break;
	case 0b110100: /* 0x34 */
		printf("Watchpoint exception from a lower Exception level, "
		      "that might be using AArch32 or AArch64");
		break;
	case 0b110101: /* 0x35 */
		printf("Watchpoint exception taken without a change in "
		      "Exception level.");
		break;
	case 0b111000: /* 0x38 */
		printf("BKPT instruction execution in AArch32 state");
		break;
	case 0b111100: /* 0x3c */
		printf("BRK instruction execution in AArch64 state.");
		break;
	default:
		printf("Unknown");
	}
	printf("\n");
	asm volatile("b .");
}

int exception_handle()
{
	dump_esr();
}
