#pragma once
#include <types.h>
#include <page.h>

#ifdef ARCH_X86_64
#include <arch/x86_64/cpu.h>
#endif // ARCH_X86_64

#ifdef ARCH_AARCH64
#include <arch/aarch64/cpu.h>
#endif // ARCH_AARCH64

#ifdef ARCH_RISCV64
#include <arch/riscv64/cpu.h>
#endif // ARCH_RISCV64

typedef enum {
  CPU_MODE_UNDEFINED = 0,
  MODE_KERN,
  MODE_USER
} cpu_mode_t;

// Methods that every CPU should implement

void init_cpu();
void init_timer();

// thread management
struct thread_t;
void cpu_set_entry_kstack(struct thread_t *th);

// Start a new user task by popping all registers off the stack and using a
// return from interrupt instruction (iret, eret, sret, etc.) to jump to
// userspace. Assumes the stack pointer points to a regs_t when called.
void jump_to_user();

// volatile read/ writes that will never be optimized out
u32  read32(u32 *addr);
void write32(u32 *addr, u32 val);

// register state management
void cpu_dump_regs(regs_t *r);
void init_user_regs(regs_t *r, virt_t init_pc, virt_t init_sp);
void set_reg_arg(regs_t *r, usize idx, u64 val);

// somewhere in a regs_t is a register that tracks what mode we were in before the interrupt
cpu_mode_t get_reg_mode(regs_t*);

// vm management
void cpu_flush_tlb();
void cpu_set_pagetable(pte_t *);

// Whenever we write instructions into RAM, on some ISAs we need to flush the
// instruction cache as the L1I and L1D caches don't talk to one another. When
// we write to memory, it goes through the L1D cache, and the L1I cache may not
// see the change. On X86_64 the L1I and L1D caches are always in sync, but on
// AARCH64 and possibly RISCV64 you may need to sync the caches then flush the
// icache after writing instructions to memory (eg when loading a program).
//
// cpu_flush_dcache flushes the data cache to the point of unification (where
// the instruction cache will see it- probably L2). cpu_flush_icache will clear
// the instruction cache, and when it is refilled it will see the latest memory
// contents.
void cpu_flush_dcache(virt_t start, virt_t end);
void cpu_flush_icache();

// interrupt management
void cpu_disable_ints();
void cpu_enable_ints();
bool cpu_ints_enabled();
// set the interrupt mode and return what it used to be before this change
bool cpu_set_ints(bool);
void cpu_halt();

// per-ISA page table management functions
bool     pte_is_valid(pte_t *);
void     pte_set_valid(pte_t *, bool);
bool     pte_is_leaf(pte_t *, pagelevel_t);
void     pte_set_leaf(pte_t *, pagelevel_t, bool);
phys_t   pte_get_phys(pte_t *);
void     pte_set_phys(pte_t *, phys_t);
void     pte_set_priv(pte_t *, cpu_mode_t);
void     pte_set_cache_policy(pte_t *, cache_policy_t);
