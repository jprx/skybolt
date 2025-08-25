ENTRY(bootup);

KERNEL_LOAD_ADDRESS = 1M;                 /* Where the kernel is loaded in physical memory */
KERNEL_LINK_ADDRESS = 0xFFFFFF0000000000; /* Where the kernel will live in virtual memory */

MULTIBOOT_MAGIC = 0x1BADB002; /* Multiboot 1 */
MULTIBOOT_FLAGS = 4;          /* flags[2] - set video mode */
MULTIBOOT_CHECKSUM = ((-(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)));

PAGE_SIZE = 4K;
LARGE_PAGE_SIZE = 2M;

SECTIONS {
  . = KERNEL_LOAD_ADDRESS;
  PROVIDE(_kernel_base_phys_addr = .);

  .multiboot_header : ALIGN(4) {
    /* Multiboot 1 Header, required for Qemu's -kernel or GRUB to load us on X86_64 systems */
    /* See: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html */
    LONG(MULTIBOOT_MAGIC);
    LONG(MULTIBOOT_FLAGS);
    LONG(MULTIBOOT_CHECKSUM);

    /* Header Address Fields (unused) */
    LONG(0);
    LONG(0);
    LONG(0);
    LONG(0);
    LONG(0);

    /* Graphics Information */
    LONG(0);     /* Mode type 0 = linear framebuffer */
    LONG(1920);  /* Framebuffer width */
    LONG(1080);  /* Framebuffer height */
    LONG(32);    /* Framebuffer depth (bpp) */
  }

  INCLUDE src/link/default.ld
}
