#pragma once
#include <file.h>

void     tar_ls_files();
file_t  *tar_open(char *path);
usize    tar_read(file_t *file, void *data, usize len);
usize    tar_write(file_t *file, void *data, usize len);
usize    tar_getdents(file_t *file, dirent_t *dent);
file_t  *tar_dup(file_t *file);
void     tar_close(file_t *file);

// Random access to any part of a file without dealing with seek
usize    tar_read_at(file_t *file, void *data, usize len, usize offset);

extern filesys_ops_t tar_fs_ops;
