CFLAGS = $(cflags)

SOURCE_ROOT = .

CC = clang
AS = llvm-as
LD = ld.lld
OBJCOPY = llvm-objcopy
OBJDUMP = llvm-objdump
READELF = llvm-readelf

BUILD = $(SOURCE_ROOT)/build

include source.mk

ALL_SRCS = $(KERNEL_SRCS) $(PLATFORM_SRCS) $(LIBC_SRCS) $(DRIVER_SRCS) $(TEST_SRCS)

C_SRCS   = $(filter %.c, $(ALL_SRCS))
ASM_SRCS = $(filter %.S, $(ALL_SRCS))
H_SRCS   = $(wildcard $(INCLUDE_DIR)/*.h)

C_OBJS   = $(addprefix $(BUILD)/, $(patsubst %.c,%.o,$(C_SRCS)))
ASM_OBJS = $(addprefix $(BUILD)/, $(patsubst %.S,%.o,$(ASM_SRCS)))

ALL_OBJS = $(C_OBJS) $(ASM_OBJS)
#$(warning ALL_SRCS $(ALL_SRCS))
#$(warning ALL_OBJS $(ALL_OBJS))

#$(warning C_SRCS $(C_SRCS) ASM_SRCS $(ASM_SRCS))
#$(warning C_OBJS $(C_OBJS) ASM_OBJS $(ASM_OBJS))


OBJ_PATHS = $(addprefix $(BUILD)/, $(sort $(dir $(ALL_SRCS))))
#$(warning OBJ_PATHS $(OBJ_PATHS))


TARGET = Kopernik
TARGET_ELF = $(BUILD)/$(TARGET).elf
TARGET_IMG = $(BUILD)/$(TARGET).bin
TARGET_MAP = $(BUILD)/$(TARGET).map
TARGET_ASM = $(BUILD)/$(TARGET).asm

LDS = $(SOURCE_ROOT)/$(TARGET).ld

CFLAGS  += -nostdlib -mgeneral-regs-only -funwind-tables \
	   --target=-aarch64 \
	   -g \
	   -Wmissing-include-dirs \
	   -nostdinc \
	   -fno-builtin -Wall -O3 -g $(foreach dir, $(INCLUDE_DIR), -I$(dir))
ASFLAGS = $(CFLAGS) -D__ASM__

LDFLAGS = -T $(LDS) -Map $(TARGET_MAP)

.PHONY: build_all clean tags

build_all: all

$(C_OBJS): $(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ASM_OBJS): $(BUILD)/%.o: %.S
	$(CC) $(ASFLAGS) -c $< -o $@

build_objs: $(C_OBJS) $(ASM_OBJS)

init:
	@mkdir -p build
	@$(foreach d,$(OBJ_PATHS), mkdir -p $(d);)

all: init build_objs
	@echo "   make ..."
	$(LD) $(ALL_OBJS) $(LDFLAGS) -o $(TARGET_ELF)
	$(OBJDUMP) -D $(TARGET_ELF) > $(TARGET_ASM)
	$(OBJCOPY) $(TARGET_ELF) -O binary $(TARGET_IMG)
	cp $(TARGET_IMG) $(TARGET).bin
	cp $(TARGET_ELF) $(TARGET).elf

tags:
	@echo "  create ctags"

count:
	@echo "  counting sizes"
	find . | egrep ".*\.[ch]$$" | xargs wc -l


clean:
	@echo "  clean ..."
	@-rm -rf build
	@-rm -rf $(TARGET).elf
	@-rm -rf $(TARGET)*.bin
