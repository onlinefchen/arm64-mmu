# define compile source file

INCLUDE_DIR += include \
	       include/libc \
	       include/libc/aarch64 \
	       arch/arm64/include \
	       plat/include

KERNEL_SRCS += arch/arm64/entry.S \
	       arch/arm64/spinlock.S \
	       arch/arm64/exception.S \
	       arch/arm64/mmu.c \
	       kernel/cpu.c \
	       kernel/handle.c \
	       kernel/init.c


PLATFORM_SRCS +=

LIBC_SRCS +=  \
	     lib/libc/memchr.c \
	     lib/libc/memcmp.c \
	     lib/libc/memcpy.c \
	     lib/libc/memmove.c \
	     lib/libc/memrchr.c \
	     lib/libc/memset.c \
	     lib/libc/printf.c \
	     lib/libc/putchar.c \
	     lib/libc/puts.c \
	     lib/libc/snprintf.c \
	     lib/libc/strchr.c \
	     lib/libc/strcmp.c \
	     lib/libc/strlcpy.c \
	     lib/libc/strlen.c \
	     lib/libc/strncmp.c \
	     lib/libc/strnlen.c \
	     lib/libc/strrchr.c

DRIVER_SRCS += driver/uart/pl011.S \
	       driver/uart/uart.c

TEST_SRCS +=
