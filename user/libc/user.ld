ENTRY(start);

PAGE_SIZE = 0x1000;
USER_BASE = 0x400000;

SECTIONS {
  /* Put text contents right after the ELF header. This shrinks the output
  * binary as it combines the ELF header and text into one segment, rather than
  * adding padding */
  . = USER_BASE + SIZEOF_HEADERS;

  .text    : { *(.text); *(.text.*); }
  .rodata  : { *(.rodata); *(.rodata.*); }
  .data    : { *(.data); *(.data.*); }
  .bss     : { *(.bss); *(.bss.*); }

  /DISCARD/ : {
    *(.eh_frame);
    *(.comment);
  }
}
