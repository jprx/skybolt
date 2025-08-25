#include <platform.h>
#include <io/serial.h>
#include <io/serial/pl011.h>
#include <cpu.h>

#define REG_DR    0x00
#define REG_FR    0x18
#define REG_IBRD  0x24
#define REG_FBRD  0x28
#define REG_LCRH  0x2C
#define REG_CR    0x30
#define REG_IFLS  0x34
#define REG_IMSC  0x38
#define REG_ICR   0x44

#define RECV_TIMEOUT 5

#define FLAG_RECV_EMPTY       BIT(4)
#define FLAG_XMIT_EMPTY       BIT(5)

#define IMSC_INTERRUPT_RECV   BIT(4)

#define CR_ENABLED            BIT(0)
#define CR_XMIT_ENABLE        BIT(8)
#define CR_RECV_ENABLE        BIT(9)

void pl011_write_reg(uart_t *port, u16 reg, u32 val) {
  write32((u32*)(port->baseaddr + reg), val);
}

u32 pl011_read_reg(uart_t *port, u16 reg) {
  return read32((u32*)(port->baseaddr + reg));
}

void pl011_init(uart_t *port) {
  // Disable interrupts (by disabling the device, then clearing all)
  pl011_write_reg(port, REG_CR, 0);
  pl011_write_reg(port, REG_ICR, 0x7FF);

  // Set BAUD rate to 115200
  // (assumed set by firmware- see init_uart_baud in legacy config.txt options for Raspberry Pi)

  // Set mode to 8N1 (8 bits, no parity, 1 stop bit) and disable FIFOs
  pl011_write_reg(port, REG_LCRH, (0b11 << 5));

  // Enable data received interrupt
  pl011_write_reg(port, REG_IMSC, IMSC_INTERRUPT_RECV);

  // Enable UART
  pl011_write_reg(port, REG_CR, CR_ENABLED | CR_RECV_ENABLE | CR_XMIT_ENABLE);
}

void pl011_send(uart_t *port, u8 c) {
  while ( pl011_read_reg(port, REG_FR) & FLAG_XMIT_EMPTY ) { }
  pl011_write_reg(port, REG_DR, c);
}

u8 pl011_recv(uart_t *port) {
  int counter = 0;
  while ( pl011_read_reg(port, REG_FR) & FLAG_RECV_EMPTY ) {
    if (counter++ > RECV_TIMEOUT) return 0;
  }
  return pl011_read_reg(port, REG_DR);
}

bool pl011_has_data(uart_t *port) {
  return (pl011_read_reg(port, REG_FR) & FLAG_RECV_EMPTY) == 0;
}

uart_ops_t pl011_uart_ops = {
  .init = pl011_init,
  .send = pl011_send,
  .recv = pl011_recv,
  .has_data = pl011_has_data,
};
