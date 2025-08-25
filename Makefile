# Skybolt Operating System
# Created by Joseph Ravichandran (@0xjprx)
# https://github.com/jprx/skybolt

KERNEL_NAME := skybolt

# Build options
# PLATFORM: Which platform do you want to compile for?
# CONFIG: DEBUG or RELEASE
PLATFORM    ?= AARCH64
CONFIG      ?= RELEASE

# Convert a variable to upper-case or lower-case
TOUPPER=$(shell echo '$1' | tr '[:lower:]' '[:upper:]')
TOLOWER=$(shell echo '$1' | tr '[:upper:]' '[:lower:]')

# Setup default platforms for the generic ISA names
ifeq ($(PLATFORM),X86_64)
override PLATFORM=PC
endif

ifeq ($(PLATFORM),AARCH64)
override PLATFORM=RASPI
endif

ifeq ($(PLATFORM),RISCV64)
override PLATFORM=VIRT_RISCV
endif

# Mapping from platform name -> ISA name
ARCH_FOR_PC              := x86_64
ARCH_FOR_RASPI           := aarch64
ARCH_FOR_VIRT_RISCV      := riscv64
# todo: add more platforms here!

ARCH := $(ARCH_FOR_$(PLATFORM))

ifeq ($(PLATFORM),)
$(error PLATFORM is undefined)
endif

ifeq ($(ARCH),)
$(error Unknown platform type "$(PLATFORM)")
endif

# Define source files to use when building the kernel.
# SRC_COMMON is shared for all ISAs, and each ISA can
# also define specific per-ISA files (eg. for per-platform drivers)
SRC_COMMON := \
  src/platform/$(call TOLOWER,$(PLATFORM)).c                 \
  src/arch/$(ARCH)/start.s                                   \
  src/arch/$(ARCH)/cswitch.s                                 \
  src/arch/$(ARCH)/trap_entry.s                              \
  src/arch/$(ARCH)/trap.c                                    \
  src/arch/$(ARCH)/cpu.c                                     \
  src/arch/$(ARCH)/pte.c                                     \
  src/fs/tar.c                                               \
  src/graphics/font.c                                        \
  src/graphics/display.c                                     \
  src/graphics/vuart.c                                       \
  src/kernel/kmain.c                                         \
  src/kernel/task.c                                          \
  src/kernel/arg.c                                           \
  src/kernel/thread.c                                        \
  src/kernel/scheduler.c                                     \
  src/kernel/sleep.c                                         \
  src/kernel/file.c                                          \
  src/kernel/lib.c                                           \
  src/kernel/kheap.c                                         \
  src/kernel/printf.c                                        \
  src/kernel/tty.c                                           \
  src/kernel/serial.c                                        \
  src/kernel/interrupt.c                                     \
  src/kernel/syscall.c                                       \
  src/kernel/exception.c                                     \
  src/kernel/copyio.c                                        \
  src/kernel/stdio.c                                         \
  src/kernel/elf.c                                           \
  src/kernel/vm.c

SRC_x86_64  := $(SRC_COMMON)                                 \
  src/arch/$(ARCH)/segment.c                                 \
  src/io/keyboard/ps2_controller.c                           \
  src/io/keyboard/ps2_keyboard.c                             \
  src/io/serial/uart16550.c                                  \
  src/io/interrupt/apic.c

SRC_aarch64 := $(SRC_COMMON)                                 \
  src/io/serial/pl011.c                                      \
  src/io/interrupt/gic.c

SRC_riscv64 := $(SRC_COMMON)                                 \
  src/io/serial/uart16550.c                                  \
  src/io/interrupt/plic.c

LINKER_SCRIPT := src/link/$(call TOLOWER,$(PLATFORM)).ld

ifeq ($(LINKER_SCRIPT),)
$(error No linker script defined)
endif

# Setup our toolchain; we use gcc as our assembler, compiler, and linker
TOOLCHAIN_PREFIX := $(ARCH)-elf-
AS               := $(TOOLCHAIN_PREFIX)gcc
CC               := $(TOOLCHAIN_PREFIX)gcc
LD               := $(TOOLCHAIN_PREFIX)gcc
OBJDUMP          := $(TOOLCHAIN_PREFIX)objdump
OBJCOPY          := $(TOOLCHAIN_PREFIX)objcopy
STRINGS          := $(TOOLCHAIN_PREFIX)strings
MKDIR            := mkdir

