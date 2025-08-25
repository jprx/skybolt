#include <types.h>
#include <defs.h>
#include <lib.h>
#include <tty.h>
#include <cpu.h>
#include <task.h>
#include <stdio.h>
#include <graphics.h>
#include <platform.h>

void NORETURN kmain() {
  init_bss();
  init_cpu();
  init_memory();
  init_kheap();
  init_vm();
  init_intctl();
  init_serial();
  init_tty();
  init_timer();
  init_files();
  init_display();
  init_stdio();
  init_keyboard();
  init_tasks();
  init_scheduler();

  panic("kmain: init_scheduler returned");
}
