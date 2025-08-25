#pragma once
#include <vm.h>
#include <cpu.h>
#include <defs.h>
#include <page.h>
#include <file.h>
#include <list.h>

#define MAX_TASKS               2048
#define TASK_NAMELEN            128
#define TASK_NUM_FILES          16
#define TASK_MAX_ARGS           32
#define TASK_ARG_LEN            128

typedef enum {
  THREAD_STATE_NONE = 0,
  THREAD_READY,
  THREAD_SLEEPING,
  THREAD_ZOMBIE,
} thread_state_t;

#define SPAWN_WAIT       0
#define SPAWN_NOWAIT     1

typedef struct thread_t {
  uintptr_t        saved_sp;
  uintptr_t        kstack_pg;
  struct task_t   *task;
  struct listlink  link;
  void            *sleep_reason;
  thread_state_t   state;
  usize            trapdepth;
  regs_t          *uregs;
} thread_t;

typedef struct task_t {
  thread_t         thread;
  ptable_t         pagetable;
  file_t          *fds[TASK_NUM_FILES];
  struct task_t   *parent;
  struct task_t   *waiting_child;
  u64              child_exit_code;
  char             taskname[TASK_NAMELEN];
  cpu_mode_t       taskmode;
  bool             pending_sigint;
  bool             valid;
} task_t;

extern task_t task_pool[MAX_TASKS];

extern struct listlink task_list;

typedef void KTHREAD (*kthread_t)(void);

static inline bool is_thread_ready(thread_t *th) {
  return th->state == THREAD_READY;
}

void          cswitch(thread_t *old_thread, thread_t *new_thread);

void          init_tasks();
void NORETURN init_scheduler();

task_t       *task_spawn_kernel(char *name, char *stdio_device_path, kthread_t entrypoint);
u64           task_spawn_user(char *path, u64 mode, virt_t argv);
void          task_exit(task_t *, u64 reason);

bool          copyin_args(thread_t *th, char **u_argv);
void          setup_user_thread(task_t *task, virt_t entrypoint);
void          setup_kernel_thread(task_t *task, kthread_t entrypoint);

void          free_task(task_t *);
void          free_thread(thread_t *);

void          thread_trap_enter(regs_t *);
void          thread_trap_exit();
thread_t     *current_thread();
task_t       *current_task();

void          task_handle_signals(task_t *);
void          task_interrupt_by_tty(struct tty_t *);
void          task_send_sigkill(task_t *);

void          sleep(void *resource);
void          wakeup(void *resource);
void          scheduler();
void          run_scheduler_if_idle();

