#pragma once

#define UTSNAMELEN 64

struct utsname {
  char sysname[UTSNAMELEN];
  char nodename[UTSNAMELEN];
  char release[UTSNAMELEN];
  char version[UTSNAMELEN];
  char machine[UTSNAMELEN];
};

void uname(struct utsname *);
