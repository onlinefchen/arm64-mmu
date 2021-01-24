#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#define SYNC_EXCEPTION_SP_EL0		0x0
#define IRQ_SP_EL0			0x1
#define FIQ_SP_EL0			0x2
#define SERROR_SP_EL0			0x3
#define SYNC_EXCEPTION_SP_ELX		0x4
#define IRQ_SP_ELX			0x5
#define FIQ_SP_ELX			0x6
#define SERROR_SP_ELX			0x7
#define SYNC_EXCEPTION_AARCH64		0x8
#define IRQ_AARCH64			0x9
#define FIQ_AARCH64			0xa
#define SERROR_AARCH64			0xb
#define SYNC_EXCEPTION_AARCH32		0xc
#define IRQ_AARCH32			0xd
#define FIQ_AARCH32			0xe
#define SERROR_AARCH32			0xf

#define BITS_PER_LONG                   64

#define BIT_MASK(nr)		((1UL << nr) - 1)

#define GET_EL(_mode)		(((_mode) >> MODE_EL_SHIFT) & MODE_EL_MASK)

#define ESR_EC(esr)		(((esr) >> 26) & BIT_MASK(6))
#define ESR_IL(esr)		(((esr) >> 25) & BIT_MASK(1))
#define ESR_ISS(esr)		((esr) & BIT_MASK(25))

#endif
