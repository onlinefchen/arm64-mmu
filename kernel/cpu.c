/*
 * multi core stack define
 */
#include <sizes.h>
#include <cpu.h>

#define STACK_SIZE    SZ_16K
#define CORE_NUM      8

unsigned char cpu_stack[STACK_SIZE * CORE_NUM] __attribute__((__aligned__(64)));
