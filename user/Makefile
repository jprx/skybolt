# Skybolt user space
# This compiles the Skybolt user programs and builds a filesystem

ARCH ?= X86_64

ALL_PROGS := \
  hello      \
  count      \
  ls         \
  cat        \
  uname      \
  argtest    \
  shell

ALL_FILES := \
  skybolt.h          \
  shell.c            \
  files/small.txt    \
  files/medium.txt   \
  files/large.txt

TOUPPER=$(shell echo '$1' | tr '[:lower:]' '[:upper:]')
TOLOWER=$(shell echo '$1' | tr '[:upper:]' '[:lower:]')

ARCH_LOWER := $(call TOLOWER,$(ARCH))

FS_TAR  := fs.tar

OUTDIR := bin/$(ARCH_LOWER)
ALL_PROGS_$(ARCH) = $(addprefix $(OUTDIR)/,$(ALL_PROGS))
ALL_FILES_$(ARCH) = $(addprefix $(OUTDIR)/,$(notdir $(ALL_FILES)))

TOOLCHAIN_PREFIX := $(ARCH_LOWER)-elf-
AS               := $(TOOLCHAIN_PREFIX)gcc
CC               := $(TOOLCHAIN_PREFIX)gcc
LD               := $(TOOLCHAIN_PREFIX)gcc
OBJDUMP          := $(TOOLCHAIN_PREFIX)objdump
OBJCOPY          := $(TOOLCHAIN_PREFIX)objcopy
STRINGS          := $(TOOLCHAIN_PREFIX)strings
MKDIR            := mkdir

.PHONY: all
all: $(addprefix $(OUTDIR)/,$(FS_TAR))

$(OUTDIR)/$(FS_TAR) : $(ALL_PROGS_$(ARCH)) $(ALL_FILES_$(ARCH))
	cd $(OUTDIR); tar --format=ustar -cf $(FS_TAR) $(ALL_PROGS) $(notdir $(ALL_FILES_$(ARCH)))

$(OUTDIR)/%: %
	cp $< $@

$(OUTDIR)/%: files/%
	cp $< $@

$(OUTDIR)/%: %.c libc/libc.c libc/crt0_$(ARCH_LOWER).s Makefile libc/user.ld
	@$(MKDIR) -p $(dir $@)
	$(CC) -g -ffreestanding -nostdlib -I. -Wl,--no-warn-rwx-segments -T libc/user.ld libc/libc.c libc/crt0_$(ARCH_LOWER).s $< -o $@

.PHONY: clean
clean:
	rm -rf bin
