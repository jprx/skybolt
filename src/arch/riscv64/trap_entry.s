.section .text

#include <defs.h>

.global riscv_trap
.global jump_to_user

#define REG_FIELD(i) (8*i)(sp)

.balign 0x1000
riscv_trap:
  // While running user code, sscratch holds the kernel stack base.
  // While running kernel code, sscratch holds 0.
  csrrw sp, sscratch, sp
  bne zero, sp, 1f
  csrr sp, sscratch

1:
  // Now, sscratch is whatever sp used to be before the trap,
  // and sp points to a valid kernel stack for us to use.
  add sp, sp, -(REGS_SIZE)
  sd x1, REG_FIELD(0)
  // skip x2 (sp) for now
  sd x3, REG_FIELD(2)
  sd x4, REG_FIELD(3)
  sd x5, REG_FIELD(4)
  sd x6, REG_FIELD(5)
  sd x7, REG_FIELD(6)
  sd x8, REG_FIELD(7)
  sd x9, REG_FIELD(8)
  sd x10, REG_FIELD(9)
  sd x11, REG_FIELD(10)
  sd x12, REG_FIELD(11)
  sd x13, REG_FIELD(12)
  sd x14, REG_FIELD(13)
  sd x15, REG_FIELD(14)
  sd x16, REG_FIELD(15)
  sd x17, REG_FIELD(16)
  sd x18, REG_FIELD(17)
  sd x19, REG_FIELD(18)
  sd x20, REG_FIELD(19)
  sd x21, REG_FIELD(20)
  sd x22, REG_FIELD(21)
  sd x23, REG_FIELD(22)
  sd x24, REG_FIELD(23)
  sd x25, REG_FIELD(24)
  sd x26, REG_FIELD(25)
  sd x27, REG_FIELD(26)
  sd x28, REG_FIELD(27)
  sd x29, REG_FIELD(28)
  sd x30, REG_FIELD(29)
  sd x31, REG_FIELD(30)
  // handle sp (x2)- store its original value from sscratch
  csrr a0, sscratch
  sd a0, REG_FIELD(1)
  csrr a0, sepc
  sd a0, REG_FIELD(31)
  csrr a0, sstatus
  sd a0, REG_FIELD(32)
  csrr a0, scause
  sd a0, REG_FIELD(33)
  csrr a0, stval
  sd a0, REG_FIELD(34)

  // We're in kernel code now, so sscratch must hold zero
  csrw sscratch, zero

  mv a0, sp
  jal riscv_handle_interrupt

jump_to_user:
  // Don't really have to restore stval/ scause,
  // but for symmetry we just restore absolutely everything
  ld a0, REG_FIELD(34)
  csrw stval, a0
  ld a0, REG_FIELD(33)
  csrw scause, a0
  ld a0, REG_FIELD(32)
  csrw sstatus, a0
  ld a0, REG_FIELD(31)
  csrw sepc, a0
  ld x31, REG_FIELD(30)
  ld x30, REG_FIELD(29)
  ld x29, REG_FIELD(28)
  ld x28, REG_FIELD(27)
  ld x27, REG_FIELD(26)
  ld x26, REG_FIELD(25)
  ld x25, REG_FIELD(24)
  ld x24, REG_FIELD(23)
  ld x23, REG_FIELD(22)
  ld x22, REG_FIELD(21)
  ld x21, REG_FIELD(20)
  ld x20, REG_FIELD(19)
  ld x19, REG_FIELD(18)
  ld x18, REG_FIELD(17)
  ld x17, REG_FIELD(16)
  ld x16, REG_FIELD(15)
  ld x15, REG_FIELD(14)
  ld x14, REG_FIELD(13)
  ld x13, REG_FIELD(12)
  ld x12, REG_FIELD(11)
  ld x11, REG_FIELD(10)
  ld x10, REG_FIELD(9)
  ld x9, REG_FIELD(8)
  ld x8, REG_FIELD(7)
  ld x7, REG_FIELD(6)
  ld x6, REG_FIELD(5)
  ld x5, REG_FIELD(4)
  ld x4, REG_FIELD(3)
  ld x3, REG_FIELD(2)
  // Don't restore sp yet- we need that still!
  ld x1, REG_FIELD(0)

  // Ok now we can do sp
  ld x2, REG_FIELD(1)
  sret
