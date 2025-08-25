#include <io/interrupt/plic.h>
#include <lib.h>

// S-mode on hart 0 is "context 1"
#define PLIC_CONTEXT            1
#define PLIC_NUM_INTERRUPTS     1024

#define REG_PRIORITY               ((0x000000))
#define REG_ENABLE(c)              ((0x002000 + ((c) * 0x80)))
#define REG_PRIORITY_THRESH(c)     ((0x200000 + ((c) * 0x1000)))
#define REG_CLAIM(c)               (((REG_PRIORITY_THRESH(c) + 4)))

void plic_write(intctl_t *c, u32 reg, u32 val) { *(volatile u32*)(c->distributor + reg) = val; }
u32 plic_read(intctl_t *c, u32 reg) { return *(volatile u32*)(c->distributor + reg); }

void plic_init(intctl_t *c) {
  plic_write(c, REG_PRIORITY_THRESH(PLIC_CONTEXT), 0);
}

irq_t plic_claim(intctl_t *c) {
  return plic_read(c, REG_CLAIM(PLIC_CONTEXT));
}

void plic_eoi(intctl_t *c, irq_t irq) {
  plic_write(c, REG_CLAIM(PLIC_CONTEXT), irq);
}

void plic_map(intctl_t *c, irq_t extnum, irq_t intnum) {
  assert(extnum == intnum);
  plic_write(c, REG_PRIORITY + (4 * intnum), 1);
  plic_write(c, REG_ENABLE(PLIC_CONTEXT) + (intnum / 32), BIT(intnum % 32));
}

intctl_ops_t plic_intc_ops = {
  .init  = plic_init,
  .claim = plic_claim,
  .eoi   = plic_eoi,
  .map   = plic_map
};
