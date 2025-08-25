ENTRY(bootup);

KERNEL_LOAD_ADDRESS = 0x80000;            /* Where the kernel is loaded in physical memory */
KERNEL_LINK_ADDRESS = 0xFFFFFF0000000000; /* Where the kernel will live in virtual memory */

PAGE_SIZE = 4K;
LARGE_PAGE_SIZE = 2M;

SECTIONS {
  . = KERNEL_LOAD_ADDRESS;
  PROVIDE(_kernel_base_phys_addr = .);
  INCLUDE src/link/default.ld
}

