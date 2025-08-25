#include <cpu.h>
#include <io/serial.h>
#include <io/serial/uart16550.h>

#define UART16550_NUM_REGS                  8

#define UART16550_DATA_REG                  0
#define UART16550_INTERRUPT_ENABLE_REG      1

// When DLAB = 1 these ports behave as BAUD rate
#define UART16550_BAUD_RATE_LSB             0
#define UART16550_BAUD_RATE_MSB             1

#define UART16550_FIFO_CONTROL_REG          2
#define UART16550_LINE_CONTROL_REG          3
#define UART16550_MODEM_CONTROL_REG         4
#define UART16550_LINE_STATUS_REG           5
#define UART16550_MODEM_STATUS_REG          6
#define UART16550_SCRATCH_REG               7

#define UART16550_LSR_RECV_READY_MASK     0x01
#define UART16550_LSR_SEND_READY_MASK     0x20

#define RECV_TIMEOUT 5

#ifdef ARCH_X86_64

void uart16550_write_reg(uart_t *port, u16 reg, u8 val) {
  if (reg > UART16550_NUM_REGS) return;
  outb(port->baseaddr + reg, val);
}

u8 uart16550_read_reg(uart_t *port, u16 reg) {
  if (reg > UART16550_NUM_REGS) return 0;
  return inb(port->baseaddr + reg);
}

#else

void uart16550_write_reg(uart_t *port, u16 reg, u8 val) {
  if (reg > UART16550_NUM_REGS) return;
  volatile u8 *regs = (u8 *)port->baseaddr;
  regs[reg] = val;
}

u8 uart16550_read_reg(uart_t *port, u16 reg) {
  if (reg > UART16550_NUM_REGS) return 0;
  volatile u8 *regs = (u8 *)port->baseaddr;
  return regs[reg];
}

#endif // ! ARCH_X86_64

void uart16550_init(uart_t *port) {
  // Disable interrupts:
  uart16550_write_reg(port, UART16550_INTERRUPT_ENABLE_REG, 0x00);

  // Set DLAB to 1 so we can set the BAUD rate
  uart16550_write_reg(port, UART16550_LINE_CONTROL_REG, 0x80);

  // Set BAUD rate to 115200 (fastest)
  // Rate of 0x0001 (115200 / 1 = 115200 AKA full speed)
  uart16550_write_reg(port, UART16550_BAUD_RATE_LSB, 0x01);
  uart16550_write_reg(port, UART16550_BAUD_RATE_MSB, 0x00);

  // 8N1 (8 bits, no parity, 1 stop bit)- this is the default for UART
  uart16550_write_reg(port, UART16550_LINE_CONTROL_REG, 0x03);

  // Set port to normal mode
  uart16550_write_reg(port, UART16550_MODEM_CONTROL_REG, 0x0F);

  // Enable Received Data Available interrupt
  uart16550_write_reg(port, UART16550_INTERRUPT_ENABLE_REG, 0x01);
}

void uart16550_send(uart_t *port, u8 c) {
  while ((uart16550_read_reg(port, UART16550_LINE_STATUS_REG) & UART16550_LSR_SEND_READY_MASK) == 0) {}
  uart16550_write_reg(port, UART16550_DATA_REG, c);
}

u8 uart16550_recv(uart_t *port) {
  int counter = 0;
  while ((uart16550_read_reg(port, UART16550_LINE_STATUS_REG) & UART16550_LSR_RECV_READY_MASK) == 0) {
    if (counter++ > RECV_TIMEOUT) return 0;
  }
  return uart16550_read_reg(port, UART16550_DATA_REG);
}

bool uart16550_has_data(uart_t *port) {
  return uart16550_read_reg(port, (UART16550_LINE_STATUS_REG) & UART16550_LSR_RECV_READY_MASK) != 0;
}

uart_ops_t uart16550_uart_ops = {
  .init = uart16550_init,
  .send = uart16550_send,
  .recv = uart16550_recv,
  .has_data = uart16550_has_data,
};
