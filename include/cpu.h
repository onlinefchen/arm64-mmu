#ifndef __CPU_H__
#define __CPU_H__

#ifndef __ASM__
/* top level view of a cpu data */
struct cpu {
	int id;
	void *stack;
};
#endif

#define CPU_ID_OFFSET                0
#define CPU_STACK_OFFSET             8

#endif
