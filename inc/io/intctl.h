#pragma once
#include <types.h>

// Usually interrupt controllers consist of a global distributor shared between
// all cores + a core-local interrupt controller for each core. We call the
// global distributor peripheral the "distributor" and the core local
// controller the "local" controller. Example: for X86_64, the IOAPIC is the
// distributor, and the LAPIC for each core is the local controller.

struct intctl_ops_t;
struct intctl_t;
typedef u64 irq_t;

typedef struct intctl_ops_t {
  void     (*init)(struct intctl_t *);
  irq_t    (*claim)(struct intctl_t *);
  void     (*eoi)(struct intctl_t *, irq_t);
  void     (*map)(struct intctl_t *, irq_t, irq_t);
} intctl_ops_t;

typedef struct intctl_t {
  intctl_ops_t        *ops;
  virt_t               distributor;
  virt_t               local;
} intctl_t;

void     timer_intr();

void     init_intctl();
irq_t    claim_interrupt();
void     send_eoi(irq_t);
