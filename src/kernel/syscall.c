#include <lib.h>
#include <syscall.h>
#include <task.h>
#include <copyio.h>
#include <lib.h>
#include <stdio.h>
#include <uname.h>

syscall_t systab[] = {
  [SYS_EXIT    ]  = (syscall_t)sys_exit,
  [SYS_SPAWN   ]  = (syscall_t)sys_spawn,
  [SYS_OPEN    ]  = (syscall_t)sys_open,
  [SYS_READ    ]  = (syscall_t)sys_read,
  [SYS_WRITE   ]  = (syscall_t)sys_write,
  [SYS_CLOSE   ]  = (syscall_t)sys_close,
  [SYS_GETDENTS]  = (syscall_t)sys_getdents,
  [SYS_UNAME   ]  = (syscall_t)sys_uname,
};

u64 do_syscall(u64 num, u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4) {
  if (num >= NUM_SYSCALLS) return -1;
  return systab[num](arg0, arg1, arg2, arg3, arg4);
}

static bool fd_valid(u64 fd) {
  if (fd >= TASK_NUM_FILES) return false;
  if (NULL == current_task()->fds[fd]) return false;
  return true;
}

i64 find_free_fd(task_t *t) {
  for (usize i = 0; i < TASK_NUM_FILES; i++) {
    if (NULL == current_task()->fds[i]) {
      return i;
    }
  }
  return -1;
}

u64 sys_open(virt_t user_path) {
  file_t *new_file = NULL;
  i64 new_fd = 0;
  if (!access_ok(current_task(), user_path, MAX_PATHLEN)) return -1;
  new_fd = find_free_fd(current_task());
  if (-1 == new_fd) return -2;
  new_file = file_open((char*)user_path);
  if (NULL != new_file) {
    current_task()->fds[new_fd] = new_file;
    return new_fd;
  }
  return -1;
}

u64 sys_read(u64 fd, virt_t user_buf, usize len) {
  if (!fd_valid(fd)) return -1;
  if (!access_ok(current_task(), user_buf, len)) return -2;
  return file_read(current_task()->fds[fd], (u8*)user_buf, len);
}

u64 sys_write(u64 fd, virt_t user_buf, usize len) {
  if (!fd_valid(fd)) return -1;
  if (!access_ok(current_task(), user_buf, len)) return -2;
  return file_write(current_task()->fds[fd], (u8*)user_buf, len);
}

u64 sys_getdents(u64 fd, virt_t user_dent) {
  if (!fd_valid(fd)) return -1;
  if (!access_ok(current_task(), user_dent, sizeof(dirent_t))) return -2;
  return file_getdents(current_task()->fds[fd], (dirent_t*)user_dent);
}

u64 sys_close(u64 fd) {
  if (!fd_valid(fd)) return -1;
  // Disallow closing stdio:
  if (FD_STDIN == fd || FD_STDOUT == fd || FD_STDERR == fd) return -2;
  file_close(current_task()->fds[fd]);
  current_task()->fds[fd] = NULL;
  return 0;
}

void NORETURN sys_exit(u64 retcode) {
  task_exit(current_task(), retcode);
  panic("sys_exit: task_exit returned");
}

u64 sys_spawn(virt_t user_path, u64 flags, virt_t argv) {
  if (!access_ok(current_task(), user_path, MAX_PATHLEN)) return -1;
  if (!access_ok(current_task(), argv, TASK_MAX_ARGS * sizeof(char*))) return -2;
  return task_spawn_user((char *)user_path, flags, argv);
}

u64 sys_uname(virt_t user_utsname) {
  if (!access_ok(current_task(), user_utsname, sizeof(struct utsname))) return -1;
  uname((struct utsname*)user_utsname);
  return 0;
}
