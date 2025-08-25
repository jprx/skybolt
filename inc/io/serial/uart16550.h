#pragma once
#include <io/serial.h>

void uart16550_init(uart_t *port);
void uart16550_send(uart_t *port, u8 c);
u8   uart16550_recv(uart_t *port);
bool uart16550_has_data(uart_t *port);

extern uart_ops_t uart16550_uart_ops;
