#pragma once
#include <types.h>

// Saved registers at a user/ kernel boundary crossing (we want all of them)
typedef struct {
  u64 x0; // X0-X7: Arguments (X0 is also the return value)
  u64 x1;
  u64 x2;
  u64 x3;
  u64 x4;
  u64 x5;
  u64 x6;
  u64 x7;
  u64 x8; // X8-X15: Caller saved
  u64 x9;
  u64 x10;
  u64 x11;
  u64 x12;
  u64 x13;
  u64 x14;
  u64 x15;
  u64 x16; // X16-X17: Intra-procedural call scratch registers
  u64 x17;
  u64 x18; // X18-X30: Callee Saved
  u64 x19;
  u64 x20;
  u64 x21;
  u64 x22;
  u64 x23;
  u64 x24;
  u64 x25;
  u64 x26;
  u64 x27;
  u64 x28;
  u64 x29; // Frame pointer
  u64 x30; // Link register (return address)
  u64 sp_el0;
  u64 spsr;
  u64 esr;
  u64 elr;
  u64 __padding; // Padding to align this to a multiple of 16
} regs_t;

// Saved registers during a kernel/ kernel context switch (we just need all callee saved + return address)
typedef struct {
  u64 x18;
  u64 x19;
  u64 x20;
  u64 x21;
  u64 x22;
  u64 x23;
  u64 x24;
  u64 x25;
  u64 x26;
  u64 x27;
  u64 x28;
  u64 x29;
  u64 kern_ra; // LR / X30
  u64 __padding; // Padding to align this to a multiple of 16
} kregs_t;
