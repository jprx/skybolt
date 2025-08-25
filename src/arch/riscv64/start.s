.section .text.start

#include <defs.h>

.global bootup

bootup:
  // Disable interrupts
  csrw mie, zero

  // Put any non-boot cores to sleep- we're single core (for now)
  csrr t0, mhartid
  bne t0, zero, non_boot_core

  // Disable all physical memory protection
  li t0, PMP_DISABLE_ALL
  li t1, PMP_ADDR_ALL
  csrw pmpcfg0, t0
  csrw pmpaddr0, t1

  // Route interrupts from M mode to S mode
  li t0, INT_DELEGATE_ALL
  csrw medeleg, t0
  csrw mideleg, t0

  // Enable supervisor mode timer
  // (See RISCV Privileged Spec Section 19: Sstc Extension)
  // set menvcfg.stce = 1
  csrr t0, menvcfg
  li t1, MENVCFG_STCE
  or t0, t0, t1
  csrw menvcfg, t0

  // set mcounteren.tm = 1
  csrr t0, mcounteren
  li t1, MCOUNTEREN_TM
  or t0, t0, t1
  csrw mcounteren, t0

  // Enable bringup page table
  la t0, bringup_pagetable
  srli t0, t0, SATP_PPN_PAGE_SHIFT
  li t1, RISCV_PAGING_MODE_SV48 << RISCV_PAGING_MODE_SHIFT
  or t0, t0, t1
  csrw satp, t0
  sfence.vma zero

  // Switch from M mode -> S mode
  la t0, enter_s_mode
  csrw mepc, t0
  csrr t0, mstatus
  li t1, RISCV_MODE_S << MSTATUS_MPP_SHIFT
  or t0, t0, t1
  csrw mstatus, t0
  mret

enter_s_mode:
  // Setup higher half stack
  la t0, bringup_stack_top
  li t1, KERNEL_LINK_ADDRESS
  add sp, t1, t0

  // Jump to higher half of the address space
  // Can't just la start_higher_half because it's too far away
  // Instead, we use the _higher_half_load_addr load memory address from the linker,
  // and manually add the higher half offset to it.
  la t0, _higher_half_load_addr
  li t1, KERNEL_LINK_ADDRESS
  add t0, t1, t0
  jr t0

non_boot_core:
  j .

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
  .quad 0xCF
  .endr

.section .text.start64

.global start_higher_half
start_higher_half:
  call kmain
  wfi
