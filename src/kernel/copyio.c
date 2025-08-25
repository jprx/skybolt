#include <file.h>
#include <lib.h>
#include <copyio.h>

bool access_ok(task_t *t, virt_t user_va, usize len) {
  virt_t user_cursor = user_va;

  while(user_cursor < user_va + len) {
    usize pgsize = get_page_size(t->pagetable, user_cursor);

    // Only enforce user VA for usermode tasks
    // (kernel threads are allowed to use kernel addrs of course)
    if (t->taskmode == MODE_USER)
      if (!is_user_va(user_cursor)) return false;

    if (!is_page_mapped(t->pagetable, user_cursor)) return false;
    user_cursor += pgsize - (user_cursor % pgsize);
  }

  return true;
}

bool copyin(task_t *t, void *kern_buf, virt_t user_va, usize len) {
  if (!access_ok(t, user_va, len))
    return false;

  memcpy(kern_buf, (u8*)user_va, len);
  return true;
}

bool copyout(task_t *t, virt_t user_va, void *kern_buf, usize len) {
  if (!access_ok(t, user_va, len))
    return false;

  memcpy((u8*)user_va, kern_buf, len);
  return true;
}
