#include <cpu.h>
#include <lib.h>
#include <platform.h>
#include <io/serial.h>
#include <io/intctl.h>
#include <io/ps2.h>
#include <syscall.h>
#include <exception.h>
#include <task.h>

#define X86_IRQ_PAGEFAULT 14

void x86_handle_syscall(regs_t *regs) {
  // We use R10 instead of RCX for arg4, since syscall/sysret use RCX
  thread_trap_enter(regs);
  regs->rax = do_syscall(regs->rax, regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8);
  thread_trap_exit();
}

void x86_handle_interrupt(regs_t *regs) {
  irq_t irqnum;
  thread_trap_enter(regs);

  switch(regs->interrupt_number) {
    case UART_IRQ:
      irqnum = claim_interrupt();
      send_eoi(irqnum);
      serial_intr();
      break;

    case LAPIC_TIMER_IRQ:
      irqnum = claim_interrupt();
      send_eoi(irqnum);
      timer_intr();
      break;

    case PS2_KEYBOARD_IRQ:
      irqnum = claim_interrupt();
      send_eoi(irqnum);
      ps2_intr();
      break;

    case X86_IRQ_PAGEFAULT:
      do_exception(regs, EX_PAGEFAULT, read_cr2());
      break;

    default:
      do_exception(regs, EX_UNKNOWN, 0);
      break;
  }

  run_scheduler_if_idle();
  thread_trap_exit();
}
