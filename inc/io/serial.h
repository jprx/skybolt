#pragma once
#include <types.h>
#include <io/intctl.h>

struct uart_ops_t;
struct uart_t;

typedef struct uart_ops_t {
  void (*init)(struct uart_t *);
  void (*send)(struct uart_t *, u8);
  u8   (*recv)(struct uart_t *);
  bool (*has_data)(struct uart_t *);
} uart_ops_t;

typedef struct uart_t {
  uart_ops_t   *ops;
  virt_t        baseaddr;
  irq_t         ext_irq;  // Interrupt number as seen from distributor
  irq_t         core_irq; // Interrupt number as routed to this CPU core
} uart_t;

void init_serial();
void serial_putc(u8);
void serial_puts(char *);
u8   serial_getc();
void serial_intr();
