#pragma once
#include <types.h>
#include <defs.h>
#include <arch/x86_64/regs.h>

#define EXPECTED_ELF_MACHINE_CODE    62

extern void syscall_entry();
extern void (*x86_vectors[IDT_NUM_ENTRIES])(void);

void init_segmentation();
void reload_segments(u16 new_ds, u16 new_cs);

void outb(u16 port, u8 val);
void outw(u16 port, u16 val);
u8   inb(u16 port);
u64  read_msr(u32 msr_idx);
void write_msr(u32 msr_idx, u64 val);
u64  read_cr2();
