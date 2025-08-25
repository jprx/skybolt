/* Super early bringup: no virtual memory, only physical addresses */
/* This code is mapped wherever physical memory starts */
.text.start   ALIGN(PAGE_SIZE) : { *(.text.start); }

/* This is the higher half, which is where the kernel actually lives (in virtual memory) */
/* Code in .text.start will initialize virtual memory and jump here, never to return */
. += KERNEL_LINK_ADDRESS;
. = ALIGN(PAGE_SIZE);
PROVIDE(_higher_half_load_addr = . - KERNEL_LINK_ADDRESS);
.text.start64 ALIGN(PAGE_SIZE) : AT(ADDR(.text.start64) - KERNEL_LINK_ADDRESS) { *(.text.start64); }
.text         ALIGN(PAGE_SIZE) : AT(ADDR(.text)         - KERNEL_LINK_ADDRESS) { *(.text); *(.text.*); }
.rodata       ALIGN(PAGE_SIZE) : AT(ADDR(.rodata)       - KERNEL_LINK_ADDRESS) { *(.rodata); *(.rodata.*); }
.data         ALIGN(PAGE_SIZE) : AT(ADDR(.data)         - KERNEL_LINK_ADDRESS) { *(.data); *(.data.*); }

.bss ALIGN(PAGE_SIZE) : AT(ADDR(.bss) - KERNEL_LINK_ADDRESS) {
  PROVIDE(_bss_start = .);
  *(.bss); *(.bss.*);
  PROVIDE(_bss_end = .);
}

PROVIDE(_last_kernel_addr = .);

/DISCARD/ : {
  *(.eh_frame);
}
