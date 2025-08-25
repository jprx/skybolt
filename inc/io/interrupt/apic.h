#pragma once
#include <vm.h>
#include <platform.h>
#include <io/intctl.h>

#define LAPIC_DEFAULT_PHYS_ADDR     0xFEE00000ull
#define IOAPIC_DEFAULT_PHYS_ADDR    0xFEC00000ull

void  apic_init(intctl_t *);
irq_t apic_claim(intctl_t *);
void  apic_eoi(intctl_t *, irq_t);
void  apic_map(intctl_t *, irq_t, irq_t);

void  ioapic_init(intctl_t *);
u32   ioapic_read(intctl_t *, u32 reg);
void  ioapic_write(intctl_t *, u32 reg, u32 val);

void  lapic_enable(intctl_t *);
void  lapic_init(intctl_t *);
u32   lapic_read(intctl_t *, u32 reg);
void  lapic_write(intctl_t *, u32 reg, u32 val);

extern intctl_ops_t apic_intc_ops;
