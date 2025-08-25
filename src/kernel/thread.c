#include <defs.h>
#include <lib.h>
#include <task.h>

#define DEFAULT_USER_STACK 0x7FFFF000ull

static void setup_blank_thread(thread_t *th, task_t *t) {
  assert(th == &t->thread);
  th->kstack_pg    = (uintptr_t)alloc_page(PAGE_SMALL);
  th->state        = THREAD_READY;
  th->uregs        = NULL;
  th->task         = t;
  th->sleep_reason = NULL;
  th->saved_sp     = STACK_BOTTOM(th->kstack_pg);
  th->trapdepth    = 0;
  list_init(&th->link);
  assert(0 != th->kstack_pg);
}

void free_thread(thread_t *th) {
  if(0 != th->kstack_pg)
    free_page((void*)th->kstack_pg, PAGE_SMALL);
  th->kstack_pg = (uintptr_t)NULL;
}

/*
 * setup_kernel_thread
 * We need to create a new thread that cswitch can load. cswitch will assume
 * the saved stack pointer points to a kregs_t on the stack. So, we push a
 * kregs_t here and fill in the return address field (the rest don't matter).
 *
 * We use the return address field of the kregs_t to direct where this thread
 * goes when it first gets scheduled. The ret instruction at the end of cswitch
 * will jump to wherever we set the return address. This can be any function in
 * the kernel you want! (Just make sure it never returns, because there's
 * nowhere for it to go after).
 *
 * setup_user_thread is just a special case of this, where the kernel's return
 * address (from kregs_t) is jump_to_user, which then proceeds to pop a full
 * regs_t off the stack and jumps to user code.
 *
 * ┌────┐       ┌────────────────────┐
 * │ sp │─────▶ │                    │ ─┐
 * └────┘       │  Kernel Registers  │  │ popped by
 *              │     (kregs_t)      │  │ cswitch
 *              │                    │ ─┘
 *              └────────────────────┘
 *                    New Stack
 */
void setup_kernel_thread(task_t *t, kthread_t entrypoint) {
  thread_t *th = &t->thread;
  kregs_t *kregs;

  setup_blank_thread(th, t);
  th->task->taskmode = MODE_KERN;

  // Push a kregs_t for cswitch to load
  // The kern_ra field will be where this thread starts running from
  th->saved_sp -= sizeof(kregs_t);
  kregs = (kregs_t *)th->saved_sp;

  memset(kregs, '\x00', sizeof(*kregs));
  kregs->kern_ra = (uintptr_t)entrypoint;
}

/*
 * setup_user_thread:
 * This constructs a thread just like setup_kernel_thread, except instead of
 * jumping to some kernel function, this thread will run jump_to_user right
 * after exiting cswitch.
 *
 * Therefore, we push two sets of registers: the top set is the usual set of
 * registers that cswitch loads (a kregs_t). Right below it we push a full
 * regs_t that jump_to_user will pop.
 *
 * You can think of this as a special case of a kernel thread that only does
 * one thing: it immediately jumps to userspace.
 *
 * ┌────┐       ┌────────────────────┐
 * │ sp │─────▶ │                    │ ─┐
 * └────┘       │  Kernel Registers  │  │ popped by
 *              │     (kregs_t)      │  │ cswitch
 *              │                    │ ─┘
 *              ├────────────────────┤
 *              │                    │ ─┐
 *              │                    │  │
 *              │                    │  │
 *              │                    │  │
 *              │                    │  │
 *              │   User Registers   │  │ popped by
 *              │      (regs_t)      │  │ jump_to_user
 *              │                    │  │
 *              │                    │  │
 *              │                    │  │
 *              │                    │  │
 *              │                    │ ─┘
 *              └────────────────────┘
 *                   New thread's
 *                   kernel stack
 */
void setup_user_thread(task_t *t, virt_t entrypoint) {
  thread_t *th = &t->thread;
  kregs_t *kregs;
  regs_t *uregs;

  setup_blank_thread(th, t);
  th->task->taskmode = MODE_USER;

  assert(!is_page_mapped(t->pagetable, DEFAULT_USER_STACK));

  map_page(
    t->pagetable,
    DEFAULT_USER_STACK,
    KERN_V2P(alloc_page(PAGE_SMALL)),
    PAGE_SMALL,
    MODE_USER
  );

  // Push the user registers that jump_to_user will use
  th->saved_sp -= sizeof(regs_t);
  uregs = (regs_t *)th->saved_sp;

  // Push the kernel registers that cswitch will use
  th->saved_sp -= sizeof(kregs_t);
  kregs = (kregs_t *)th->saved_sp;

  memset(uregs, '\x00', sizeof(*uregs));
  memset(kregs, '\x00', sizeof(*kregs));

  th->uregs = uregs;
  init_user_regs(th->uregs, entrypoint, STACK_BOTTOM(DEFAULT_USER_STACK));

  // This tells cswitch to run jump_to_user when it returns
  kregs->kern_ra = (u64)jump_to_user;
}
