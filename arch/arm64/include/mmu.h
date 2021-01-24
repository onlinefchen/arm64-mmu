#ifndef __MMU_H__
#define __MMU_H__

#include <stddef.h>

#define CONFIG_ARM64_VA_BITS 100
#define CONFIG_MMU_PAGE_SIZE 0x1000
#define CONFIG_MAX_XLAT_TABLES 32


#define __aligned(x)	__attribute__((__aligned__(x)))

#define MMU_DEBUG_PRINTS	1

/* To get prints from MMU driver, it has to initialized after console driver */
#define MMU_DEBUG_PRIORITY	70

#if MMU_DEBUG_PRINTS
/* To dump page table entries while filling them, set DUMP_PTE macro */
#define DUMP_PTE		1
#define MMU_DEBUG(fmt, ...)	printf(fmt, ##__VA_ARGS__)
#else
#define MMU_DEBUG(...)
#endif

#if DUMP_PTE
#define L0_SPACE ""
#define L1_SPACE "  "
#define L2_SPACE "    "
#define L3_SPACE "      "
#define XLAT_TABLE_LEVEL_SPACE(level)		\
	(((level) == 0) ? L0_SPACE :		\
	((level) == 1) ? L1_SPACE :		\
	((level) == 2) ? L2_SPACE : L3_SPACE)
#endif

/*
 * 48-bit address with 4KB granule size:
 *
 * +------------+------------+------------+------------+-----------+
 * | VA [47:39] | VA [38:30] | VA [29:21] | VA [20:12] | VA [11:0] |
 * +---------------------------------------------------------------+
 * |     L0     |     L1     |     L2     |     L3     | block off |
 * +------------+------------+------------+------------+-----------+
 */

/* Only 4K granule is supported */
#define PAGE_SIZE_SHIFT		12U

/* 48-bit VA address */
#define VA_SIZE_SHIFT_MAX	48U

/* Maximum 4 XLAT table (L0 - L3) */
#define XLAT_LEVEL_MAX		4U

/* The VA shift of L3 depends on the granule size */
#define L3_XLAT_VA_SIZE_SHIFT	PAGE_SIZE_SHIFT

/* Number of VA bits to assign to each table (9 bits) */
#define Ln_XLAT_VA_SIZE_SHIFT	((VA_SIZE_SHIFT_MAX - L3_XLAT_VA_SIZE_SHIFT) / \
				 XLAT_LEVEL_MAX)

/* Starting bit in the VA address for each level */
#define L2_XLAT_VA_SIZE_SHIFT	(L3_XLAT_VA_SIZE_SHIFT + Ln_XLAT_VA_SIZE_SHIFT)
#define L1_XLAT_VA_SIZE_SHIFT	(L2_XLAT_VA_SIZE_SHIFT + Ln_XLAT_VA_SIZE_SHIFT)
#define L0_XLAT_VA_SIZE_SHIFT	(L1_XLAT_VA_SIZE_SHIFT + Ln_XLAT_VA_SIZE_SHIFT)

#define LEVEL_TO_VA_SIZE_SHIFT(level)			\
	(PAGE_SIZE_SHIFT + (Ln_XLAT_VA_SIZE_SHIFT *	\
	((XLAT_LEVEL_MAX - 1) - (level))))

/* Number of entries for each table (512) */
#define Ln_XLAT_NUM_ENTRIES	(1U << Ln_XLAT_VA_SIZE_SHIFT)

/* Virtual Address Index within a given translation table level */
#define XLAT_TABLE_VA_IDX(va_addr, level) \
	((va_addr >> LEVEL_TO_VA_SIZE_SHIFT(level)) & (Ln_XLAT_NUM_ENTRIES - 1))

/*
 * Calculate the initial translation table level from CONFIG_ARM64_VA_BITS
 * For a 4 KB page size:
 *
 * (va_bits <= 20)	 - base level 3
 * (21 <= va_bits <= 29) - base level 2
 * (30 <= va_bits <= 38) - base level 1
 * (39 <= va_bits <= 47) - base level 0
 */
#define GET_BASE_XLAT_LEVEL(va_bits)				\
	 ((va_bits > L0_XLAT_VA_SIZE_SHIFT) ? 0U		\
	: (va_bits > L1_XLAT_VA_SIZE_SHIFT) ? 1U		\
	: (va_bits > L2_XLAT_VA_SIZE_SHIFT) ? 2U : 3U)

