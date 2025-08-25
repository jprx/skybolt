#include <lib.h>
#include <graphics.h>
#include <io/display.h>

// A vuart is a "virtual UART" that acts as a sink for writing characters to a
// display. Reading data from a vuart is undefined; you can only send data to
// it. The intention is this is attached to a tty for each virtual desktop the
// system needs. Input data for a GUI system comes from the keyboard, which is
// separate from the virtual UART device.

void vuart_init(uart_t *port) {
  return;
}

void vuart_send(uart_t *port, u8 c) {
  if ('\r' == c) return;
  display_putc((display_t*)port->baseaddr, c);
}

u8 vuart_recv(uart_t *port) {
  panic("vuart_recv: cannot receive from a vuart");
}

bool vuart_has_data(uart_t *port) {
  panic("vuart_has_data: undefined for a vuart");
}

uart_ops_t vuart_ops = {
  .init     = vuart_init,
  .send     = vuart_send,
  .recv     = vuart_recv,
  .has_data = vuart_has_data,
};
