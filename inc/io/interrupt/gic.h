#pragma once
#include <io/intctl.h>

void  gic_init(intctl_t *);
irq_t gic_claim(intctl_t *);
void  gic_eoi(intctl_t *, irq_t);
void  gic_map(intctl_t *, irq_t, irq_t);

u32   gicd_read(intctl_t *, u32);
u8    gicd_read8(intctl_t *, u32);
void  gicd_write(intctl_t *, u32, u32);
void  gicd_write8(intctl_t *, u32, u8);

u32   gicc_read(intctl_t *, u32);
void  gicc_write(intctl_t *, u32, u32);

extern intctl_ops_t gic_intc_ops;
