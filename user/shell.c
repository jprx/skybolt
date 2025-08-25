#include "skybolt.h"

#define MAX_ARGS 32

int main(int argc, char **argv) {
  char buf[1024];
  char *args[MAX_ARGS];
  usize n;
  i64 rval;
  while (true) {
    puts("$ ");
    memset(buf, '\x00', sizeof(buf));
    n = read(stdin, buf, sizeof(buf));

    if (0 == n) {
      puts("shell: error reading from stdin\n");
      return 1;
    }

    if (buf[n-1] != '\n') {
      puts("shell: input was not a full line\n");
      continue;
    }

    buf[n-1] = '\x00'; // replace '\n' with '\x00'

    if (strequal(buf, "exit")) {
      return 0;
    }

    if (strequal(buf, "crash")) {
      *(u32*)(0xDEADC0DE) = 1234;
      puts("shell: you should never see this (1)");
    }

    if (strequal(buf, "panic")) {
      open("/dev/panic1");
      puts("shell: you should never see this (2)");
    }

    if (strequal(buf, "assertfail")) {
      open("/dev/panic2");
      puts("shell: you should never see this (2)");
    }

    if (strequal(buf, "")) continue;

    for (usize i = 0; i < MAX_ARGS; i++)
      args[i] = NULL;

    char *cursor = buf;
    args[0] = cursor;
    usize arg_counter = 1;
    while (*cursor) {
      if (' ' == *cursor && (' ' != *(cursor+1) && '\x00' != *(cursor+1)))
        args[arg_counter++] = cursor + 1;

      if (' ' == *cursor)
        *cursor = '\x00';

      if (arg_counter >= MAX_ARGS)
        break;
      cursor++;
    }

    rval = spawn(buf, SPAWN_WAIT, args);
    if (0 != rval) {
      printf("%s returned error 0x%X\n", buf, rval);
    }
  }
}
