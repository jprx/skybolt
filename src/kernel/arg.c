#include <cpu.h>
#include <lib.h>
#include <task.h>
#include <copyio.h>

#define DEFAULT_ARG_LOCATION 0x70000000ull

#define ARG_IDX_ARGC         0
#define ARG_IDX_ARGV         1

bool copyin_args(thread_t *th, char **argv_in) {
  usize argc = 0;
  bool success = false;
  virt_t arg_page = (virt_t)alloc_page(PAGE_SMALL);
  char **argv_out = (char**)arg_page;
  char *out_str_cursor = (char*)(arg_page + (sizeof(char*) * TASK_MAX_ARGS));

  assert(!is_page_mapped(th->task->pagetable, DEFAULT_ARG_LOCATION));
  assert(current_task() == th->task->parent);
  assert(NULL != th->uregs);
  assert(NULL != th->task);
  assert(NULL != th->task->parent);

  map_page(th->task->pagetable, DEFAULT_ARG_LOCATION, KERN_V2P(arg_page), PAGE_SMALL, MODE_USER);

  for (usize i = 0; i < TASK_MAX_ARGS; i++)
    argv_out[i] = NULL;

  if ((virt_t)0 == argv_in)
    goto out;

  while(NULL != argv_in[argc]) {
    success = copyin(th->task->parent, out_str_cursor, (virt_t)argv_in[argc], TASK_ARG_LEN);
    if (!success) return false;
    argv_out[argc] = (char*)((uintptr_t)out_str_cursor - arg_page + DEFAULT_ARG_LOCATION);
    out_str_cursor += strlen(argv_in[argc]) + 1;
    out_str_cursor = (char*)ROUND_UP_POW2((uintptr_t)out_str_cursor, 8);
    argc++;
  }

out:
  set_reg_arg(th->uregs, ARG_IDX_ARGC, argc);
  set_reg_arg(th->uregs, ARG_IDX_ARGV, (uintptr_t)DEFAULT_ARG_LOCATION);
  return true;
}
