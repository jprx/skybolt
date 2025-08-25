#include <types.h>
#include <cpu.h>
#include <vm.h>
#include <lib.h>
#include <arch/x86_64/segment.h>
#include <io/interrupt/apic.h>
#include <task.h>

extern tss_t tss;

u64 per_cpu_scratch[8]; // Used as scratch register during syscall entry

void init_cpu() {
  init_segmentation();
  write_msr(MSR_STAR, (SYSCALL_SELECTOR << 32ull) | (SYSRET_SELECTOR << 48ull));
  write_msr(MSR_LSTAR, (u64)syscall_entry);
  write_msr(MSR_SFMASK, RFLAGS_IF); // Mask off rflags.if to disable interrupts
  write_msr(MSR_FSBASE, 0);

  // Changing the PAT register overrides what various bits in the PTEs do.
  // Here's our new configuration:
  // PA0- Writeback:       PAT = 0, PCD = 0, PWT = 0
  // PA1- Write Combining: PAT = 0, PCD = 0, PWT = 1
  // PA2- Uncacheable:     PAT = 0, PCD = 1, PWT = 0
  write_msr(MSR_PAT, (PAT_WB << 0) | (PAT_WC << 8) | (PAT_UC << 16));

  // We should really use swapgs to switch user and kernel gs's, but we don't
  // let userspace change gs anyways so we can just keep both equal to our
  // scratch region for simplicity.
  write_msr(MSR_GSBASE, (uintptr_t)&per_cpu_scratch);
  write_msr(MSR_KGSBASE, (uintptr_t)&per_cpu_scratch);
}

void cpu_halt() {
  asm volatile("hlt");
}

void init_timer() {
  // lapic driver handles this for us; nothing to do here
}

void cpu_dump_regs(regs_t *r) {
  if (!r) return;
  printf("RIP: 0x%X\n", r->rip);
  printf("RAX: 0x%X    ", r->rax);
  printf("RBX: 0x%X    ", r->rbx);
  printf("RCX: 0x%X    ", r->rcx);
  printf("RDX: 0x%X\n", r->rdx);
  printf("RDI: 0x%X    ", r->rdi);
  printf("RSI: 0x%X    ", r->rsi);
  printf("RBP: 0x%X    ", r->rbp);
  printf("RSP: 0x%X\n", r->rsp);
  printf("R8:  0x%X    ", r->r8);
  printf("R9:  0x%X    ", r->r9);
  printf("R10: 0x%X    ", r->r10);
  printf("R11: 0x%X\n", r->r11);
  printf("R12: 0x%X    ", r->r12);
  printf("R13: 0x%X    ", r->r13);
  printf("R14: 0x%X    ", r->r14);
  printf("R15: 0x%X\n", r->r15);
  printf("ERR: 0x%X    ", r->error_code);
  printf("CS:  0x%X    ", r->cs);
  printf("SS:  0x%X    ", r->ss);
  printf("RFL: 0x%X\n", r->rflags);
}

cpu_mode_t get_reg_mode(regs_t *r) {
  if (USER_CS   == r->cs) return MODE_USER;
  if (KERNEL_CS == r->cs) return MODE_KERN;
  panic("get_reg_mode: improper value in cs selector");
}

void cpu_set_entry_kstack(struct thread_t *th) {
  tss.rsp0 = STACK_BOTTOM(th->kstack_pg);
}

void init_user_regs(regs_t *r, virt_t init_pc, virt_t init_sp) {
  memset(r, '\x00', sizeof(*r));
  r->rip = init_pc;
  r->cs  = USER_CS;
  r->rflags = RFLAGS_IF;
  r->rsp = init_sp;
  r->ss  = USER_DS;
}

void set_reg_arg(regs_t *r, usize idx, u64 val) {
  switch(idx) {
    case 0: r->rdi = val; break;
    case 1: r->rsi = val; break;
    default: panic("set_reg_arg: invalid idx");
  }
}

void cpu_disable_ints() {
  asm volatile("cli");
}

void cpu_enable_ints() {
  asm volatile("sti");
}

bool cpu_ints_enabled() {
  u64 old_rflags;
  asm volatile(
    "pushfq\n"
    "pop %0"
    : "=r"(old_rflags)
  );

  return (old_rflags & RFLAGS_IF) != 0;
}

bool cpu_set_ints(bool new_mode) {
  bool rval = cpu_ints_enabled();
  if (new_mode) cpu_enable_ints();
  else cpu_disable_ints();
  return rval;
}

void cpu_flush_tlb() {
  u64 tmp;
  asm volatile(
    "mov %0, cr3\n"
    "mov cr3, %0\n"
    : "=r"(tmp)
  );
}

void cpu_flush_dcache(virt_t start, virt_t end) {
  // On X86_64, L1I and L1D caches are always in sync, so do nothing here.
}

void cpu_flush_icache() {
  // On X86_64, L1I and L1D caches are always in sync, so do nothing here.
  // The wbnoinvd instruction can be used to flush all caches
  // to memory if you want memory to be up to date.
}

void cpu_set_pagetable(pte_t *new_pt) {
  asm volatile(
    "mov cr3, %0"
    :
    : "r"(KERN_V2P((u64)new_pt))
  );
}

pte_t *cpu_get_pagetable() {
  u64 out_pt;

  asm volatile(
    "mov %0, cr3\n"
    : "=r"(out_pt)
  );

  out_pt = KERN_P2V(out_pt);
  return (pte_t *)out_pt;
}

void outb(u16 port, u8 val) {
  asm volatile(
    "outb %0, %1"
    :
    : "dN"(port), "a"(val)
  );
}

void outw(u16 port, u16 val) {
  asm volatile(
    "outw %0, %1"
    :
    : "dN"(port), "ax"(val)
  );
}

u8 inb(u16 port) {
  u8 outval;
  asm volatile(
    "inb %0, %1"
    : "=a"(outval)
    : "dN"(port)
  );
  return outval;
}

u64 read_cr2() {
  u64 outval;
  asm volatile(
    "mov %0, cr2"
    : "=a"(outval)
  );
  return outval;
}

u64 read_msr (u32 msr_idx) {
  u32 val_hi;
  u32 val_low;
  asm volatile(
    "rdmsr"
    : "=a"(val_low), "=d"(val_hi)
    : "c"(msr_idx)
  );

  return ((u64)val_hi << 32) | val_low;
}

void write_msr(u32 msr_idx, u64 val) {
  u32 val_hi = (val >> 32);
  u32 val_low = val;

  asm volatile(
    "wrmsr"
    :: "c"(msr_idx), "a"(val_low), "d"(val_hi)
  );
}
