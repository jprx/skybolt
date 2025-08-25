#pragma once
#define REGS_SIZE     ((35 * 8))
#define KREGS_SIZE    ((13 * 8))

/* mstatus fields */
#define RISCV_MODE_M           ((0b11))
#define RISCV_MODE_S           ((0b01))
#define RISCV_MODE_U           ((0b00))
#define MSTATUS_MPP_SHIFT   ((11))
#define MSTATUS_UXLEN_SHIFT ((32ull))
#define MSTATUS_SXLEN_SHIFT ((34ull))

/* Paging configuration (mode field of satp) */
#define RISCV_PAGING_MODE_DISABLED ((0ull))
#define RISCV_PAGING_MODE_SV39     ((8ull))
#define RISCV_PAGING_MODE_SV48     ((9ull))
#define RISCV_PAGING_MODE_SV57     ((10ull))
#define RISCV_PAGING_MODE_SV64     ((11ull))
#define RISCV_PAGING_MODE_SHIFT    ((60ull))

/* How many bits do we need to shift a physical address by
 * to get something we can shove in satp? */
#define SATP_PPN_PAGE_SHIFT ((12ull))

/* Physical memory protection (pmp) configuration bits */
#define PMP_R_BIT       ((0b001))
#define PMP_W_BIT       ((0b010))
#define PMP_X_BIT       ((0b100))
#define PMP_A_SHIFT     ((3))
#define PMP_A_OFF_VAL   ((0b00))
#define PMP_A_NAPOT_VAL ((0b11))
#define PMP_A_OFF       ((PMP_A_OFF_VAL << PMP_A_SHIFT))
#define PMP_A_NAPOT     ((PMP_A_NAPOT_VAL << PMP_A_SHIFT))

/* Disable all PMP protections (by enabling all kinds of memory accesses) */
#define PMP_DISABLE_ALL      ((PMP_R_BIT | PMP_W_BIT | PMP_X_BIT | PMP_A_NAPOT))

/* All 1s means all of memory when NAPOT is selected */
#define PMP_ADDR_ALL        ((-1))

/* Set every bit in medeleg and mideleg */
#define INT_DELEGATE_ALL    ((-1))

/* Interrupt bits: see RISCV Privileged Spec Table 14 in Section 3.1 */
/* These are the fields we care about in mie, mip, sie, and sip */
#define INT_SSOFTWARE       ((BIT(1)))
#define INT_STIMER          ((BIT(5)))
#define INT_SEXT            ((BIT(9)))

/* STCE: STimecmp Enable */
/* This turns on the Sstc extension, giving us a timer in S mode */
#define MENVCFG_STCE        ((BIT(63)))

/* This enables the time CSR as well as allowing S mode to access stimecmp */
#define MCOUNTEREN_TM       ((BIT(1)))

#define SSTATUS_SIE         ((BIT(1)))

#define SSTATUS_GET_SPP(x)  (( ((x) >> 8) & MASK(1) ))

// Allow supervisor access to user memory
#define SSTATUS_SUM         BIT(18)
#define SSTATUS_UXL         ((2ull << 32ull))

#define RISCV_INST_LEN      4
