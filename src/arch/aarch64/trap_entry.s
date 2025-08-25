.section .text

#include <defs.h>

.global exception_table
.global jump_to_user

#define REG_FIELD(i) [sp,#(8*i)]

.macro DISABLE_INTS
msr daifset, 0x3
.endm

// Push all registers to the stack, forming a regs_t
.macro PUSH_ALL
  add sp, sp, -(REGS_SIZE)
  str x0,  REG_FIELD(0)
  str x1,  REG_FIELD(1)
  str x2,  REG_FIELD(2)
  str x3,  REG_FIELD(3)
  str x4,  REG_FIELD(4)
  str x5,  REG_FIELD(5)
  str x6,  REG_FIELD(6)
  str x7,  REG_FIELD(7)
  str x8,  REG_FIELD(8)
  str x9,  REG_FIELD(9)
  str x10, REG_FIELD(10)
  str x11, REG_FIELD(11)
  str x12, REG_FIELD(12)
  str x13, REG_FIELD(13)
  str x14, REG_FIELD(14)
  str x15, REG_FIELD(15)
  str x16, REG_FIELD(16)
  str x17, REG_FIELD(17)
  str x18, REG_FIELD(18)
  str x19, REG_FIELD(19)
  str x20, REG_FIELD(20)
  str x21, REG_FIELD(21)
  str x22, REG_FIELD(22)
  str x23, REG_FIELD(23)
  str x24, REG_FIELD(24)
  str x25, REG_FIELD(25)
  str x26, REG_FIELD(26)
  str x27, REG_FIELD(27)
  str x28, REG_FIELD(28)
  str x29, REG_FIELD(29)
  str x30, REG_FIELD(30)
  mrs x0, sp_el0
  str x0, REG_FIELD(31)
  mrs x0, spsr_el1
  str x0, REG_FIELD(32)
  mrs x0, esr_el1
  str x0, REG_FIELD(33)
  mrs x0, elr_el1
  str x0, REG_FIELD(34)
.endm

.macro POP_ALL
  ldr x0, REG_FIELD(34)
  msr elr_el1, x0
  ldr x0, REG_FIELD(33)
  msr esr_el1, x0
  ldr x0, REG_FIELD(32)
  msr spsr_el1, x0
  ldr x0, REG_FIELD(31)
  msr sp_el0, x0
  ldr x30, REG_FIELD(30)
  ldr x29, REG_FIELD(29)
  ldr x28, REG_FIELD(28)
  ldr x27, REG_FIELD(27)
  ldr x26, REG_FIELD(26)
  ldr x25, REG_FIELD(25)
  ldr x24, REG_FIELD(24)
  ldr x23, REG_FIELD(23)
  ldr x22, REG_FIELD(22)
  ldr x21, REG_FIELD(21)
  ldr x20, REG_FIELD(20)
  ldr x19, REG_FIELD(19)
  ldr x18, REG_FIELD(18)
  ldr x17, REG_FIELD(17)
  ldr x16, REG_FIELD(16)
  ldr x15, REG_FIELD(15)
  ldr x14, REG_FIELD(14)
  ldr x13, REG_FIELD(13)
  ldr x12, REG_FIELD(12)
  ldr x11, REG_FIELD(11)
  ldr x10, REG_FIELD(10)
  ldr x9,  REG_FIELD(9)
  ldr x8,  REG_FIELD(8)
  ldr x7,  REG_FIELD(7)
  ldr x6,  REG_FIELD(6)
  ldr x5,  REG_FIELD(5)
  ldr x4,  REG_FIELD(4)
  ldr x3,  REG_FIELD(3)
  ldr x2,  REG_FIELD(2)
  ldr x1,  REG_FIELD(1)
  ldr x0,  REG_FIELD(0)
  add sp, sp, REGS_SIZE
.endm

jump_to_user:
  POP_ALL
  eret

exception_sync:
  PUSH_ALL
  mov x0, sp
  bl arm_handle_sync_exception
  POP_ALL
  eret

exception_fiq:
exception_irq:
  PUSH_ALL
  mov x0, sp
  bl arm_handle_interrupt
  POP_ALL
  eret

exception_serror:
  PUSH_ALL
  bl arm_handle_serror
  POP_ALL
  eret

.balign 0x1000
exception_table:
// Same EL, SP0 (we should never hit these)
.balign 0x80
  DISABLE_INTS
  b .
.balign 0x80
  DISABLE_INTS
  b .
.balign 0x80
  DISABLE_INTS
  b .
.balign 0x80
  DISABLE_INTS
  b .

// Same EL, SP_EL1- trap from kernelspace
.balign 0x80
  DISABLE_INTS
  b exception_sync
.balign 0x80
  DISABLE_INTS
  b exception_irq
.balign 0x80
  DISABLE_INTS
  b exception_fiq
.balign 0x80
  DISABLE_INTS
  b exception_serror

// Lower EL, AARCH64- trap from userspace
.balign 0x80
  DISABLE_INTS
  b exception_sync
.balign 0x80
  DISABLE_INTS
  b exception_irq
.balign 0x80
  DISABLE_INTS
  b exception_fiq
.balign 0x80
  DISABLE_INTS
  b exception_serror

// Lower EL, AARCH32 (we should never hit these)
.balign 0x80
  DISABLE_INTS
  b .
.balign 0x80
  DISABLE_INTS
  b .
.balign 0x80
  DISABLE_INTS
  b .
.balign 0x80
  DISABLE_INTS
  b .
