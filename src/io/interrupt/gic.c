#include <defs.h>
#include <lib.h>
#include <platform.h>
#include <io/interrupt/gic.h>

// Distributor regs
#define GICD_CTLR            0x0000
#define GICD_ISENABLER       0x0100
#define GICD_ITARGET         0x0800

// Core regs
#define GICC_CTLR    0x0000
#define GICC_PMR     0x0004
#define GICC_IAR     0x000C
#define GICC_EOIR    0x0010

#define GIC_FIRST_SPI_NUM 32

// Higher number = lower priority
#define PMR_PRIORITY_ALLOW_ALL     MASK(8)

#define CTLR_ENABLE                BIT(0)
#define ITARGET_CORE0              BIT(0)

u32 gicd_read(intctl_t *c, u32 reg) { return *(volatile u32*)(c->distributor + reg); }
u8  gicd_read8(intctl_t *c, u32 reg) { return *(volatile u8*)(c->distributor + reg); }
u32 gicc_read(intctl_t *c, u32 reg) { return *(volatile u32*)(c->local + reg); }
void gicd_write(intctl_t *c, u32 reg, u32 val) { *(volatile u32*)(c->distributor + reg) = val; }
void gicd_write8(intctl_t *c, u32 reg, u8 val) {  *(volatile u8*)(c->distributor + reg) = val; }
void gicc_write(intctl_t *c, u32 reg, u32 val) { *(volatile u32*)(c->local + reg) = val; }

void gic_init(intctl_t *c) {
  gicd_write(c, GICD_CTLR, CTLR_ENABLE);
  gicc_write(c, GICC_CTLR, CTLR_ENABLE);
  gicc_write(c, GICC_PMR,  PMR_PRIORITY_ALLOW_ALL);
}

irq_t gic_claim(intctl_t *c) {
  return gicc_read(c, GICC_IAR);
}

void gic_eoi(intctl_t *c, irq_t irq) {
  gicc_write(c, GICC_EOIR, irq);
}

void gic_map(intctl_t *c, irq_t extnum, irq_t intnum) {
  // GIC doesn't support redirecting vector numbers,
  // so check the caller expects them to be the same.
  assert(extnum == intnum);

  gicd_write(c, GICD_ISENABLER + 4 * (intnum / 32), BIT(intnum % 32));
  gicd_write8(c, GICD_ITARGET + intnum, ITARGET_CORE0);
}

intctl_ops_t gic_intc_ops = {
  .init  = gic_init,
  .claim = gic_claim,
  .eoi   = gic_eoi,
  .map   = gic_map,
};
