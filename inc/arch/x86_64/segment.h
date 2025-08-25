#pragma once
#include <defs.h>
#include <types.h>

/*
 * GDT top:
 * +-----------+
 * |    None   |
 * +-----------+
 * | Kernel CS | <- star.syscall_cs points here
 * +-----------+
 * | Kernel DS |    (syscall_cs + 8)
 * +-----------+
 * |  (Unused) | <- star.syret_cs points here
 * +-----------+
 * |  User DS  |    (sysret_cs + 8)
 * +-----------+
 * |  User CS  |    (sysret_cs + 16)
 * +-----------+
 * |    TSS    | <- task register points here
 * +-----------+
 * | TSS (cont)|
 * +-----------+
 * | (Padding) |
 * +-----------+
 *
 * On syscall, cs <- star.syscall_cs; ds <- star.syscall_cs + 8
 * On sysret,  cs <- star.sysret_cs + 16; ds <- star.sysret_cs + 8
 */
enum {
  GDTENTRY_NONE = 0,
  GDTENTRY_KERNEL_CS,
  GDTENTRY_KERNEL_DS,
  GDTENTRY_UNUSED0,
  GDTENTRY_USER_DS,
  GDTENTRY_USER_CS,
  GDTENTRY_TSS0,
  GDTENTRY_TSS1,
  GDTENTRY_UNUSED1,

  GDT_NUM_ENTRIES,
};

typedef struct __attribute__((packed)) {
  u32 reserved0;
  u64 rsp0;
  u64 rsp1;
  u64 rsp2;
  u64 reserved1;
  u64 ist[7];
  u64 reserved2;
  u16 reserved3;
  u16 iomap_base;
  // WARNING: iomap_base is an offset from the START of the tss, meaning if you
  // set this to zero it will overlap important fields like rsp0. Set this to
  // -1 to put it as far away from us as possible, far beyond the end of the
  // TSS, so we never have to think about it.
} tss_t;

typedef struct __attribute__((packed)) {
  u16 limit0_15;
  u16 base0_15;
  u8  base16_23;
  u8  type : 5, dpl : 2, present : 1;
  u8  limit16_19 : 4, zero0 : 4;
  u8  base24_31;
  u32 base32_63;
  u32 zero1;
} tss_desc_t;

typedef struct __attribute__((packed)) {
  u16 addr0_15;
  u16 cs;
  u8  ist;
  u8  type : 5, dpl : 2, present : 1;
  u16 addr16_31;
  u32 addr32_63;
  u32 zero0;
} idt_entry_t;

#define TSS_MIN_SIZE     ((104))

#define GDT_IDX_TO_SELECTOR(x) (((x) << 3))
#define DPL_KERNEL             ((0b00ull))
#define DPL_USER               ((0b11ull))

#define SELECTOR_TSS     ((GDT_IDX_TO_SELECTOR(GDTENTRY_TSS0)      | DPL_KERNEL))

#define SYSCALL_SELECTOR     ((KERNEL_CS))
#define SYSRET_SELECTOR      ((GDT_IDX_TO_SELECTOR(GDTENTRY_UNUSED0) | DPL_USER))

#define SEGDESC_L         BIT(53)
#define SEGDESC_P         BIT(47)
#define SEGDESC_W         BIT(41)

#define SEGDESC_DATA     ((SEGDESC_P | SEGDESC_W | BIT(44)))
#define SEGDESC_CODE     ((SEGDESC_P | SEGDESC_L | BIT(44) | BIT(43)))

#define SEGDESC_DPL_SHIFT       ((45ull))

#define SYSSEGMENT_TYPE_TSS             0x9

// interrupt gate = we enter with interrupts off (rflags.if cleared)
// trap gate = we enter without changing the interrupt flag
#define SYSSEGMENT_TYPE_INT_GATE        0xE
#define SYSSEGMENT_TYPE_TRAP_GATE       0xF
