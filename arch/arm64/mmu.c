#include <mmu.h>
#include <arch.h>
#include <types.h>

static uint64_t kernel_xlat_tables[CONFIG_MAX_XLAT_TABLES * Ln_XLAT_NUM_ENTRIES]
		__aligned(Ln_XLAT_NUM_ENTRIES * sizeof(uint64_t));

static struct arm_mmu_ptables kernel_ptables = {
	.xlat_tables = kernel_xlat_tables,
};

/* Translation table control register settings */
static unsigned long get_tcr(int el)
{
	unsigned long tcr;
	unsigned long va_bits = CONFIG_ARM64_VA_BITS;
	unsigned long tcr_ps_bits;

	tcr_ps_bits = TCR_PS_BITS;

	if (el == 1) {
		tcr = (tcr_ps_bits << TCR_EL1_IPS_SHIFT);
		/*
		 * TCR_EL1.EPD1: Disable translation table walk for addresses
		 * that are translated using TTBR1_EL1.
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

static int pte_desc_type(unsigned long *pte)
{
	return *pte & PTE_DESC_TYPE_MASK;
}

static unsigned long *calculate_pte_index(struct arm_mmu_ptables *ptables,
				     unsigned long addr, unsigned int level)
{
	int base_level = BASE_XLAT_LEVEL;
	unsigned long *pte;
	unsigned long idx;
	unsigned int i;

	/* Walk through all translation tables to find pte index */
	pte = (unsigned long *)ptables->xlat_tables;
	for (i = base_level; i < XLAT_LEVEL_MAX; i++) {
		idx = XLAT_TABLE_VA_IDX(addr, i);
		pte += idx;

		/* Found pte index */
		if (i == level)
			return pte;
		/* if PTE is not table desc, can't traverse */
		if (pte_desc_type(pte) != PTE_TABLE_DESC)
			return NULL;
		/* Move to the next translation table level */
		pte = (unsigned long *)(*pte & 0x0000fffffffff000ULL);
	}

	return NULL;
}

static void set_pte_table_desc(unsigned long *pte, unsigned long *table, unsigned int level)
{
#if DUMP_PTE
	MMU_DEBUG("%s", XLAT_TABLE_LEVEL_SPACE(level));
	MMU_DEBUG("%p: [Table] %p\n", pte, table);
#endif
	/* Point pte to new table */
	*pte = PTE_TABLE_DESC | (unsigned long)table;
}

static unsigned long get_region_desc(unsigned int attrs)
{
	unsigned int mem_type;
	unsigned long desc = 0;

	/* NS bit for security memory access from secure state */
	desc |= (attrs & MT_NS) ? PTE_BLOCK_DESC_NS : 0;

	/*
	 * AP bits for EL0 / ELh Data access permission
	 *
	 *   AP[2:1]   ELh  EL0
	 * +--------------------+
	 *     00      RW   NA
	 *     01      RW   RW
	 *     10      RO   NA
	 *     11      RO   RO
	 */

	/* AP bits for Data access permission */
	desc |= (attrs & MT_RW) ? PTE_BLOCK_DESC_AP_RW : PTE_BLOCK_DESC_AP_RO;

	/* Mirror permissions to EL0 */
	desc |= (attrs & MT_RW_AP_ELx) ?
		 PTE_BLOCK_DESC_AP_ELx : PTE_BLOCK_DESC_AP_EL_HIGHER;

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
		if ((attrs & MT_RW) || (attrs & MT_P_EXECUTE_NEVER))
			printf("test\n");
			//desc |= PTE_BLOCK_DESC_PXN;

		if (((attrs & MT_RW) && (attrs & MT_RW_AP_ELx)) ||
		     (attrs & MT_U_EXECUTE_NEVER))
			desc |= PTE_BLOCK_DESC_UXN;

		if (mem_type == MT_NORMAL)
			desc |= PTE_BLOCK_DESC_INNER_SHARE;
		else
			desc |= PTE_BLOCK_DESC_OUTER_SHARE;
	}

	return desc;
}

static unsigned long get_region_desc_from_pte(unsigned long *pte)
{
	return ((*pte) & DESC_ATTRS_MASK);
}

static void set_pte_block_desc(unsigned long *pte, unsigned long addr_pa,
			       unsigned long desc, unsigned int level)
{
	desc |= addr_pa;
	desc |= (level == 3) ? PTE_PAGE_DESC : PTE_BLOCK_DESC;

#if DUMP_PTE
	uint8_t mem_type = (desc >> 2) & MT_TYPE_MASK;

	MMU_DEBUG("%s", XLAT_TABLE_LEVEL_SPACE(level));
	MMU_DEBUG("%p: ", pte);
	MMU_DEBUG((mem_type == MT_NORMAL) ? "MEM" :
		  ((mem_type == MT_NORMAL_NC) ? "NC" : "DEV"));
	MMU_DEBUG((desc & PTE_BLOCK_DESC_AP_RO) ? "-RO" : "-RW");
	MMU_DEBUG((desc & PTE_BLOCK_DESC_NS) ? "-NS" : "-S");
	MMU_DEBUG((desc & PTE_BLOCK_DESC_AP_ELx) ? "-ELx" : "-ELh");
	MMU_DEBUG((desc & PTE_BLOCK_DESC_PXN) ? "-PXN" : "-PX");
	MMU_DEBUG((desc & PTE_BLOCK_DESC_UXN) ? "-UXN" : "-UX");
	MMU_DEBUG("\n");
#endif

	*pte = desc;
}

