#include <defs.h>
#include <cpu.h>
#include <lib.h>
#include <io/serial.h>
#include <io/display.h>
#include <stdarg.h>
#include <uname.h>

extern u8 _bss_start, _bss_end;

extern void _printf_internal(char *fmt, va_list *args);

void init_bss() {
  memset(&_bss_start, '\x00', (&_bss_end - &_bss_start));
}

void NORETURN _panic_internal(regs_t *regs, char *reason, va_list *args) {
  cpu_disable_ints();
  display_begin_panic();
  printf("panic: ");
  _printf_internal(reason, args);
  printf("\n");
  if (regs)
    cpu_dump_regs(regs);
  display_finish_panic();
  while(true)
    cpu_halt();
}

void NORETURN panic(char *reason, ...) {
  va_list args;
  va_start(args, reason);
  _panic_internal(NULL, reason, &args);
  va_end(args);
  while(true);
}

void NORETURN panic_regs(regs_t *regs, char *reason, ...) {
  va_list args;
  va_start(args, reason);
  _panic_internal(regs, reason, &args);
  va_end(args);
  while(true);
}

void uname(struct utsname *u) {
  strncpy(u->sysname, KERNEL_NAME, UTSNAMELEN);
  strncpy(u->nodename, "", UTSNAMELEN);
  strncpy(u->release, KERNEL_VARIANT, UTSNAMELEN);
  strncpy(u->version, KERNEL_VERS, UTSNAMELEN);
  strncpy(u->machine, ARCH_NAME, UTSNAMELEN);
}

// @TODO: replace with a much faster version of this
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

void strncpy(char *dst, char *src, usize len) {
  for (usize i = 0; i < len; i++) {
    dst[i] = src[i];
    if ('\x00' == src[i]) return;
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

bool memequal(void *a, void *b, usize len) {
  for (usize i = 0; i < len; i++) {
    if (((u8*)a)[i] != ((u8*)b)[i]) return false;
  }
  return true;
}

bool strprefix(char *s, char *prefix) {
  return memequal(prefix, s, strlen(prefix));
}

bool strequal(char *a, char *b) {
  while (*a && *b) {
    if (*a != *b) return false;
    a++;
    b++;
  }
  return (*a == *b);
}
