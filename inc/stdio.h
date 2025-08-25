#pragma once
#include <file.h>

#define FD_STDIN  0
#define FD_STDOUT 1
#define FD_STDERR 2

void init_stdio();

file_t *stdio_open(char *path);
usize   stdio_read(file_t *file, void *out, usize len);
usize   stdio_write(file_t *file, void *data, usize len);
file_t *stdio_dup(file_t *file);
void    stdio_flush(file_t *file);
void    stdio_close(file_t *file);

extern filesys_ops_t stdio_fs_ops;