/* Level for the base XLAT */
#define BASE_XLAT_LEVEL	GET_BASE_XLAT_LEVEL(CONFIG_ARM64_VA_BITS)

#if (CONFIG_ARM64_PA_BITS == 48)
#define TCR_PS_BITS TCR_PS_BITS_256TB
#elif (CONFIG_ARM64_PA_BITS == 44)
#define TCR_PS_BITS TCR_PS_BITS_16TB
#elif (CONFIG_ARM64_PA_BITS == 42)
#define TCR_PS_BITS TCR_PS_BITS_4TB
#elif (CONFIG_ARM64_PA_BITS == 40)
#define TCR_PS_BITS TCR_PS_BITS_1TB
#elif (CONFIG_ARM64_PA_BITS == 36)
#define TCR_PS_BITS TCR_PS_BITS_64GB
#else
#define TCR_PS_BITS TCR_PS_BITS_4GB
#endif

/* Upper and lower attributes mask for page/block descriptor */
#define DESC_ATTRS_UPPER_MASK	GENMASK(63, 51)
#define DESC_ATTRS_LOWER_MASK	GENMASK(11, 2)

#define DESC_ATTRS_MASK		(DESC_ATTRS_UPPER_MASK | DESC_ATTRS_LOWER_MASK)

/*
 * TCR definitions.
 */
#define TCR_EL1_IPS_SHIFT	32U
#define TCR_EL2_PS_SHIFT	16U
#define TCR_EL3_PS_SHIFT	16U

#define TCR_T0SZ_SHIFT		0U
#define TCR_T0SZ(x)		((64 - (x)) << TCR_T0SZ_SHIFT)

#define TCR_IRGN_NC		(0ULL << 8)
#define TCR_IRGN_WBWA		(1ULL << 8)
#define TCR_IRGN_WT		(2ULL << 8)
#define TCR_IRGN_WBNWA		(3ULL << 8)
#define TCR_IRGN_MASK		(3ULL << 8)
#define TCR_ORGN_NC		(0ULL << 10)
#define TCR_ORGN_WBWA		(1ULL << 10)
#define TCR_ORGN_WT		(2ULL << 10)
#define TCR_ORGN_WBNWA		(3ULL << 10)
#define TCR_ORGN_MASK		(3ULL << 10)
#define TCR_SHARED_NON		(0ULL << 12)
#define TCR_SHARED_OUTER	(2ULL << 12)
#define TCR_SHARED_INNER	(3ULL << 12)
#define TCR_TG0_4K		(0ULL << 14)
#define TCR_TG0_64K		(1ULL << 14)
#define TCR_TG0_16K		(2ULL << 14)
#define TCR_EPD1_DISABLE	(1ULL << 23)

#define TCR_PS_BITS_4GB		0x0ULL
#define TCR_PS_BITS_64GB	0x1ULL
#define TCR_PS_BITS_1TB		0x2ULL
#define TCR_PS_BITS_4TB		0x3ULL
#define TCR_PS_BITS_16TB	0x4ULL
#define TCR_PS_BITS_256TB	0x5ULL

/*
 * PTE descriptor can be Block descriptor or Table descriptor
 * or Page descriptor.
 */
#define PTE_DESC_TYPE_MASK	3U
#define PTE_BLOCK_DESC		1U
#define PTE_TABLE_DESC		3U
#define PTE_PAGE_DESC		3U
#define PTE_INVALID_DESC	0U

/*
 * Block and Page descriptor attributes fields
 */
#define PTE_BLOCK_DESC_MEMTYPE(x)	(x << 2)
#define PTE_BLOCK_DESC_NS		(1ULL << 5)
#define PTE_BLOCK_DESC_AP_ELx		(1ULL << 6)
#define PTE_BLOCK_DESC_AP_EL_HIGHER	(0ULL << 6)
#define PTE_BLOCK_DESC_AP_RO		(1ULL << 7)
#define PTE_BLOCK_DESC_AP_RW		(0ULL << 7)
#define PTE_BLOCK_DESC_NON_SHARE	(0ULL << 8)
#define PTE_BLOCK_DESC_OUTER_SHARE	(2ULL << 8)
#define PTE_BLOCK_DESC_INNER_SHARE	(3ULL << 8)
#define PTE_BLOCK_DESC_AF		(1ULL << 10)
#define PTE_BLOCK_DESC_NG		(1ULL << 11)
#define PTE_BLOCK_DESC_PXN		(1ULL << 53)
#define PTE_BLOCK_DESC_UXN		(1ULL << 54)

