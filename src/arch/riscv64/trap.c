#include <cpu.h>
#include <defs.h>
#include <task.h>
#include <lib.h>
#include <platform.h>
#include <exception.h>
#include <syscall.h>
#include <task.h>

#define SCAUSE_INTERRUPT       BIT(63)
#define SCAUSE_SOFTWARE        ((1 | SCAUSE_INTERRUPT))
#define SCAUSE_TIMER           ((5 | SCAUSE_INTERRUPT))
#define SCAUSE_EXTERNAL        ((9 | SCAUSE_INTERRUPT))
#define SCAUSE_SYSCALL         ((8))
#define SCAUSE_INST_PAGEFAULT  ((12))
#define SCAUSE_DATA_PAGEFAULT  ((13))
#define SCAUSE_SAMO_PAGEFAULT  ((15))

void external_interrupt(regs_t *r) {
  irq_t irq = claim_interrupt();
  send_eoi(irq);

  switch(irq) {
    case UART_IRQ:
      serial_intr();
      break;

    default:
      panic("Unknown interrupt");
      break;
  }
}

void riscv_handle_interrupt(regs_t *regs) {
  // When in kernel code, sscratch must be zero (set by riscv_trap).
  // This way, future traps know that sp is already a kernel sp and don't
  // overwrite the existing user state.
  assert(read_csr(sscratch) == 0);
  thread_trap_enter(regs);

  switch(regs->scause) {
    case SCAUSE_SYSCALL:
      regs->x10 = do_syscall(regs->x10, regs->x11, regs->x12, regs->x13, regs->x14, regs->x15);
      regs->sepc += RISCV_INST_LEN;
      break;

    case SCAUSE_EXTERNAL:
      external_interrupt(regs);
      break;

    case SCAUSE_TIMER:
      timer_intr();
      riscv_timer_schedule();
      break;

    case SCAUSE_DATA_PAGEFAULT:
    case SCAUSE_INST_PAGEFAULT:
    case SCAUSE_SAMO_PAGEFAULT:
      do_exception(regs, EX_PAGEFAULT, read_csr(stval));
      break;

    default:
      printf("unknown scause reason: 0x%X\n", regs->scause);
      do_exception(regs, EX_UNKNOWN, 0);
      break;
  }

  run_scheduler_if_idle();
  thread_trap_exit();

  if (MODE_USER == get_reg_mode(regs)) {
    assert(current_thread()->trapdepth == 0);
    assert(SSTATUS_GET_SPP(regs->sstatus) == RISCV_MODE_U);
    assert(read_csr(sscratch) != 0);
  }

  if (MODE_KERN == get_reg_mode(regs)) {
    assert(SSTATUS_GET_SPP(regs->sstatus) == RISCV_MODE_S);
    assert(read_csr(sscratch) == 0);
  }
}
