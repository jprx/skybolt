#include <cpu.h>
#include <platform.h>
#include <lib.h>
#include <io/intctl.h>
#include <task.h>
#include <tty.h>

void timer_intr() {
  scheduler();
}

void serial_intr() {
  tty_new_data(&g_serial_tty, serial_getc());
}

void init_intctl() {
  // Make sure the platform header agrees with the platform defs
  assert(g_uart.core_irq == UART_IRQ);

  g_intc.ops->init(&g_intc);

  g_intc.ops->map(
    &g_intc,
    g_uart.ext_irq,
    g_uart.core_irq
  );
}

irq_t claim_interrupt() {
  return g_intc.ops->claim(&g_intc);
}

void send_eoi(irq_t irqnum) {
  g_intc.ops->eoi(&g_intc, irqnum);
}
