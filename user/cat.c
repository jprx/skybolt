#include "skybolt.h"

char buf[1024];

int main(int argc, char **argv) {
  usize bytes_read = 0;
  if (argc < 1)
    return 1;

  if (argc != 2) {
    printf("usage: %s [file]\n", argv[0]);
    exit(1);
  }

  int fd = open(argv[1]);
  if (fd < 0) {
    printf("couldn't find %s\n", argv[1]);
    exit(1);
  }

  memset(buf, '\x00', sizeof(buf));
  while (0 != (bytes_read = read(fd, buf, sizeof(buf)))) {
    write(stdout, buf, bytes_read);
    memset(buf, '\x00', sizeof(buf));
  }
  close(fd);
  return 0;
}
