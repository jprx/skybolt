#include <io/intctl.h>
#include <io/interrupt/apic.h>
#include <platform.h>
#include <lib.h>
#include <cpu.h>
#include <defs.h>

// External devices are attached to the IOAPIC. The IOAPIC has 24 redirection
// tables that route each interrupt to a given CPU core's LAPIC. LAPICs don't
// have any registers related to external interrupts- if the IOAPIC sends a
// message, the LAPIC will receive it and respond to it. The LAPIC also has a
// table of local interrupts called the "local vector table" just for local
// interrupts (eg the timer).

// To configure an external interrupt, we just need to route it from the IOAPIC
// to a LAPIC (probably LAPIC 0, the bootstrap core). To configure a core local
// interrupt, you use your LAPIC's LVT for that interrupt kind (eg. the timer).

#define I8259A_PIC1_CMD                0x20
#define I8259A_PIC1_DATA               0x21
#define I8259A_PIC2_CMD                0xA0
#define I8259A_PIC2_DATA               0xA1

#define IOAPIC_IOREGSEL                0x0000
#define IOAPIC_IOWIN                   0x0010

#define IOAPIC_REG_ID                  0x00
#define IOAPIC_REG_VER                 0x01
#define IOAPIC_REG_TABLE               0x10

#define IOAPIC_INT_DISABLE             BIT(16)
#define IOAPIC_DEST_SHIFT              24ull

#define LAPIC_INT_DISABLE              BIT(16)
#define APIC_TIMER_PERIODIC            BIT(17)

#define LAPIC_REG_ID                   0x20
#define LAPIC_REG_EOI                  0xB0
#define LAPIC_REG_TPR                  0x80
#define LAPIC_REG_TIMER_LVT            0x320
#define LAPIC_REG_TIMER_INIT_COUNT     0x380

#define IOAPIC_NUM_INTERRUPTS          24

u32 ioapic_read(intctl_t *c, u32 reg) {
  *((volatile u32*)(c->distributor + IOAPIC_IOREGSEL)) = reg;
  return *((volatile u32*)(c->distributor + IOAPIC_IOWIN));
}

void ioapic_write(intctl_t *c, u32 reg, u32 val) {
  *((volatile u32*)(c->distributor + IOAPIC_IOREGSEL)) = reg;
  *((volatile u32*)(c->distributor + IOAPIC_IOWIN)) = val;
}

u32 lapic_read(intctl_t *c, u32 reg) {
  return *((volatile u32*)(c->local + reg));
}

void lapic_write(intctl_t *c, u32 reg, u32 val) {
  *((volatile u32*)(c->local + reg)) = val;
}

// i8259A is the old school interrupt controller, enabled by default.
// Need to turn it off before we can use the APIC.
void legacy_pic_disable() {
  outb(I8259A_PIC1_DATA, 0xFF); // Mask all
  outb(I8259A_PIC2_DATA, 0xFF); // Mask all
  outb(I8259A_PIC1_CMD,  0x20); // EOI
  outb(I8259A_PIC2_CMD,  0x20); // EOI
}

void lapic_enable(intctl_t *c) {
  u64 apic_base;

  legacy_pic_disable();
  apic_base = read_msr(MSR_APIC_BASE);
  assert((apic_base & ~MASK(12)) == LAPIC_DEFAULT_PHYS_ADDR);
  apic_base |= APIC_ENABLE;
  write_msr(MSR_APIC_BASE, apic_base);
}

u32 get_lapic_id(intctl_t *c) {
  return lapic_read(c, LAPIC_REG_ID) >> 24ull;
}

// Maps an IOAPIC interrupt (as viewed from the IOAPIC) to an arbitrary LAPIC interrupt on a given core
void ioapic_enable_irq(intctl_t *c, u32 extnum, u32 lapic_id, u32 intnum) {
  assert(intnum >= 32); // Must not overlap a CPU exception in the IDT
  assert(extnum < IOAPIC_NUM_INTERRUPTS);
  ioapic_write(c, IOAPIC_REG_TABLE + (2 * extnum) + 0, intnum);
  ioapic_write(c, IOAPIC_REG_TABLE + (2 * extnum) + 1, lapic_id << IOAPIC_DEST_SHIFT);
}

void ioapic_disable_irq(intctl_t *c, u32 int_num) {
  ioapic_write(c, IOAPIC_REG_TABLE + (2 * int_num) + 0, IOAPIC_INT_DISABLE);
  ioapic_write(c, IOAPIC_REG_TABLE + (2 * int_num) + 1, 0);
}

void ioapic_init(intctl_t *c) {
  for (usize i = 0; i < IOAPIC_NUM_INTERRUPTS; i++) {
    ioapic_disable_irq(c, i);
  }
}

void lapic_init(intctl_t *c) {
  lapic_enable(c);
  lapic_write(c, LAPIC_REG_TIMER_LVT, LAPIC_TIMER_IRQ | APIC_TIMER_PERIODIC);
  lapic_write(c, LAPIC_REG_TIMER_INIT_COUNT, LAPIC_TIMER_DEADLINE);
  lapic_write(c, LAPIC_REG_EOI, 0);
  lapic_write(c, LAPIC_REG_TPR, 0);
}

void apic_init(intctl_t *c) {
  lapic_init(c);
  ioapic_init(c);
}

irq_t apic_claim(intctl_t *c) {
  return 0;
}

void apic_eoi(intctl_t *c, irq_t irq) {
  lapic_write(c, LAPIC_REG_EOI, 0);
}

void apic_map(intctl_t *c, irq_t extnum, irq_t intnum) {
  assert(get_lapic_id(c) == 0);
  ioapic_enable_irq(c, extnum, get_lapic_id(c), intnum);
}

intctl_ops_t apic_intc_ops = {
  .init  = apic_init,
  .claim = apic_claim,
  .eoi   = apic_eoi,
  .map   = apic_map,
};
