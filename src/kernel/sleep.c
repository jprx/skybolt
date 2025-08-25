#include <task.h>
#include <lib.h>
#include <cpu.h>

void sleep(void *resource) {
  thread_t *th = current_thread();
  if (NULL == current_thread())
    panic("sleep: tried to sleep but there isn't a thread yet");

  th->state = THREAD_SLEEPING;
  th->sleep_reason = resource;
  scheduler();
  assert(THREAD_READY == th->state);
  assert(current_thread() == th);
  assert(NULL == th->sleep_reason);
}

void wakeup(void *resource) {
  struct listlink *pos;
  if (NULL == current_thread())
    return;

  assert(!cpu_ints_enabled());
  list_for_each(pos, &task_list) {
    thread_t *cursor = container_of(pos, thread_t, link);
    if (THREAD_SLEEPING == cursor->state && cursor->sleep_reason == resource) {
      cursor->sleep_reason = NULL;
      cursor->state = THREAD_READY;
    }
  }
}
