#include <page.h>
#include <file.h>
#include <stdio.h>
#include <lib.h>
#include <fs/tar.h>

#define NUM_FILES  1024

file_t filetab[NUM_FILES];

void init_files() {
  for (usize i = 0; i < NUM_FILES; i++) {
    filetab[i].valid = false;
  }
}

file_t *file_alloc() {
  for (usize i = 0; i < NUM_FILES; i++) {
    if (!filetab[i].valid) {
      filetab[i].ops = NULL;
      filetab[i].inode = 0;
      filetab[i].pos = 0;
      filetab[i].tty = NULL;
      filetab[i].type = FILE_NONE;
      filetab[i].valid = true;
      return &filetab[i];
    }
  }
  panic("file_alloc: out of available files");
}

void file_free(file_t *f) {
  f->valid = false;
  f->inode = 0;
  f->ops = NULL;
}

file_t *file_open(char *path) {
  if (strequal(path, "/dev/panic1"))
    *(u32 *)0xDEADC0DE = 0x1234;

  if (strequal(path, "/dev/panic2"))
    assert(false);

  if (strprefix(path, "/dev/"))
    return stdio_open(path);

  return tar_fs_ops.open(path);
}

usize   file_read(file_t *f, void *b, usize n) { return f->ops->read(f, b, n); }
usize   file_write(file_t *f, void *b, usize n) { return f->ops->write(f, b, n); }
usize   file_getdents(file_t *f, dirent_t *d) { return f->ops->getdents(f, d); }
file_t *file_dup(file_t *f) { return f->ops->dup(f); }
void    file_close(file_t *f) { return f->ops->close(f); }
