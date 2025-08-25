#include <platform.h>
#include <io/serial.h>

void init_serial() {
  g_uart.ops->init(&g_uart);
}

void serial_putc(u8 c) {
  if (c == '\n') g_uart.ops->send(&g_uart, '\r');
  g_uart.ops->send(&g_uart, c);
}

void serial_puts(char *s) {
  char *cursor = s;
  while (*cursor) {
    serial_putc(*cursor);
    cursor++;
  }
}

u8 serial_getc() {
  return g_uart.ops->recv(&g_uart);
}
