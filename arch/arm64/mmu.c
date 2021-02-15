#include <arch.h>
#include <arch_help.h>
#include <mmu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

static u64 base_xlat_table[NUM_BASE_LEVEL_ENTRIES]
__aligned(0x1000);

static u64 xlat_tables[CONFIG_MAX_XLAT_TABLES][XLAT_TABLE_ENTRIES]
__aligned(0x1000);

/* Translation table control register settings */
static u64 get_tcr(int el)
{
	u64 tcr;
	u64 pa_bits = CONFIG_ARM64_PA_BITS;
	u64 va_bits = CONFIG_ARM64_VA_BITS;
	u64 tcr_ps_bits;

	switch (pa_bits) {
	case 48:
		tcr_ps_bits = TCR_PS_BITS_256TB;
		break;
	case 44:
		tcr_ps_bits = TCR_PS_BITS_16TB;
		break;
	case 42:
		tcr_ps_bits = TCR_PS_BITS_4TB;
		break;
	case 40:
		tcr_ps_bits = TCR_PS_BITS_1TB;
		break;
	case 36:
		tcr_ps_bits = TCR_PS_BITS_64GB;
		break;
	default:
		tcr_ps_bits = TCR_PS_BITS_4GB;
		break;
	}

	if (el == 1) {
		tcr = (tcr_ps_bits << TCR_EL1_IPS_SHIFT);
		/*
		 * TCR_EL1.EPD1: Disable translation table walk for addresses
		 * that are translated using TTBR1_el2.
		 */
		tcr |= TCR_EPD1_DISABLE;
	} else
		tcr = (tcr_ps_bits << TCR_EL2_PS_SHIFT);

	tcr |= TCR_T0SZ(va_bits);
	/*
	 * Translation table walk is cacheable, inner/outer WBWA and
	 * inner shareable
	 */
	tcr |= TCR_TG0_4K | TCR_SHARED_INNER | TCR_ORGN_WBWA | TCR_IRGN_WBWA;

	return tcr;
}

static int pte_desc_type(u64 *pte)
{
	return *pte & PTE_DESC_TYPE_MASK;
}

static u64 *calculate_pte_index(u64 addr, int level)
{
	int base_level = XLAT_TABLE_BASE_LEVEL;
	u64 *pte;
	u64 idx;
	unsigned int i;

	/* Walk through all translation tables to find pte index */
	pte = (u64 *)base_xlat_table;
	for (i = base_level; i <= CONFIG_MAX_XLAT_TABLES; i++) {
		idx = XLAT_TABLE_VA_IDX(addr, i);
		pte += idx;

		/* Found pte index */
		if (i == level)
			return pte;
		/* if PTE is not table desc, can't traverse */
		if (pte_desc_type(pte) != PTE_TABLE_DESC)
			return NULL;
		/* Move to the next translation table level */
		pte = (u64 *)(*pte & 0x0000fffffffff000ULL);
	}

	return NULL;
}

static void set_pte_table_desc(u64 *pte, u64 *table, unsigned int level)
{
#if DUMP_PTE
	printf("%s", XLAT_TABLE_LEVEL_SPACE(level));
	printf("%p: [Table] %p\n", pte, table);
#endif
	/* Point pte to new table */
	*pte = PTE_TABLE_DESC | (u64)table;
}

static void set_pte_block_desc(u64 *pte, u64 addr_pa, unsigned int attrs,
			       unsigned int level)
{
	u64 desc = addr_pa;
	unsigned int mem_type;

	desc |= (level == 3) ? PTE_PAGE_DESC : PTE_BLOCK_DESC;

	/* NS bit for security memory access from secure state */
	desc |= (attrs & MT_NS) ? PTE_BLOCK_DESC_NS : 0;

	/* AP bits for Data access permission */
	desc |= (attrs & MT_RW) ? PTE_BLOCK_DESC_AP_RW : PTE_BLOCK_DESC_AP_RO;

	/* the access flag */
	desc |= PTE_BLOCK_DESC_AF;

	/* memory attribute index field */
	mem_type = MT_TYPE(attrs);
	desc |= PTE_BLOCK_DESC_MEMTYPE(mem_type);

	switch (mem_type) {
	case MT_DEVICE_nGnRnE:
	case MT_DEVICE_nGnRE:
	case MT_DEVICE_GRE:
		/* Access to Device memory and non-cacheable memory are coherent
		 * for all observers in the system and are treated as
		 * Outer shareable, so, for these 2 types of memory,
		 * it is not strictly needed to set shareability field
		 */
		desc |= PTE_BLOCK_DESC_OUTER_SHARE;
		/* Map device memory as execute-never */
		desc |= PTE_BLOCK_DESC_PXN;
		desc |= PTE_BLOCK_DESC_UXN;
		break;
	case MT_NORMAL_NC:
	case MT_NORMAL:
		/* Make Normal RW memory as execute never */
		/* if ((attrs & MT_RW) || (attrs & MT_EXECUTE_NEVER)) {
			desc |= PTE_BLOCK_DESC_PXN;
			desc |= PTE_BLOCK_DESC_UXN;
		} */
		if (mem_type == MT_NORMAL)
			desc |= PTE_BLOCK_DESC_INNER_SHARE;
		else
			desc |= PTE_BLOCK_DESC_OUTER_SHARE;
	}

#if DUMP_PTE
	printf(" %s", XLAT_TABLE_LEVEL_SPACE(level));
	printf("%p: ", pte);
	printf((mem_type == MT_NORMAL) ?
			  "MEM" :
			  ((mem_type == MT_NORMAL_NC) ? "NC" : "DEV"));
	printf((attrs & MT_RW) ? "-RW" : "-RO");
	printf((attrs & MT_NS) ? "-NS" : "-S");
	printf((attrs & MT_P_EXECUTE_NEVER) ? "-XN" : "-EXEC");
	printf("\n");
#endif

	*pte = desc;
}

