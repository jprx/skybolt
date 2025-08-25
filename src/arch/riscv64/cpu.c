#include <types.h>
#include <defs.h>
#include <cpu.h>
#include <lib.h>
#include <task.h>
#include <platform.h>

void init_cpu() {
  write_csr(sie, -1ull); // enable all
  write_csr(stvec, riscv_trap);
}

void cpu_halt() {
  asm volatile("wfi");
}

void init_timer() {
  riscv_timer_schedule();
}

void riscv_timer_schedule() {
  write_csr(stimecmp, read_csr(time) + STIME_DEADLINE);
}

void cpu_set_entry_kstack(struct thread_t *th) {
  if (th->trapdepth > 1 || MODE_KERN == th->task->taskmode) {
    write_csr(sscratch, 0);
    return;
  }

  if (MODE_USER == th->task->taskmode) {
    // trapdepth of 0 means we haven't started the task yet
    assert(th->trapdepth <= 1);
    write_csr(sscratch, STACK_BOTTOM(th->kstack_pg));
    return;
  }

  panic("cpu_set_entry_kstack: unhandled case");
}

void cpu_dump_regs(regs_t *r) {
  if (!r) return;
  printf("SEPC:0x%X\n", r->sepc);
  printf("X1:  0x%X    ", r->x1);
  printf("X2:  0x%X    ", r->x2);
  printf("X3:  0x%X    ", r->x3);
  printf("X4:  0x%X\n", r->x4);
  printf("X5:  0x%X    ", r->x5);
  printf("X6:  0x%X    ", r->x6);
  printf("X7:  0x%X    ", r->x7);
  printf("X8:  0x%X\n", r->x8);
  printf("X9:  0x%X    ", r->x9);
  printf("X10: 0x%X    ", r->x10);
  printf("X11: 0x%X    ", r->x11);
  printf("X12: 0x%X\n", r->x12);
  printf("X13: 0x%X    ", r->x13);
  printf("X14: 0x%X    ", r->x14);
  printf("X15: 0x%X    ", r->x15);
  printf("X16: 0x%X\n", r->x16);
  printf("X17: 0x%X    ", r->x17);
  printf("X18: 0x%X    ", r->x18);
  printf("X19: 0x%X    ", r->x19);
  printf("X20: 0x%X\n", r->x20);
  printf("X21: 0x%X    ", r->x21);
  printf("X22: 0x%X    ", r->x22);
  printf("X23: 0x%X    ", r->x23);
  printf("X24: 0x%X\n", r->x24);
  printf("X25: 0x%X    ", r->x25);
  printf("X26: 0x%X    ", r->x26);
  printf("X27: 0x%X    ", r->x27);
  printf("X28: 0x%X\n", r->x28);
  printf("X29: 0x%X    ", r->x29);
  printf("X30: 0x%X    ", r->x30);
  printf("X31: 0x%X    \n", r->x31);
  printf("SSTATUS:  0x%X\n", r->sstatus);
  printf("SCAUSE:   0x%X\n", r->scause);
  printf("STVAL:    0x%X\n", r->stval);
}

cpu_mode_t get_reg_mode(regs_t *r) {
  if (RISCV_MODE_U == SSTATUS_GET_SPP(r->sstatus)) return MODE_USER;
  if (RISCV_MODE_S == SSTATUS_GET_SPP(r->sstatus)) return MODE_KERN;
  panic("get_reg_mode: unknown mode");
}

void init_user_regs(regs_t *r, virt_t init_pc, virt_t init_sp) {
  memset(r, '\x00', sizeof(*r));
  r->sepc = init_pc;
  r->x2 = init_sp;
  r->sstatus = SSTATUS_SUM | SSTATUS_UXL;
}

void set_reg_arg(regs_t *r, usize idx, u64 val) {
  switch(idx) {
    case 0: r->x10 = val; break;
    case 1: r->x11 = val; break;
    default: panic("set_reg_arg: invalid idx");
  }
}

void cpu_disable_ints() {
  write_csr(sstatus, read_csr(sstatus) & ~SSTATUS_SIE);
}

void cpu_enable_ints() {
  write_csr(sstatus, read_csr(sstatus) | SSTATUS_SIE);
}

bool cpu_ints_enabled() {
  return (read_csr(sstatus) & SSTATUS_SIE) != 0;
}

bool cpu_set_ints(bool new_mode) {
  bool rval = cpu_ints_enabled();
  if (new_mode) cpu_enable_ints();
  else cpu_disable_ints();
  return rval;
}

void cpu_flush_tlb() {
  asm volatile("sfence.vma zero, zero");
}

void cpu_flush_dcache(virt_t start, virt_t end) {
  // todo: handle this for real hardware.
}

void cpu_flush_icache() {
  // todo: handle this for real hardware.
  // (right now we only run in a VM for RISCV64).
}

void cpu_set_pagetable(pte_t *new_pt) {
  asm volatile(
    "sfence.vma zero, zero\n"
    "csrw satp, %0\n"
    "sfence.vma zero, zero\n"
    :
    : "r"(KERN_V2P((u64)new_pt) >> SATP_PPN_PAGE_SHIFT | (RISCV_PAGING_MODE_SV48 << RISCV_PAGING_MODE_SHIFT))
  );
}

u32 read32(u32 *addr) {
  u32 rval;
  asm volatile(
    "lw %0, 0(%1)"
    : "=r"(rval)
    : "r"(addr)
  );

  return rval;
}

void write32(u32 *addr, u32 val) {
  asm volatile(
    "sw %1, 0(%0)"
    :
    : "r"(addr), "r"(val)
  );
}

