#include "skybolt.h"

void busyloop() {
  for (usize i = 0; i < 100; i++) {
    asm volatile("nop");
  }
}

int main() {
  usize i = 0;
  while(true) {
    busyloop();
    printf("%x\n", i++);
  }
  return 0;
}
