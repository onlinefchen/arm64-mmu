/*
 * aarch64 arch define
 */
#ifndef __ARCH_H__
#define __ARCH_H__

#define BIT(n)  (1 << (n))

#define SCTLR_M			BIT(0)
#define SCTLR_A			BIT(1)
#define SCTLR_C			BIT(2)
#define SCTLR_SA		BIT(3)
#define SCTLR_I			BIT(12)
#define SCTLR_EE	        BIT(25)
#define SCTLR_V		        BIT(13)

#define DAIFSET_FIQ		BIT(0)
#define DAIFSET_IRQ		BIT(1)
#define DAIFSET_ABT		BIT(2)
#define DAIFSET_DBG		BIT(3)

#define SPSR_MODE_EL0T		(0x0)
#define SPSR_MODE_EL1T		(0x4)
#define SPSR_MODE_EL1H		(0x5)

#endif /* __ARCH_H__ */
