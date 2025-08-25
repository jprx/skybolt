#include "skybolt.h"
void main() {
  struct utsname sysinf;

  if (0 != uname(&sysinf))
    exit(1);

  printf("%s %s %s %s\n", sysinf.sysname, sysinf.version, sysinf.release, sysinf.machine);
}