struct arm_mmu_ptables {
	unsigned long *xlat_tables;
	unsigned int next_table;
};
/* Following Memory types supported through MAIR encodings can be passed
 * by user through "attrs"(attributes) field of specified memory region.
 * As MAIR supports such 8 encodings, we will reserve attrs[2:0];
 * so that we can provide encodings upto 7 if needed in future.
 */
#define MT_TYPE_MASK		0x7U
#define MT_TYPE(attr)		(attr & MT_TYPE_MASK)
#define MT_DEVICE_nGnRnE	0U
#define MT_DEVICE_nGnRE		1U
#define MT_DEVICE_GRE		2U
#define MT_NORMAL_NC		3U
#define MT_NORMAL		4U
#define MT_NORMAL_WT		5U

#define MEMORY_ATTRIBUTES	((0x00 << (MT_DEVICE_nGnRnE * 8)) |	\
				(0x04 << (MT_DEVICE_nGnRE * 8))   |	\
				(0x0c << (MT_DEVICE_GRE * 8))     |	\
				(0x44 << (MT_NORMAL_NC * 8))      |	\
				(0xffUL << (MT_NORMAL * 8))	  |	\
				(0xbbUL << (MT_NORMAL_WT * 8)))

/* More flags from user's perpective are supported using remaining bits
 * of "attrs" field, i.e. attrs[31:3], underlying code will take care
 * of setting PTE fields correctly.
 *
 * current usage of attrs[31:3] is:
 * attrs[3] : Access Permissions
 * attrs[4] : Memory access from secure/ns state
 * attrs[5] : Execute Permissions privileged mode (PXN)
 * attrs[6] : Execute Permissions unprivileged mode (UXN)
 * attrs[7] : Mirror RO/RW permissions to EL0
 *
 */
#define MT_PERM_SHIFT		3U
#define MT_SEC_SHIFT		4U
#define MT_P_EXECUTE_SHIFT	5U
#define MT_U_EXECUTE_SHIFT	6U
#define MT_RW_AP_SHIFT		7U

#define MT_RO			(0U << MT_PERM_SHIFT)
#define MT_RW			(1U << MT_PERM_SHIFT)

#define MT_RW_AP_ELx		(1U << MT_RW_AP_SHIFT)
#define MT_RW_AP_EL_HIGHER	(0U << MT_RW_AP_SHIFT)

#define MT_SECURE		(0U << MT_SEC_SHIFT)
#define MT_NS			(1U << MT_SEC_SHIFT)

#define MT_P_EXECUTE		(0U << MT_P_EXECUTE_SHIFT)
#define MT_P_EXECUTE_NEVER	(1U << MT_P_EXECUTE_SHIFT)

#define MT_U_EXECUTE		(0U << MT_U_EXECUTE_SHIFT)
#define MT_U_EXECUTE_NEVER	(1U << MT_U_EXECUTE_SHIFT)

#define MT_P_RW_U_RW		(MT_RW | MT_RW_AP_ELx | MT_P_EXECUTE_NEVER | MT_U_EXECUTE_NEVER)
#define MT_P_RW_U_NA		(MT_RW | MT_RW_AP_EL_HIGHER  | MT_P_EXECUTE_NEVER | MT_U_EXECUTE_NEVER)
#define MT_P_RO_U_RO		(MT_RO | MT_RW_AP_ELx | MT_P_EXECUTE_NEVER | MT_U_EXECUTE_NEVER)
#define MT_P_RO_U_NA		(MT_RO | MT_RW_AP_EL_HIGHER  | MT_P_EXECUTE_NEVER | MT_U_EXECUTE_NEVER)
#define MT_P_RO_U_RX		(MT_RO | MT_RW_AP_ELx | MT_P_EXECUTE_NEVER | MT_U_EXECUTE)
#define MT_P_RX_U_RX		(MT_RO | MT_RW_AP_ELx | MT_P_EXECUTE | MT_U_EXECUTE)
#define MT_P_RX_U_NA		(MT_RO | MT_RW_AP_EL_HIGHER  | MT_P_EXECUTE | MT_U_EXECUTE_NEVER)

#ifdef CONFIG_ARMV8_A_NS
#define MT_DEFAULT_SECURE_STATE	MT_NS
#else
#define MT_DEFAULT_SECURE_STATE	MT_SECURE
#endif

void add_map(const char *name,
		    unsigned long phys, unsigned long virt, int size, unsigned int attrs);
void enable_mmu();

#define GENMASK(h, l) \
	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (64 - 1 - (h))))

#endif
