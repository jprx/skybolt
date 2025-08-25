#pragma once
#include <types.h>
#include <io/serial.h>
#include <io/intctl.h>
#include <io/display.h>

#ifdef PLATFORM_PC
#include <platform/pc.h>
#endif // PLATFORM_PC

#ifdef PLATFORM_RASPI
#include <platform/raspi.h>
#endif // PLATFORM_RASPI

#ifdef PLATFORM_VIRT_RISCV
#include <platform/virt_riscv.h>
#endif // PLATFORM_VIRT_RISCV

// Locate the initial ramdisk in memory, and find a safe place to allocate the
// kernel heap. For some platforms this can be hard-coded, for others it must
// be found using some kind of boot table (eg. multiboot).
void init_memory();
void init_keyboard();

// Structs / addresses we expect every platform to declare:
extern uart_t              g_uart;
extern intctl_t            g_intc;
extern framebuffer_t       g_framebuffer;
extern void               *g_initrd;
extern void               *g_kheap;
