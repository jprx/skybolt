#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_PATHLEN 255

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef size_t   usize;

typedef enum {
  SYS_EXIT     = 0,
  SYS_SPAWN    = 1,
  SYS_OPEN     = 2,
  SYS_READ     = 3,
  SYS_WRITE    = 4,
  SYS_CLOSE    = 5,
  SYS_GETDENTS = 6,
  SYS_UNAME    = 7,
} syscall_t;

#define stdin  0
#define stdout 1
#define stderr 2

#define SPAWN_WAIT       0
#define SPAWN_NOWAIT     1

typedef struct {
  u8   filetype;
  char name[MAX_PATHLEN + 1];
} dirent_t;

#define UTSNAMELEN 64

struct utsname {
  char sysname[UTSNAMELEN];
  char nodename[UTSNAMELEN];
  char release[UTSNAMELEN];
  char version[UTSNAMELEN];
  char machine[UTSNAMELEN];
};

extern uint64_t do_syscall(uint64_t, ...);

// libc
void  puts(char *);
void  fdputs(int, char *);
void  memset(void *b, u8 c, usize len);
usize strlen(char *s);
bool  strequal(char *a, char *b);
void  memcpy(void *dst, void *src, usize len);
void  printf(char *format, ...);

// syscalls
void  exit(u64 reason);
u64   spawn(char *path, u64 mode, char **argv);
int   open(char *);
usize read(int fd, void *buf, usize len);
usize write(int fd, void *buf, usize len);
u64   close(int fd);
u64   getdents(int fd, dirent_t *d);
u64   uname(struct utsname*);
