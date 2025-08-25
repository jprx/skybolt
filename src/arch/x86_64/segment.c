#include <cpu.h>
#include <lib.h>
#include <arch/x86_64/segment.h>

u64   __attribute__((aligned(16))) gdt[GDT_NUM_ENTRIES];
tss_t __attribute__((aligned(16))) tss;
idt_entry_t __attribute__((aligned(16))) idt[IDT_NUM_ENTRIES];

typedef struct __attribute__((packed)) {
  u16 limit;
  u64 addr;
} segment_table_ptr_t;

void check_selectors() {
  assert(KERNEL_CS == 0x08);
  assert(KERNEL_DS == 0x10);
  assert(USER_CS   == 0x2B);
  assert(USER_DS   == 0x23);
  assert(KERNEL_CS == ((GDT_IDX_TO_SELECTOR(GDTENTRY_KERNEL_CS) | DPL_KERNEL)));
  assert(KERNEL_DS == ((GDT_IDX_TO_SELECTOR(GDTENTRY_KERNEL_DS) | DPL_KERNEL)));
  assert(USER_CS   == ((GDT_IDX_TO_SELECTOR(GDTENTRY_USER_CS)   | DPL_USER)));
  assert(USER_DS   == ((GDT_IDX_TO_SELECTOR(GDTENTRY_USER_DS)   | DPL_USER)));
  assert(SELECTOR_TSS == 0x30);
  assert(GDT_NUM_ENTRIES == 9);
  assert(sizeof(tss) == TSS_MIN_SIZE);
  assert(offsetof(tss_t, rsp0) == TSS_RSP0);
}

void setup_tss_desc(tss_desc_t *desc, tss_t *t, usize sz) {
  assert(sz >= TSS_MIN_SIZE);
  memset(desc, 0, sizeof(*desc));
  desc->limit0_15 = sz & MASK(16);
  desc->limit16_19 = (sz >> 16) & MASK(4);
  desc->base0_15 = ((u64)t) & MASK(16);
  desc->base16_23 = ((u64)t >> 16) & MASK(8);
  desc->base24_31 = ((u64)t >> 24) & MASK(8);
  desc->base32_63 = ((u64)t >> 32) & MASK(32);
  desc->dpl = DPL_KERNEL;
  desc->present = 1;
  desc->type = SYSSEGMENT_TYPE_TSS;
}

void setup_idt_entry(idt_entry_t *entry, void (*handler)(void)) {
  memset(entry, 0, sizeof(*entry));
  entry->addr0_15 = ((u64)handler) & MASK(16);
  entry->addr16_31 = ((u64)handler >> 16ull) & MASK(16);
  entry->addr32_63 = ((u64)handler >> 32ull) & MASK(32);
  entry->cs = KERNEL_CS;
  entry->dpl = DPL_KERNEL;
  entry->present = 1;
  entry->type = SYSSEGMENT_TYPE_INT_GATE; // always enter with interrupts disabled
}

void init_gdt() {
  // Make these volatile, otherwise the compiler may optimize out writes to them!
  // This could result in your limit or base address not being set...
  volatile tss_desc_t tss_desc;
  volatile segment_table_ptr_t gdt_ptr;

  memset(gdt, 0, sizeof(gdt));
  setup_tss_desc((tss_desc_t *)&tss_desc, &tss, sizeof(tss));
  gdt[GDTENTRY_NONE]      = 0;
  gdt[GDTENTRY_KERNEL_CS] = SEGDESC_CODE | (DPL_KERNEL << SEGDESC_DPL_SHIFT);
  gdt[GDTENTRY_KERNEL_DS] = SEGDESC_DATA | (DPL_KERNEL << SEGDESC_DPL_SHIFT);
  gdt[GDTENTRY_UNUSED0]   = 0;
  gdt[GDTENTRY_USER_DS]   = SEGDESC_DATA | (DPL_USER << SEGDESC_DPL_SHIFT);
  gdt[GDTENTRY_USER_CS]   = SEGDESC_CODE | (DPL_USER << SEGDESC_DPL_SHIFT);
  memcpy(&gdt[GDTENTRY_TSS0], (void*)&tss_desc, sizeof(tss_desc));
  gdt[GDTENTRY_UNUSED1]   = 0;

  gdt_ptr.addr  = (u64)&gdt;
  gdt_ptr.limit = sizeof(gdt) - 1;
  asm volatile("lgdt [%0]" :: "r"(&gdt_ptr));
  reload_segments(KERNEL_DS, KERNEL_CS);
}

void init_tss() {
  memset(&tss, 0, sizeof(tss));
  tss.rsp0 = 0;
  tss.iomap_base = -1;
  asm volatile("ltr %0" :: "r"(SELECTOR_TSS));
}

void init_idt() {
  volatile segment_table_ptr_t idt_ptr;
  for (usize i = 0; i < IDT_NUM_ENTRIES; i++) {
    setup_idt_entry(&idt[i], x86_vectors[i]);
  }

  idt_ptr.addr = (u64)&idt;
  idt_ptr.limit = sizeof(idt) - 1;
  asm volatile("lidt [%0]" :: "r"(&idt_ptr));
}

void init_segmentation() {
  check_selectors();
  init_gdt();
  init_tss();
  init_idt();
}
