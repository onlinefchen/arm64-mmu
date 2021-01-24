#ifndef __MSR__

/**
 * Macros to stringify a parameter, and to allow the results of a macro to be
 * stringified in turn.
 */
#define str_(s) #s
#define str(s) str_(s)

/**
 * Reads a system register, supported by the current assembler, and returns the
 * result.
 */
#define read_msr(name)                                              \
        __extension__({                                             \
                unsigned long __v;                                      \
                __asm__ volatile("mrs %0, " str(name) : "=r"(__v)); \
                __v;                                                \
        })

/**
 * Writes the value to the system register, supported by the current assembler.
 */
#define write_msr(name, value)                                \
        __extension__({                                       \
                __asm__ volatile("msr " str(name) ", %x0"     \
                                 :                            \
                                 : "rZ"((unsigned long)(value))); \
        })

#endif
