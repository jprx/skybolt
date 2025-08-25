#pragma once

#define REGS_SIZE          ((22 * 8))
#define KREGS_SIZE         ((7 * 8))

#define CR4_PAE_BIT        5
#define CR0_PG_BIT         31
#define EFER_LME_BIT       8
#define EFER_SCE_BIT       0

#define IDT_NUM_ENTRIES    256

#define MSR_EFER           0xC0000080
#define MSR_LSTAR          0xC0000082
#define MSR_STAR           0xC0000081
#define MSR_SFMASK         0xC0000084
#define MSR_FSBASE         0xC0000100
#define MSR_GSBASE         0xC0000101
#define MSR_KGSBASE        0xC0000102
#define MSR_PAT            0x00000277
#define MSR_APIC_BASE      0x0000001B

#define TSS_RSP0           4

#define USER_CS            0x2Bull
#define USER_DS            0x23ull
#define KERNEL_CS          0x08ull
#define KERNEL_DS          0x10ull

#define GS_SAVED_RCX       0
#define GS_SAVED_RSP       8

#define INTR_SYSCALL       1024

#define RFLAGS_IF          BIT(9)

#define PAT_UC             0x00
#define PAT_WC             0x01
#define PAT_WB             0x06

#define APIC_ENABLE        BIT(11)
