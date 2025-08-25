#include <cpu.h>
#include <lib.h>
#include <platform.h>
#include <io/intctl.h>
#include <syscall.h>
#include <exception.h>
#include <task.h>

void arm_handle_sync_exception(regs_t *regs) {
  thread_trap_enter(regs);

  switch(ESR_REASON(regs->esr)) {
    case ESR_SVC64:
      regs->x0 = do_syscall(regs->x0, regs->x1, regs->x2, regs->x3, regs->x4, regs->x5);
      break;

    case ESR_DATA_ABORT_LOWLEVEL:
    case ESR_DATA_ABORT_NOCHANGE:
    case ESR_INST_ABORT_LOWLEVEL:
    case ESR_INST_ABORT_NOCHANGE:
      do_exception(regs, EX_PAGEFAULT, read_msr(far_el1));
      break;

    default:
      do_exception(regs, EX_UNKNOWN, 0);
      break;
  }

  thread_trap_exit();
}

void arm_handle_interrupt(regs_t *regs) {
  irq_t irq;
  thread_trap_enter(regs);
  irq = claim_interrupt();
  send_eoi(irq);

  switch(irq) {
    case UART_IRQ:
      serial_intr();
      break;

    case TIMER_IRQ:
      timer_intr();
      arm_timer_schedule();
      break;

    default:
      panic("Unknown interrupt");
      break;
  }

  run_scheduler_if_idle();
  thread_trap_exit();
}

void arm_handle_serror(regs_t *regs) {
  thread_trap_enter(regs);
  panic("serror");
  thread_trap_exit();
}
