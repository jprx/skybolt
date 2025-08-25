#pragma once
#include <arch/riscv64/regs.h>

#define EXPECTED_ELF_MACHINE_CODE    243

#define read_csr(r) ({ u64 ___tmp_csr_val; \
    asm volatile ("csrr %0, " #r : "=r"(___tmp_csr_val)); \
    ___tmp_csr_val; })

#define write_csr(r,v) ({asm volatile("csrw " #r ", %0" :: "rK"(v));})

void riscv_trap();
void riscv_timer_schedule();
