#pragma once
#include <io/serial.h>

void pl011_init(uart_t *port);
void pl011_send(uart_t *port, u8 c);
u8   pl011_recv(uart_t *port);
bool pl011_has_data(uart_t *port);

extern uart_ops_t pl011_uart_ops;
