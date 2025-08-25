#pragma once
#include <types.h>

// Saved registers at a user/ kernel boundary crossing (we want all of them)
typedef struct {
  u64 x1; // Return Address (ra)
  u64 x2; // Stack Pointer (sp)
  u64 x3;
  u64 x4;
  u64 x5;
  u64 x6;
  u64 x7;
  u64 x8;
  u64 x9;
  u64 x10;
  u64 x11;
  u64 x12;
  u64 x13;
  u64 x14;
  u64 x15;
  u64 x16;
  u64 x17;
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
  u64 x30;
  u64 x31;
  u64 sepc;
  u64 sstatus;
  u64 scause;
  u64 stval;
} regs_t;

// Saved registers during a kernel/ kernel context switch (we just need all callee saved + return address)
typedef struct {
  u64 s0;
  u64 s1;
  u64 s2;
  u64 s3;
  u64 s4;
  u64 s5;
  u64 s6;
  u64 s7;
  u64 s8;
  u64 s9;
  u64 s10;
  u64 s11;
  u64 kern_ra; // RA / X1
} kregs_t;
