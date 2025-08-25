#include <vm.h>
#include <lib.h>
#include <platform.h>
#include <io/display.h>
#include <io/serial/uart16550.h>
#include <io/interrupt/apic.h>
#include <io/ps2.h>
#include <platform/multiboot.h>

extern u64 *multiboot_info_ptr;

extern u8 _last_kernel_addr;

void *g_initrd = NULL;
void *g_kheap = NULL;

uart_t g_uart = {
  .ops      = &uart16550_uart_ops,
  .baseaddr = 0x3F8,
  .ext_irq  = 4,
  .core_irq = UART_IRQ,
};

intctl_t g_intc = {
  .ops         = &apic_intc_ops,
  .distributor = KERN_P2DV(0xFEC00000ull),
  .local       = KERN_P2DV(0xFEE00000ull),
};

framebuffer_t g_framebuffer = {
  .framebuffer = NULL,
  .pxwidth = 0,
  .pxheight = 0,
};

void init_keyboard() {
  init_i8042();
}

void init_memory() {
  multiboot_info_t *mb_info;
  multiboot_mod_t *boot_module;
  mb_info = (multiboot_info_t*)KERN_P2V(multiboot_info_ptr);

  if (0 != mb_info->framebuffer_addr) {
    g_framebuffer.framebuffer = (void*)KERN_P2WC(mb_info->framebuffer_addr);
    g_framebuffer.pxwidth = mb_info->framebuffer_width;
    g_framebuffer.pxheight = mb_info->framebuffer_height;
  }

  assert(NULL != multiboot_info_ptr);
  assert(mb_info->mods_count == 1);
  assert(0 != mb_info->mods_addr);

  boot_module = (multiboot_mod_t*)(KERN_P2V(mb_info->mods_addr));
  assert(0 != boot_module->mod_start);
  assert(0 != boot_module->mod_end);

  // @TODO: this wastes memory, instead we should put the initrd right
  // at the large page nearest the end of the initrd. For now we space things
  // out more though to prevent any off by one errors.
  g_initrd = (void*)KERN_P2V(boot_module->mod_start);
  g_kheap = (void*)(ALIGN_LARGE_PAGE((u64)g_initrd + MAX_INITRD_SIZE));
  assert((uintptr_t)g_initrd > (uintptr_t)&_last_kernel_addr);
}