CFLAGS_DEFS      := $(call TOUPPER,-DCONFIG_$(CONFIG) -DARCH_$(ARCH) -DPLATFORM_$(PLATFORM))

CFLAGS_DEBUG     := -O0 -g
CFLAGS_RELEASE   := -O2 -g

CFLAGS_x86_64    := -mno-red-zone -masm=intel -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-ssse3 -msoft-float
CFLAGS_aarch64   := -mgeneral-regs-only -mstrict-align
CFLAGS_riscv64   :=

WARN             := -Werror -Wall
INCFLAGS         := -Iinc
CFLAGS           := $(CFLAGS_$(CONFIG)) $(CFLAGS_$(ARCH)) $(CFLAGS_DEFS) $(INCFLAGS) $(WARN) -nostdlib -ffreestanding -MMD -mcmodel=large
ASFLAGS          := $(CFLAGS) -x assembler-with-cpp
LDFLAGS1         := -nostdlib -ffreestanding
LDFLAGS2         := -static-libgcc -lgcc -Wl,--no-warn-rwx-segments

# Where to save our output files
OUTDIR := $(call TOLOWER,bin/$(PLATFORM)/$(CONFIG))
OUTBIN := $(call TOLOWER,bin/$(KERNEL_NAME).$(ARCH).$(CONFIG).$(PLATFORM))

USERFS := user/bin/$(ARCH)/fs.tar

ASM_SRCS := $(filter %.s,$(SRC_$(ARCH)))
C_SRCS   := $(filter %.c,$(SRC_$(ARCH)))

ASM_OBJS := $(patsubst src/%.s,$(OUTDIR)/%.o,$(ASM_SRCS))
C_OBJS   := $(patsubst src/%.c,$(OUTDIR)/%.o,$(C_SRCS))
OBJS     := $(ASM_OBJS) $(C_OBJS)

.PHONY: all
all: $(OUTBIN) $(USERFS)

$(OUTDIR)/%.o: src/%.s
	@$(MKDIR) -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

$(OUTDIR)/%.o: src/%.c
	@$(MKDIR) -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTBIN): $(OBJS) $(LINKER_SCRIPT)
	@$(MKDIR) -p $(dir $@)
	$(LD) $(LDFLAGS1) $(OBJS) -T $(LINKER_SCRIPT) -o $@ $(LDFLAGS2)

.PHONY: $(USERFS)
$(USERFS):
	@make -C user ARCH=$(ARCH)

# Qemu requires 32 bit ELFs, so convert our 64 bit one -> 32 bit.
# We assume we start out in 32 bit mode on X86_64 platforms anyways.
ifeq ($(ARCH),x86_64)
all: $(OUTBIN).qemu
$(OUTBIN).qemu: $(OUTBIN)
	@$(MKDIR) -p $(dir $@)
	$(OBJCOPY) -O elf32-i386 $(OUTBIN) $(OUTBIN).qemu

all: bin/$(KERNEL_NAME).iso $(OUTBIN).qemu
bin/$(KERNEL_NAME).iso: $(OUTBIN) $(USERFS)
	@$(MKDIR) -p bin/grubfs/boot/grub
	@cp src/platform/grub.cfg bin/grubfs/boot/grub/grub.cfg
	@cp $(OUTBIN) bin/grubfs/$(KERNEL_NAME)
	@cp $(USERFS) bin/grubfs/fs.img
	@grub-mkrescue -o bin/$(KERNEL_NAME).iso bin/grubfs
endif

# Raspberry Pi requires a bare metal image, not an ELF,
# even when running Qemu with -kernel. (-kernel will work with an ELF,
# but it'll load you in EL3 instead of EL2).
ifeq ($(PLATFORM),RASPI)
all: $(OUTBIN).img
$(OUTBIN).img: $(OUTBIN)
	@$(MKDIR) -p $(dir $@)
	$(OBJCOPY) -O binary $(OUTBIN) $(OUTBIN).img
endif

.PHONY: clean
clean:
	rm -rf bin
	make -C user clean

-include $(OBJS:%.o=%.d)
