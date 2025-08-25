#include "skybolt.h"

int main(int argc, char **argv) {
  for (usize i = 0; i < argc; i++)
    printf("arg %x: %s\n", i, argv[i]);

  return 0;
}
