#pragma once
#include <vm.h>
#include <file.h>
#include <task.h>

// Returns true on success, false on failure
bool access_ok(task_t *, virt_t user_va, usize len);
bool copyin(task_t *, void *kern_buf, virt_t user_va, usize len);
bool copyout(task_t *, virt_t user_va, void *kern_buf, usize len);
