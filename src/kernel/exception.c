#include <lib.h>
#include <task.h>
#include <stdio.h>
#include <file.h>
#include <exception.h>

#define CRASH_MESSAGE "Segmentation Fault\n"

void do_exception(regs_t *regs, exception_kind_t kind, u64 error_code) {
  if (get_reg_mode(regs) == MODE_USER) {
    // Exception caused by userspace; exit this task
    assert(1 == current_thread()->trapdepth);
    task_t *t = current_task();
    file_write(t->fds[FD_STDOUT], CRASH_MESSAGE, strlen(CRASH_MESSAGE));
    task_exit(t, 1);
  }

  switch(kind) {
    case EX_PAGEFAULT:
      panic_regs(regs, "Page Fault: 0x%X", error_code);
      break;

    case EX_UNKNOWN:
      panic_regs(regs, "Unknown CPU exception");
      break;
  }
}
