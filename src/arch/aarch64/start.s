.section .text.start

#include <defs.h>

.global bootup

// MOV64(r, imm)- mov 64-bit immediate imm into register r
.macro MOV64 r, imm
    movz \r, #(((\imm) >> 48) & 0x0FFFF), lsl #48
    movk \r, #(((\imm) >> 32) & 0x0FFFF), lsl #32
    movk \r, #(((\imm) >> 16) & 0x0FFFF), lsl #16
    movk \r, #(((\imm) >> 00) & 0x0FFFF), lsl #00
.endm

// Entrypoint to the kernel:
bootup:
  msr daifset, DAIFSET_DISABLE_INTERRUPTS

  // Put any non-boot cores to sleep- we're single core (for now)
  mrs x0, mpidr_el1
  and x0, x0, #0x0FF
  cmp x0, xzr
  bne non_boot_core

  // If we're in EL1, don't do anything.
  // If we're in EL2, switch to EL1.
  // If we're in EL3, which we shouldn't be, jump to an infinite loop.
  mrs x0, CurrentEL
  cmp x0, CURRENTEL_EL3 << CURRENTEL_SHIFT
  beq panic_el3

  cmp x0, CURRENTEL_EL1 << CURRENTEL_SHIFT
  beq bootup_el1

switch_el2_to_el1:
  msr sctlr_el1, xzr
  msr hcr_el2, xzr

  // set hcr_el2.rw to 1 for 64 bit mode
  mrs x0, hcr_el2
  orr x0, x0, HCR_EL2_RW
  msr hcr_el2, x0

  // use ERET to switch modes
  adr x0, bootup_el1
  msr elr_el2, x0
  MOV64 x0, SPSR_EL2_MODE_EL1H | SPSR_EL2_DISABLE_FIQ | SPSR_EL2_DISABLE_IRQ
  msr spsr_el2, x0
  eret

bootup_el1:
  adr x0, bringup_pagetable
  msr ttbr0_el1, x0
  msr ttbr1_el1, x0

  MOV64 x0, CPACR_EL1_FPEN
  msr cpacr_el1, x0

  MOV64 x0, MAIR_BRINGUP
  msr mair_el1, x0

  MOV64 x0, TCR_DEFAULT_EL1
  msr tcr_el1, x0
  isb

  // Turn on paging
  mrs x0, sctlr_el1
  orr x0, x0, SCTLR_EL1_ICACHE
  orr x0, x0, SCTLR_EL1_DCACHE
  orr x0, x0, SCTLR_EL1_MMU
  msr sctlr_el1, x0
  isb

  // Flush TLB
  dsb ishst
  tlbi vmalle1
  dsb ish
  isb

  // Setup higher half stack
  adrp x0, bringup_stack_top
  MOV64 x1, KERNEL_LINK_ADDRESS
  add x0, x0, x1
  mov sp, x0

  // Jump to higher half of the address space
  adrp x0, _higher_half_load_addr
  MOV64 x1, KERNEL_LINK_ADDRESS
  add x0, x0, x1
  br x0

panic_el3:
  b .

non_boot_core:
  b .

.balign 8
bringup_stack:
  .rept 2048
  .quad 0
  .endr
bringup_stack_top:
  .quad 0

.balign SMALL_PAGE_SIZE
bringup_pagetable:
  .rept 512
  .quad (bringup_pgtable_l3 + 0x403)
  .endr

.balign SMALL_PAGE_SIZE
bringup_pgtable_l3:
  .rept 512 / 4
  .quad 0x00000401
  .quad 0x40000401
  .quad 0x80000401
  .quad 0xC0000401
  .endr

.section .text.start64

.global start_higher_half
start_higher_half:
  bl kmain
  wfe