/* Returns a new reallocated table */
static unsigned long *new_prealloc_table(struct arm_mmu_ptables *ptables)
{
	ptables->next_table++;

	if(ptables->next_table >= CONFIG_MAX_XLAT_TABLES)
		printf("Enough xlat tables not allocated");

	return (unsigned long *)(&ptables->xlat_tables[ptables->next_table *
			    Ln_XLAT_NUM_ENTRIES]);
}

/* Splits a block into table with entries spanning the old block */
static void split_pte_block_desc(struct arm_mmu_ptables *ptables, unsigned long *pte,
				 unsigned long desc, unsigned int level)
{
	unsigned long old_block_desc = *pte;
	unsigned long *new_table;
	unsigned int i = 0;
	/* get address size shift bits for next level */
	unsigned int levelshift = LEVEL_TO_VA_SIZE_SHIFT(level + 1);

	MMU_DEBUG("Splitting existing PTE %p(L%d)\n", pte, level);

	new_table = new_prealloc_table(ptables);

	for (i = 0; i < Ln_XLAT_NUM_ENTRIES; i++) {
		new_table[i] = old_block_desc | (i << levelshift);

		if ((level + 1) == 3)
			new_table[i] |= PTE_PAGE_DESC;
	}

	/* Overwrite existing PTE set the new table into effect */
	set_pte_table_desc(pte, new_table, level);
}

void add_map(const char *name,
		    unsigned long phys, unsigned long virt, int size, unsigned int attrs)
{
	unsigned long desc, *pte;
	unsigned long level_size;
	unsigned long *new_table;
	unsigned int level = BASE_XLAT_LEVEL;
	struct arm_mmu_ptables *ptables = &kernel_ptables;

	MMU_DEBUG("mmap [%s]: virt %lx phys %lx size %lx\n",
		   name, virt, phys, size);

	/* check minimum alignment requirement for given mmap region */
	if(((virt & (CONFIG_MMU_PAGE_SIZE - 1)) != 0) ||
		 ((size & (CONFIG_MMU_PAGE_SIZE - 1)) != 0))
		 printf("address/size are not page aligned\n");

	desc = get_region_desc(attrs);

	while (size) {
		printf("level %d size %lx\n", level, size);
		if(level >= XLAT_LEVEL_MAX)
			 printf("max translation table level exceeded\n");

		/* Locate PTE for given virtual address and page table level */
		pte = calculate_pte_index(ptables, virt, level);
		if(pte == NULL)
			printf("pte not found\n");

		level_size = 1ULL << LEVEL_TO_VA_SIZE_SHIFT(level);

		if (size >= level_size && !(virt & (level_size - 1))) {
			/* Given range fits into level size,
			 * create block/page descriptor
			 */
			set_pte_block_desc(pte, phys, desc, level);
			virt += level_size;
			phys += level_size;
			size -= level_size;
			/* Range is mapped, start again for next range */
			level = BASE_XLAT_LEVEL;
		} else if (pte_desc_type(pte) == PTE_INVALID_DESC) {
			/* Range doesn't fit, create subtable */
			new_table = new_prealloc_table(ptables);
			set_pte_table_desc(pte, new_table, level);
			level++;
		} else if (pte_desc_type(pte) == PTE_BLOCK_DESC) {
			/* Check if the block is already mapped with the correct attrs */
			if (desc == get_region_desc_from_pte(pte))
				return;

			/* We need to split a new table */
			split_pte_block_desc(ptables, pte, desc, level);
			level++;
		} else if (pte_desc_type(pte) == PTE_TABLE_DESC)
			level++;
	}
	printf("end\n");
}

void enable_mmu()
{
	unsigned long val;

	/* Set MAIR, TCR and TBBR registers */
	__asm__ volatile("msr mair_el2, %0"
			:
			: "r" (MEMORY_ATTRIBUTES)
			: "memory", "cc");
	__asm__ volatile("msr tcr_el2, %0"
			:
			: "r" (get_tcr(1))
			: "memory", "cc");
	__asm__ volatile("msr ttbr0_el2, %0"
			:
			: "r" ((unsigned long)kernel_ptables.xlat_tables)
			: "memory", "cc");

	/* Ensure these changes are seen before MMU is enabled */
	__asm__ volatile("isb");
	/* Enable the MMU and data cache */
	__asm__ volatile("mrs %0, sctlr_el2" : "=r" (val));
	printf("enable %s %d\n", __func__, __LINE__);
	__asm__ volatile("msr sctlr_el2, %0"
			:
			: "r" (val | SCTLR_M | SCTLR_C)
			: "memory", "cc");
	printf("enable %s %d\n", __func__, __LINE__);
	/* Ensure the MMU enable takes effect immediately */
	__asm__ volatile("isb");

	MMU_DEBUG("MMU enabled with dcache\n");
}
