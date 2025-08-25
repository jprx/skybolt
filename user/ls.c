#include "skybolt.h"

void main() {
  dirent_t d;
  int fd = open("/");
  while (0 != getdents(fd, &d)) {
    printf("%s\n", d.name);
  }
  close(fd);
}

