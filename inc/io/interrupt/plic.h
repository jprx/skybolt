#pragma once
#include <io/intctl.h>

void  plic_init(intctl_t *);
irq_t plic_claim(intctl_t *);
void  plic_eoi(intctl_t *, irq_t);
void  plic_map(intctl_t *, irq_t, irq_t);

void  plic_write(intctl_t *, u32 reg, u32 val);
u32   plic_read(intctl_t *, u32 reg);

extern intctl_ops_t plic_intc_ops;