/* Returns a new reallocated table */
static u64 *new_prealloc_table(void)
{
	static unsigned int table_idx;
	return (u64 *)(xlat_tables[table_idx++]);
}

/* Splits a block into table with entries spanning the old block */
static void split_pte_block_desc(u64 *pte, int level)
{
	u64 old_block_desc = *pte;
	u64 *new_table;
	unsigned int i = 0;
	/* get address size shift bits for next level */
	int levelshift = LEVEL_TO_VA_SIZE_SHIFT(level + 1);

	printf("Splitting existing PTE %p(L%d)\n", pte, level);

	new_table = new_prealloc_table();

	for (i = 0; i < XLAT_TABLE_ENTRIES; i++) {
		new_table[i] = old_block_desc | (i << levelshift);

		if ((level + 1) == 3)
			new_table[i] |= PTE_PAGE_DESC;
	}

	/* Overwrite existing PTE set the new table into effect */
	set_pte_table_desc(pte, new_table, level);
}

/* Create/Populate translation table(s) for given region */
void add_map(const char *name,
                    unsigned long phys, unsigned long virt, int size, unsigned int attrs)

{
	u64 *pte;
	u64 level_size;
	u64 *new_table;
	unsigned int level = XLAT_TABLE_BASE_LEVEL;

	printf("mmap: virt %llx phys %llx size %llx\n", virt, phys, size);

	while (size) {
		/* Locate PTE for given virtual address and page table level */
		pte = calculate_pte_index(virt, level);

		level_size = 1ULL << LEVEL_TO_VA_SIZE_SHIFT(level);

		if (size >= level_size && !(virt & (level_size - 1))) {
			/* Given range fits into level size,
			 * create block/page descriptor
			 */
			set_pte_block_desc(pte, phys, attrs, level);
			virt += level_size;
			phys += level_size;
			size -= level_size;
			/* Range is mapped, start again for next range */
			level = XLAT_TABLE_BASE_LEVEL;
		} else if (pte_desc_type(pte) == PTE_INVALID_DESC) {
			/* Range doesn't fit, create subtable */
			new_table = new_prealloc_table();
			set_pte_table_desc(pte, new_table, level);
			level++;
		} else if (pte_desc_type(pte) == PTE_BLOCK_DESC) {
			split_pte_block_desc(pte, level);
			level++;
		} else if (pte_desc_type(pte) == PTE_TABLE_DESC) {
			level++;
		}
	}
}

void enable_mmu()
{
	u64 val;

	/* Set MAIR, TCR and TBBR registers */
	__asm__ volatile("msr mair_el2, %0"
			 :
			 : "r"(MEMORY_ATTRIBUTES)
			 : "memory", "cc");
	__asm__ volatile("msr tcr_el2, %0"
			 :
			 : "r"(get_tcr(2))
			 : "memory", "cc");
	__asm__ volatile("msr ttbr0_el2, %0"
			 :
			 : "r"((u64)base_xlat_table)
			 : "memory", "cc");

	/* Ensure these changes are seen before MMU is enabled */
	isb();

	/* Enable the MMU and data cache */
	__asm__ volatile("mrs %0, sctlr_el2" : "=r"(val));
	__asm__ volatile("msr sctlr_el2, %0"
			 :
			 : "r"(val | SCTLR_M | SCTLR_C)
			 : "memory", "cc");

	/* Ensure the MMU enable takes effect immediately */
	isb();

	printf("MMU enabled with dcache\n");
}
