#pragma once
#include <types.h>

// Saved registers at a user/ kernel boundary crossing (we want all of them)
typedef struct {
  // We push these:
  u64 rax;
  u64 rbx;
  u64 rcx;
  u64 rdx;
  u64 rdi;
  u64 rsi;
  u64 rbp;
  u64 r8;
  u64 r9;
  u64 r10;
  u64 r11;
  u64 r12;
  u64 r13;
  u64 r14;
  u64 r15;

  // HW / interrupt handler pushes these:
  u64 interrupt_number;
  u64 error_code;
  u64 rip;
  u64 cs;
  u64 rflags;
  u64 rsp;
  u64 ss;
} regs_t;

// Saved registers during a kernel/ kernel context switch (we just need all callee saved + return address)
typedef struct {
  // We push these:
  u64 rbx;
  u64 rbp;
  u64 r12;
  u64 r13;
  u64 r14;
  u64 r15;

  // HW pushes this:
  u64 kern_ra; // RIP
} kregs_t;
