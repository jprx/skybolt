#include <platform.h>
#include <types.h>
#include <defs.h>
#include <lib.h>
#include <cpu.h>
#include <io/intctl.h>

// We use the virtual timer rather than the physical one so that it'll work
// when running with acceleration under Apple's HVF (where direct access to the
// physical timer is disallowed). This also works fine on real hw.

#define CNTP_CTL_INTERRUPTS_ENABLED         BIT(0)
#define CNTV_CTL_INTERRUPTS_ENABLED         BIT(0)

// Should learn this from reading CLIDR, but for now just assume 64 byte lines
#define CACHE_LINE_SIZE 64

void init_cpu() {
  write_msr(vbar_el1, &exception_table);
  write_msr(mair_el1, MAIR_RUNTIME);
}

void cpu_halt() {
  asm volatile("wfi");
}

void init_timer() {
  arm_timer_init();
  arm_timer_schedule();
  g_intc.ops->map(&g_intc, TIMER_IRQ, TIMER_IRQ);
}

void cpu_set_entry_kstack(struct thread_t *th) {
  // Done automatically for us via SP_EL1
}

void cpu_dump_regs(regs_t *r) {
  if (!r) return;
  printf("ELR: 0x%X\n", r->elr);
  printf("X0:  0x%X    ", r->x0);
  printf("X1:  0x%X    ", r->x1);
  printf("X2:  0x%X    ", r->x2);
  printf("X3:  0x%X\n", r->x3);
  printf("X4:  0x%X    ", r->x4);
  printf("X5:  0x%X    ", r->x5);
  printf("X6:  0x%X    ", r->x6);
  printf("X7:  0x%X\n", r->x7);
  printf("X8:  0x%X    ", r->x8);
  printf("X9:  0x%X    ", r->x9);
  printf("X10: 0x%X    ", r->x10);
  printf("X11: 0x%X\n", r->x11);
  printf("X12: 0x%X    ", r->x12);
  printf("X13: 0x%X    ", r->x13);
  printf("X14: 0x%X    ", r->x14);
  printf("X15: 0x%X\n", r->x15);
  printf("X16: 0x%X    ", r->x16);
  printf("X17: 0x%X    ", r->x17);
  printf("X18: 0x%X    ", r->x18);
  printf("X19: 0x%X\n", r->x19);
  printf("X20: 0x%X    ", r->x20);
  printf("X21: 0x%X    ", r->x21);
  printf("X22: 0x%X    ", r->x22);
  printf("X23: 0x%X\n", r->x23);
  printf("X24: 0x%X    ", r->x24);
  printf("X25: 0x%X    ", r->x25);
  printf("X26: 0x%X    ", r->x26);
  printf("X27: 0x%X\n", r->x27);
  printf("X28: 0x%X    ", r->x28);
  printf("X29: 0x%X    ", r->x29);
  printf("X30: 0x%X\n", r->x30);
  printf("SP0: 0x%X    ", r->sp_el0);
  printf("ESR: 0x%X    ", r->esr);
  printf("SPSR:0x%X\n", r->spsr);
}

cpu_mode_t get_reg_mode(regs_t *r) {
  if (SPSR_EL1_USER == SPSR_GET_M(r->spsr)) return MODE_USER;
  if (SPSR_EL1_KERN == SPSR_GET_M(r->spsr)) return MODE_KERN;
  panic("get_reg_mode: unknown mode");
}

void init_user_regs(regs_t *r, virt_t init_pc, virt_t init_sp) {
  memset(r, '\x00', sizeof(*r));
  r->elr = init_pc;
  r->sp_el0 = init_sp;
  r->spsr = SPSR_EL1_USER;
}

void set_reg_arg(regs_t *r, usize idx, u64 val) {
  switch(idx) {
    case 0: r->x0 = val; break;
    case 1: r->x1 = val; break;
    default: panic("set_reg_arg: invalid idx");
  }
}

void cpu_disable_ints() {
  asm volatile("msr daifset, 0x3");
}

void cpu_enable_ints() {
  asm volatile("msr daifclr, 0x3");
}

bool cpu_ints_enabled() {
  u64 daif;
  asm volatile(
    "mrs %0, daif\n"
    : "=r"(daif)
  );

  return 0 == ((daif & DAIF_FIQ) | (daif & DAIF_IRQ));
}

bool cpu_set_ints(bool new_mode) {
  bool rval = cpu_ints_enabled();
  if (new_mode) cpu_enable_ints();
  else cpu_disable_ints();
  return rval;
}

void cpu_flush_tlb() {
  // See ARM100940_0101 Section 10
  cpu_flush_icache();
  asm volatile(
    "dsb ishst\n"
    "tlbi vmalle1\n"
    "dsb ish\n"
    "isb\n"
  );
}

void cpu_set_pagetable(pte_t *new_pt) {
  asm volatile(
    "msr ttbr0_el1, %0\n"
    "msr ttbr1_el1, %1\n"
    :
    : "r"(KERN_V2P((u64)&new_pt[0])), "r"(KERN_V2P((u64)&new_pt[NUM_PT_ENTRIES / 2]))
  );
  cpu_flush_tlb();
}

void cpu_flush_dcache(virt_t start, virt_t end) {
  // Flush to point of unification (likely L2), so that future instruction
  // cache refills observe any instructions we just wrote.
  for (usize i = start; i < end; i += CACHE_LINE_SIZE) {
    asm volatile("dc cvau, %0" :: "r"(i));
  }
  asm volatile("dsb sy");
}

void cpu_flush_icache() {
  asm volatile(
    "dsb sy\n"
    "ic iallu\n"
    "dsb sy\n"
    "isb\n"
  );
}

u32 read32(u32 *addr) {
  u32 rval;
  asm volatile(
    "ldr %w0, [%1]"
    : "=r"(rval)
    : "r"(addr)
  );

  return rval;
}

void write32(u32 *addr, u32 val) {
  asm volatile(
    "str %w1, [%0]"
    :
    : "r"(addr), "r"(val)
  );
}

void arm_timer_schedule() {
  asm volatile(
    "msr CNTV_TVAL_EL0, %0"
    :: "r"(CNTV_DEADLINE)
  );
}

void arm_timer_init() {
  asm volatile(
    "msr CNTV_CTL_EL0, %0"
    :: "r"(CNTV_CTL_INTERRUPTS_ENABLED)
  );
}
