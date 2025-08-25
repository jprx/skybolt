#include <tty.h>
#include <lib.h>
#include <task.h>
#include <io/serial.h>
#include <platform.h>

tty_t g_serial_tty;

void init_tty() {
  tty_attach(&g_serial_tty, &g_uart);
}

void tty_attach(tty_t *self, uart_t *port_in) {
  self->buf = alloc_page(PAGE_SMALL);
  memset(self->buf, '\x00', SMALL_PAGE_SIZE);
  self->cursor = 0;
  self->port = port_in;
  self->data_ready = false;
}

usize tty_writeout(tty_t *self, u8 *data, usize len) {
  for (usize i = 0; i < len; i++) {
    // UARTs require "\r\n" instead of "\n"
    // VUARTs (our display driver) will ignore any '\r's we send it
    if ('\n' == data[i])
      self->port->ops->send(self->port, '\r');
    self->port->ops->send(self->port, data[i]);
  }

  return len;
}

usize tty_readin(tty_t *self, u8 *data, usize len) {
  usize n;

  // If multiple readers are waiting for this tty, then we could get woken up
  // after another thread has drained the buffer. May need to sleep again.
  while (!self->data_ready)
    sleep(self);

  // @todo: when len < self->cursor, don't throw away rest of data
  // @todo: handle case when buffer has multiple lines in it
  assert(self->data_ready);
  self->data_ready = false;

  n = min(len, self->cursor);
  memcpy(data, self->buf, n);

  self->cursor = 0;
  return n;
}

void tty_new_data(tty_t *self, u8 c) {
  if ('\r' == c)
    c = '\n'; // Termios ICRNL behavior: convert CR ('\r') into LF ('\n')

  if (ASCII_EOT == c) {
    task_interrupt_by_tty(self);
    return;
  }

  if (ASCII_DELETE == c || '\b' == c) {
    // UARTs use delete for backspace, displays use \b
    if (self->cursor > 0) {
      self->buf[self->cursor--] = '\x00';
      self->port->ops->send(self->port, '\b');
      self->port->ops->send(self->port, ' ');
      self->port->ops->send(self->port, '\b');
    }
    return;
  }

  if (c == '\n')
    self->port->ops->send(self->port, '\r');
  self->port->ops->send(self->port, c);

  if (self->cursor < TTY_BUFIO_LEN - 1)
    self->buf[self->cursor++] = c;

  if ('\n' == c) {
    if (self->cursor == TTY_BUFIO_LEN - 1)
      self->buf[self->cursor] = c;
    self->data_ready = true;
    wakeup(self);
  }
}
