#pragma once
#include <defs.h>
#include <types.h>
#include <cpu.h>

#define __assert_tostr_helper__(x) #x
#define __assert_tostr__(x) __assert_tostr_helper__(x)

#define assert(x) \
  do {                    \
    if (!((x))) { panic("assertion failed: " #x " at " __FILE__ ":" __assert_tostr__(__LINE__)); }  \
  } while(0);

void NORETURN panic(char *reason, ...);
void NORETURN panic_regs(regs_t *r, char *reason, ...);

void init_bss();
void memset(void *b, u8 c, usize len);
void memcpy(void *dst, void *src, usize len);
bool memequal(void *a, void *b, usize len);
bool strequal(char *a, char *b);
bool strprefix(char *s, char *prefix);
void strncpy(char *dst, char *src, usize maxlen);
usize strlen(char *s);
void printf(char *fmt, ...);

static inline u64 min(u64 a, u64 b) {
  return a < b ? a : b;
}

static inline u64 max(u64 a, u64 b) {
  return a > b ? a : b;
}
