#pragma once
#include <types.h>

#define MAX_PATHLEN 255

struct file_t;
struct tty_t;

typedef enum {
  FILE_NONE = 0,
  FILE_REGULAR,
  FILE_DIRECTORY,
  FILE_CONSOLE,
} filetype_t;

typedef struct {
  u8   filetype;
  char name[MAX_PATHLEN + 1];
} dirent_t;

typedef struct {
  struct file_t* (*open)(char *);
  usize          (*read)(struct file_t*, void *buf, usize len);
  usize          (*write)(struct file_t*, void *buf, usize len);
  usize          (*getdents)(struct file_t *, dirent_t *out);
  struct file_t* (*dup)(struct file_t *);
  void           (*close)(struct file_t*);
} filesys_ops_t;

typedef struct file_t {
  filesys_ops_t  *ops;
  u64             inode;
  struct tty_t   *tty;
  usize           pos;
  filetype_t      type;
  bool            valid;
} file_t;

void     init_files();
file_t  *file_alloc();
void     file_free(file_t *);

file_t  *file_open(char *);
usize    file_read(file_t *f, void *buf, usize len);
usize    file_write(file_t *f, void *buf, usize len);
usize    file_getdents(file_t *, dirent_t *);
file_t  *file_dup(file_t *);
void     file_close(file_t*);
