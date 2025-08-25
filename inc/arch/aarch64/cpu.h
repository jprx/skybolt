#pragma once
#include <types.h>
#include <arch/aarch64/regs.h>

#define EXPECTED_ELF_MACHINE_CODE    183

#define read_msr(r) ({ u64 ___tmp_msr_val; \
    asm volatile ("mrs %0, " #r : "=r"(___tmp_msr_val)); \
    ___tmp_msr_val; })

#define write_msr(r,v) ({asm volatile("msr " #r ", %0" :: "r"(v));})

void arm_timer_init();
void arm_timer_schedule();

extern u64 exception_table;
