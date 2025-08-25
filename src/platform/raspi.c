#include <defs.h>
#include <lib.h>
#include <platform.h>
#include <io/serial/pl011.h>
#include <io/interrupt/gic.h>

// Assuming the Raspberry Pi is operating in low peripheral mode

extern u8 _last_kernel_addr;

void *g_initrd = NULL;
void *g_kheap = NULL;

uart_t g_uart = {
  .ops         = &pl011_uart_ops,
  .baseaddr    = KERN_P2DV(0xFE201000),
  .ext_irq     = UART_IRQ,
  .core_irq    = UART_IRQ,
};

intctl_t g_intc = {
  .ops         = &gic_intc_ops,
  .distributor = KERN_P2DV(0xFF841000),
  .local       = KERN_P2DV(0xFF842000)
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
  // Qemu tries to load the ramdisk at 128MB (qemu:v9.1.0/hw/arm/boot.c:1031)
  // We use config.txt to load the initrd here too on real Pis
  g_initrd = (void*)(KERN_P2V(128 * ONE_MB));
  g_kheap = (void*)((u64)g_initrd + MAX_INITRD_SIZE);
  assert((uintptr_t)g_initrd > (uintptr_t)&_last_kernel_addr);
}
