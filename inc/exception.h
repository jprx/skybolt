#pragma once
#include <types.h>
#include <cpu.h>

typedef enum {
  EX_UNKNOWN = 0,
  EX_PAGEFAULT,
} exception_kind_t;

void do_exception(regs_t *, exception_kind_t, u64);
