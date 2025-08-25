#include <vm.h>
#include <file.h>
#include <lib.h>
#include <copyio.h>
#include <fs/tar.h>
#include <task.h>
#include <stdio.h>
#include <loader/elf.h>

task_t task_pool[MAX_TASKS];

// Private task creation helpers
static task_t *create_empty_task(char *name);
static task_t *create_user_task_elf(char *exe);

void init_tasks() {
  for (usize i = 0; i < MAX_TASKS; i++) {
    task_pool[i].valid = false;
  }
}

static task_t *create_empty_task(char *name) {
  task_t *t = NULL;

  for (usize i = 0; i < MAX_TASKS; i++) {
    if (!task_pool[i].valid) {
      t = &task_pool[i];
      memset(t, '\x00', sizeof(*t));
      task_pool[i].valid = true;
      break;
    }
  }

  assert(NULL != t);

  for (usize i = 0; i < TASK_NUM_FILES; i++)
    t->fds[i] = NULL;

  t->parent = NULL;
  t->waiting_child = NULL;
  t->child_exit_code = 0;
  t->pending_sigint = false;
  t->thread.state = THREAD_STATE_NONE;
  t->taskmode = CPU_MODE_UNDEFINED;
  t->pagetable = alloc_pagetable();
  assert(NULL != t->pagetable);

  map_kernel(t->pagetable);
  strncpy(t->taskname, name, TASK_NAMELEN);
  return t;
}

static task_t *create_user_task_elf(char *exe) {
  file_t *elf;
  task_t *t;
  bool load_success = false;

  elf = file_open(exe);
  if (NULL == elf) return NULL;

  t = create_empty_task(exe);
  assert(NULL == t->fds[FD_STDIN]);
  assert(NULL == t->fds[FD_STDOUT]);
  assert(NULL == t->fds[FD_STDERR]);

  load_success = load_elf(t->pagetable, elf);
  if (!load_success) {
    free_task(t);
    return NULL;
  }

  setup_user_thread(t, elf_entrypoint(elf));
  file_close(elf);
  return t;
}

task_t *task_spawn_kernel(char *name, char *stdio_device_path, kthread_t entrypoint) {
  task_t *new_task = create_empty_task(name);
  if (!new_task) return NULL;
  new_task->fds[FD_STDIN]  = file_open(stdio_device_path);
  new_task->fds[FD_STDOUT] = file_open(stdio_device_path);
  new_task->fds[FD_STDERR] = file_open(stdio_device_path);
  setup_kernel_thread(new_task, entrypoint);
  list_add(&new_task->thread.link, &task_list);
  return new_task;
}

u64 task_spawn_user(char *path, u64 mode, virt_t argv) {
  task_t *cur_task = current_task();
  task_t *new_task = create_user_task_elf(path);

  if (!new_task) return -1;

  new_task->parent = cur_task;
  new_task->fds[FD_STDIN]  = file_dup(cur_task->fds[FD_STDIN]);
  new_task->fds[FD_STDOUT] = file_dup(cur_task->fds[FD_STDOUT]);
  new_task->fds[FD_STDERR] = file_dup(cur_task->fds[FD_STDERR]);
  assert(NULL != new_task->fds[FD_STDIN ]);
  assert(NULL != new_task->fds[FD_STDOUT]);
  assert(NULL != new_task->fds[FD_STDERR]);

  if (!copyin_args(&new_task->thread, (char**)argv)) {
    new_task->thread.state = THREAD_ZOMBIE;
    free_task(new_task);
    return -1;
  }

  list_add(&new_task->thread.link, &task_list);

  if (mode == SPAWN_NOWAIT) {
    scheduler();
    return 0;
  }

  cur_task->waiting_child = new_task;
  cur_task->thread.state = THREAD_SLEEPING;
  sleep(&cur_task->waiting_child);

  assert(cur_task == current_task());
  assert(THREAD_READY == cur_task->thread.state);
  assert(NULL == cur_task->waiting_child);
  return cur_task->child_exit_code;
}

void task_interrupt_by_tty(struct tty_t *tty_to_find) {
  struct listlink *pos, *tmp;

  list_for_each_safe(pos, tmp, &task_list) {
    thread_t *cursor = container_of(pos, thread_t, link);

    if (!is_thread_ready(cursor))
      continue;

    if (NULL == cursor->task->fds[FD_STDIN])
      continue;

    if (tty_to_find != cursor->task->fds[FD_STDIN]->tty)
      continue;

    task_send_sigkill(cursor->task);
  }
}

void task_send_sigkill(task_t *t) {
  t->pending_sigint = true;
}

void task_handle_signals(task_t *t) {
  if (t->pending_sigint) {
    assert(current_task() == t);
    task_exit(t, 0);
  }
}

void free_task(task_t *t) {
  struct listlink *pos;
  assert(current_task() != t);

  list_for_each(pos, &task_list) {
    thread_t *cursor = container_of(pos, thread_t, link);
    if (cursor->task->parent == t)
      cursor->task->parent = NULL;
  }

  for (usize i = 0; i < TASK_NUM_FILES; i++) {
    if (t->fds[i])
      file_close(t->fds[i]);
  }

  unmap_kernel(t->pagetable);
  free_pagetable(t->pagetable);
  free_thread(&t->thread);
  t->taskmode = CPU_MODE_UNDEFINED;
  t->valid = false;
}

void task_exit(task_t *t, u64 reason) {
  if (NULL != t->parent && THREAD_SLEEPING == t->parent->thread.state && t == t->parent->waiting_child) {
    t->parent->waiting_child = NULL;
    t->parent->child_exit_code = reason;
    wakeup(&t->parent->waiting_child);
  }
  t->thread.state = THREAD_ZOMBIE;

  scheduler();
  if (current_task() == t)
    panic("task_exit: scheduler() returned to a task that should have been deleted");
}
