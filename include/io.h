#ifndef __IO__
#define __IO__
#include <types.h>

#define IMPORT_SYM(type, sym, name) \
	extern char sym[];\
	static const __attribute__((unused)) type name = (type) sym;

static inline void write_8(uintptr_t addr, uint8_t value)
{
	*(volatile uint8_t*)addr = value;
}

static inline uint8_t read_8(uintptr_t addr)
{
	return *(volatile uint8_t*)addr;
}

static inline void write_16(uintptr_t addr, uint16_t value)
{
	*(volatile uint16_t*)addr = value;
}

static inline uint16_t read_16(uintptr_t addr)
{
	return *(volatile uint16_t*)addr;
}

static inline void clrsetbits_16(uintptr_t addr,
				uint16_t clear,
				uint16_t set)
{
	write_16(addr, (read_16(addr) & ~clear) | set);
}

static inline void write_32(uintptr_t addr, uint32_t value)
{
	*(volatile uint32_t*)addr = value;
}

static inline uint32_t read_32(uintptr_t addr)
{
	return *(volatile uint32_t*)addr;
}

static inline void write_64(uintptr_t addr, uint64_t value)
{
	*(volatile uint64_t*)addr = value;
}

static inline uint64_t read_64(uintptr_t addr)
{
	return *(volatile uint64_t*)addr;
}

static inline void clrbits_32(uintptr_t addr, uint32_t clear)
{
	write_32(addr, read_32(addr) & ~clear);
}

static inline void setbits_32(uintptr_t addr, uint32_t set)
{
	write_32(addr, read_32(addr) | set);
}

static inline void clrsetbits_32(uintptr_t addr,
				uint32_t clear,
				uint32_t set)
{
	write_32(addr, (read_32(addr) & ~clear) | set);
}

#endif /* H */
