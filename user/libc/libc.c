#include "../skybolt.h"
#include <stdarg.h>

#define CHARS_PER_U64 16
const char *digits = "0123456789ABCDEF";

void memset(void *b, u8 c, usize len) {
  u8 *region = (u8*)b;
  for (usize i = 0; i < len; i++) {
    region[i] = c;
  }
}

void memcpy(void *dst, void *src, usize len) {
  for (usize i = 0; i < len; i++) {
    ((u8 *)dst)[i] = ((u8 *)src)[i];
  }
}

usize strlen(char *s) {
  char *cursor = s;
  usize len = 0;
  while (*cursor) {
    len++;
    cursor++;
  }
  return len;
}

bool strequal(char *a, char *b) {
  while (*a && *b) {
    if (*a != *b) return false;
    a++;
    b++;
  }
  return (*a == *b);
}

void puts(char *s) {
  write(stdout, s, strlen(s));
}

void fdputs(int fd, char *s) {
  write(fd, s, strlen(s));
}

void putc(char c) {
  write(stdout, &c, 1);
}

void _printf_internal(char *fmt, va_list *args);

void printf(char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  _printf_internal(fmt, &args);
  va_end(args);
}

void _print_hex(u64 val) {
  for (usize i = 0; i < CHARS_PER_U64; i++) {
    putc(digits[val >> 60ull]);
    val <<= 4ull;
  }
}

void _print_str(char *s) {
  puts(s);
}

void _printf_internal(char *fmt, va_list *args) {
  for (char *i = fmt; *i != 0; i++) {
    if (*i != '%') {
      putc(*i);
      continue;
    }

    i++;

    switch (*i) {
      case 'x':
      case 'X':
        _print_hex(va_arg(*args, u64));
        break;

      case 's':
        _print_str(va_arg(*args, char*));
        break;

      default:
        putc('%');
        putc(*i);
        break;
    }
  }
}

void exit(u64 reason) {
  do_syscall(SYS_EXIT, reason);
}

u64 spawn(char *path, u64 flags, char **argv) {
  return do_syscall(SYS_SPAWN, path, flags, argv);
}

int open(char *path) {
  return do_syscall(SYS_OPEN, path);
}

usize read(int fd, void *buf, usize len) {
  return do_syscall(SYS_READ, fd, buf, len);
}

usize write(int fd, void *buf, usize len) {
  return do_syscall(SYS_WRITE, fd, buf, len);
}

u64 close(int fd) {
  return do_syscall(SYS_CLOSE, fd);
}

u64 getdents(int fd, dirent_t *d) {
  return do_syscall(SYS_GETDENTS, fd, d);
}

u64 uname(struct utsname *u) {
  return do_syscall(SYS_UNAME, u);
}
