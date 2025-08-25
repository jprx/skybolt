#pragma once
#include <defs.h>
#include <types.h>

enum {
  SYS_EXIT     = 0,
  SYS_SPAWN    = 1,
  SYS_OPEN     = 2,
  SYS_READ     = 3,
  SYS_WRITE    = 4,
  SYS_CLOSE    = 5,
  SYS_GETDENTS = 6,
  SYS_UNAME    = 7,

  NUM_SYSCALLS
};

void NORETURN sys_exit(u64 retcode);
u64 sys_spawn(virt_t user_path, u64 flags, virt_t argv);
u64 sys_open(virt_t user_path);
u64 sys_read(u64 fd, virt_t user_buf, usize len);
u64 sys_write(u64 fd, virt_t user_buf, usize len);
u64 sys_close(u64 fd);
u64 sys_getdents(u64 fd, virt_t user_dent);
u64 sys_uname(virt_t user_utsname);

typedef u64 (*syscall_t)(u64, u64, u64, u64, u64);

u64 do_syscall(u64 num, u64 arg0, u64 arg1, u64 arg2, u64 arg3, u64 arg4);
