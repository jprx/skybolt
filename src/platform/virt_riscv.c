#include <defs.h>
#include <lib.h>
#include <platform.h>
#include <io/serial/uart16550.h>
#include <io/interrupt/plic.h>

extern u8 _kernel_base_phys_addr;
extern u8 _last_kernel_addr;

void *g_initrd = NULL;
void *g_kheap = NULL;

uart_t g_uart = {
  .ops          = &uart16550_uart_ops,
  .baseaddr     = KERN_P2DV(0x10000000),
  .ext_irq      = UART_IRQ,
  .core_irq     = UART_IRQ,
};

intctl_t g_intc = {
  .ops          = &plic_intc_ops,
  .distributor  = KERN_P2DV(0xC000000),
  .local        = 0, // unused on PLIC
};

framebuffer_t g_framebuffer = {
  .framebuffer = NULL,
  .pxwidth = 0,
  .pxheight = 0,
};

void init_keyboard() {
  // No keyboard for this platform (yet)
}

void init_memory() {
  // Qemu tries to load the ramdisk at 512MB from the top of RAM (qemu:v9.1.0/hw/riscv/boot.c:198)
  g_initrd = (void*)(KERN_P2V((512 * ONE_MB) + (u64)&_kernel_base_phys_addr));
  g_kheap = (void*)((u64)g_initrd + MAX_INITRD_SIZE);
  assert((uintptr_t)g_initrd > (uintptr_t)&_last_kernel_addr);
}
