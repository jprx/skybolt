#pragma once
#define REGS_SIZE     ((36 * 8))
#define KREGS_SIZE    ((14 * 8))

#define CURRENTEL_EL0 0b00
#define CURRENTEL_EL1 0b01
#define CURRENTEL_EL2 0b10
#define CURRENTEL_EL3 0b11
#define CURRENTEL_SHIFT 2

// EL1 with SP_EL1
#define SPSR_EL2_MODE_EL1H ((0b0101))

#define SPSR_EL2_DISABLE_FIQ ((BIT(6)))
#define SPSR_EL2_DISABLE_IRQ ((BIT(7)))

// M = 0 (EL0), DAIF = 0 (all enabled)
#define SPSR_EL1_USER       ((0b0000))
#define SPSR_EL1_KERN       ((0b0101))
#define SPSR_GET_M(x)       (( (x) & MASK(4) ))

// reading from DAIF returns bits [9:6],
// whereas writing to DAIFSet/Clr uses bits [3:0]
#define DAIFSET_DISABLE_INTERRUPTS ((0xF))
#define DAIF_IRQ   BIT(6)
#define DAIF_FIQ   BIT(7)

#define HCR_EL2_RW BIT(31)

#define SCTLR_EL1_MMU         BIT(0)
#define SCTLR_EL1_ICACHE      BIT(12)
#define SCTLR_EL1_DCACHE      BIT(2)

#define CPACR_EL1_FPEN        ((3ull << 20))

// 4K paging
#define TCR_EL1_TG0        ((0b00 << 14ULL))
#define TCR_EL1_TG1        ((0b10 << 30ULL))

// We configure T0SZ and T1SZ to use a 47 bit virtual address, meaning both of
// them point to a top level page table with 256 entries. TTBR0 refers to all
// addresses where the upper 17 bits are 0, and TTBR1 refers to all addresses
// where the upper 17 bits are 1.
//
// We allocate one 4K (512 entry) page table and point TTBR0 to the 0th entry
// and TTBR1 to the 256th entry. This matches the X86 and RISCV behavior.
//
// TTBR0: The lower half exists where bits[63:47] are always 0:
//   0x0000000000000000         ->         0x00007FFFFFFFFFFF
//   [0]                                   [BIT(47) - 1]
//
// TTBR1: The upper half exists where bits [63:47] are always 1:
//   0xFFFF800000000000         ->         0xFFFFFFFFFFFFFFFF
//   [0 | (MASK(17) << 47)]                [(BIT(47) - 1) | (MASK(17) << 47)]
//
//                       ┌────────────────────────────────┐
// 0x000000000000000 ┌─  │                                │
//                   │   │                                │
//                   │   │                                │
//                   │   │                                │
//                   │   │                                │
//       ┌─────┐     │   │                                │
//       │TTBR0├─────┤   │   Page Table Entries [0:255]   │
//       └─────┘     │   │                                │
//                   │   │         (User Memory)          │
//                   │   │                                │
//                   │   │                                │
//                   │   │                                │
//                   │   │                                │
//                   │   │                                │
// 0x00007FFFFFFFFFF └─  │                                │
//                       ├────────────────────────────────┤
// 0xFFFF80000000000 ┌─  │                                │
//                   │   │                                │
//                   │   │                                │
//                   │   │                                │
//                   │   │                                │
//       ┌─────┐     │   │  Page Table Entries [256:511]  │
//       │TTBR1├─────┤   │                                │
//       └─────┘     │   │        (Kernel Memory)         │
//                   │   │                                │
//                   │   │                                │
//                   │   │                                │
//                   │   │                                │
//                   │   │                                │
//                   │   │                                │
// 0xFFFFFFFFFFFFFFF └─  │                                │
//                       └────────────────────────────────┘
#define TCR_EL1_T0SZ       ((17 << 0ULL))
#define TCR_EL1_T1SZ       ((17 << 16ULL))

// 52 bit PAs
#define TCR_EL1_IPS        ((0b110 << 32ULL))
#define TCR_EL1_HA         ((1 << 39ULL))

/* IRGN/ ORGN and SH0 setup cacheability and shareability for the page tables */
/* As we are single-core we want all of these on to be as cacheable as possible */
// IRGN == Inner cacheability attribute
// ORGN == Outer cacheability attribute
// 0b00: Non-cacheable
// 0b01: Write-back
// 0b10: Write-Through
// 0b11: Write-back, no write-allocate
#define TCR_IRGN ((0b01))
#define TCR_ORGN ((0b01))
#define TCR_ORGN0 ((TCR_ORGN << 10ULL))
#define TCR_ORGN1 ((TCR_ORGN << 26ULL))
#define TCR_IRGN0 ((TCR_IRGN << 8ULL))
#define TCR_IRGN1 ((TCR_IRGN << 24ULL))

// SH0/ SH1 inner shared
#define TCR_SH0 ((0b11 << 12ULL))
#define TCR_SH1 ((0b11 << 28ULL))

#define TCR_DEFAULT_EL1 ((TCR_EL1_TG0 | TCR_EL1_TG1 | TCR_EL1_T0SZ | TCR_EL1_T1SZ | TCR_EL1_IPS | TCR_EL1_HA | TCR_IRGN0 | TCR_ORGN0 | TCR_IRGN1 | TCR_ORGN1 | TCR_SH0 | TCR_SH1))

// Everything is device memory nGnRnE during bringup
#define MAIR_BRINGUP 0x0000000000000000ULL

// MAIR0: normal memory, everything else is device nGnRnE
#define MAIR_RUNTIME 0x00000000000000FFULL

#define ESR_REASON(x) (( ((x) >> 26) & MASK(6) ))

#define ESR_SVC64                     0b010101
#define ESR_INST_ABORT_LOWLEVEL       0b100000
#define ESR_INST_ABORT_NOCHANGE       0b100001
#define ESR_DATA_ABORT_LOWLEVEL       0b100100
#define ESR_DATA_ABORT_NOCHANGE       0b100101
