#include <lib.h>
#include <task.h>
#include <tty.h>
#include <list.h>
#include <stdio.h>
#include <graphics.h>
#include <io/display.h>
#include <platform.h>

#define INIT_TASK_NAME "shell"

static thread_t *cur_thread  = NULL;
static task_t   *idle_task = NULL;

struct listlink task_list;

thread_t *current_thread() { return cur_thread; }
task_t   *current_task()   { return cur_thread->task; }

void switchto(thread_t *next) {
  thread_t *prev = cur_thread;
  assert(!cpu_ints_enabled());
  cpu_set_entry_kstack(next);
  cpu_set_pagetable(next->task->pagetable);
  cpu_flush_tlb();
  assert(THREAD_READY == next->state);
  cur_thread = next;
  cswitch(prev, next);
}

void thread_trap_enter(regs_t *r) {
  if (NULL == cur_thread) return;

  cur_thread->trapdepth++;
  if (1 == cur_thread->trapdepth)
    cur_thread->uregs = r;

  assert(cur_thread->trapdepth != 0);

  if (MODE_KERN == cur_thread->task->taskmode) {
    assert(MODE_KERN == get_reg_mode(r));
  }

  if (MODE_USER == cur_thread->task->taskmode) {
    if (1 == cur_thread->trapdepth) {
      assert(MODE_USER == get_reg_mode(r));
    }
    else {
      assert(MODE_KERN == get_reg_mode(r));
    }
  }
}

void thread_trap_exit() {
  if (NULL == cur_thread) return;
  assert(cur_thread->trapdepth != 0);

  cur_thread->trapdepth--;
  if (0 == cur_thread->trapdepth)
    cpu_set_entry_kstack(cur_thread);
}

thread_t *find_runnable_thread() {
  struct listlink *tmp, *pos;

  // 1. Search from current task to end of list
  list_for_each_from(cur_thread->link.next, pos, &task_list) {
    thread_t *cursor = container_of(pos, thread_t, link);
    if (cursor->task == idle_task) continue;
    if (is_thread_ready(cursor))
      return cursor;
  }

  // 2. If we found nothing, scan the whole list from front to end
  list_for_each_safe(pos, tmp, &task_list) {
    thread_t *cursor = container_of(pos, thread_t, link);
    if (cursor->task == idle_task) continue;
    if (is_thread_ready(cursor))
      return cursor;
  }

  // 3. Finally, run the idle task
  assert(is_thread_ready(&idle_task->thread));
  return &idle_task->thread;
}

void reap_zombies() {
  struct listlink *tmp, *pos;
  assert(!cpu_ints_enabled());

  list_for_each_safe(pos, tmp, &task_list) {
    thread_t *cursor = container_of(pos, thread_t, link);
    if (cursor != cur_thread && THREAD_ZOMBIE == cursor->state) {
      list_del(&cursor->link);
      free_task(cursor->task);
    }
  }
}

void scheduler() {
  bool old_ints = cpu_set_ints(false);
  assert(!cpu_ints_enabled());
  reap_zombies();
  switchto(find_runnable_thread());
  task_handle_signals(current_task());
  cpu_set_ints(old_ints);
}

void run_scheduler_if_idle() {
  if (cur_thread->task == idle_task)
    scheduler();
}

void KTHREAD thread_idle() {
  cpu_enable_ints();
  while(true) {
    assert(cpu_ints_enabled());
    cpu_halt();
  }
}

void KTHREAD thread_root() {
  char *args[2];
  args[0] = INIT_TASK_NAME;
  args[1] = NULL;
  while(true) {
    display_reset_tty(current_task()->fds[FD_STDIN]->tty);
    task_spawn_user(INIT_TASK_NAME, SPAWN_WAIT, (uintptr_t)args);
  }
}

void NORETURN init_scheduler() {
  char display_path[32];
  thread_t scratch_thread;
  task_t *root_task;

  assert(offsetof(thread_t, saved_sp) == 0);
  assert(REGS_SIZE  == sizeof(regs_t));
  assert(KREGS_SIZE == sizeof(kregs_t));
  assert(cur_thread == NULL);
  list_init(&task_list);

  root_task = task_spawn_kernel("root", "/dev/serial", thread_root);
  idle_task = task_spawn_kernel("idle", "/dev/null", thread_idle);

  if (is_display_active()) {
    task_spawn_kernel("renderer", "/dev/null", thread_display_update);

    assert(strlen(DISPLAY_DEVICE_PREFIX) + 2 < sizeof(display_path));
    strncpy(display_path, DISPLAY_DEVICE_PREFIX "X", sizeof(display_path));
    for (usize i = 0; i < NUM_DISPLAYS; i++) {
      display_path[strlen(DISPLAY_DEVICE_PREFIX)] = i + '0';
      assert(strprefix(display_path, DISPLAY_DEVICE_PREFIX));
      task_spawn_kernel("display_root", display_path, thread_root);
    }
  }

  cur_thread = &scratch_thread;
  switchto(&root_task->thread);
  panic("init_scheduler returned");
}
