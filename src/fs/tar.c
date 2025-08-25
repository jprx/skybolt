#include <fs/tar.h>
#include <platform.h>
#include <lib.h>
#include <file.h>

#define TAR_BLOCKSIZE          512

// Size is 12 chars, but the last is a space (' '), so we should only read 11 of them
#define TAR_SIZE_FIELD_LEN     11

typedef struct __attribute__((packed)) {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag;
  char linkname[100];
} tar_header_t;

// Given a string in octal, convert it to a u64
u64 stoul_octal(char *str, usize str_len) {
  u64 accumulator = 0;
  for (usize i = 0; i < str_len; i++) {
    assert(str[i] >= '0' && str[i] < '9');
    accumulator = (accumulator * 8) + (str[i] - '0');
  }
  return accumulator;
}

usize tar_filesize(tar_header_t *h) {
  return stoul_octal(h->size, TAR_SIZE_FIELD_LEN);
}

usize tar_num_blocks_for_file(tar_header_t *h) {
  // Add 1 to account for the header block, which every file has
  return 1 + (ROUND_UP_POW2(tar_filesize(h), TAR_BLOCKSIZE) / TAR_BLOCKSIZE);
}

tar_header_t *tar_to_next(tar_header_t *h) {
  return (tar_header_t*)(((uintptr_t)h) + (TAR_BLOCKSIZE * tar_num_blocks_for_file(h)));
}

bool tar_is_valid(tar_header_t *h) {
  return h->name[0] != '\x00';
}

file_t *tar_open(char *path) {
  file_t *new_file = NULL;
  tar_header_t *cursor = (tar_header_t *)g_initrd;

  while(tar_is_valid(cursor)) {
    if (strequal(path, cursor->name)) {
      new_file = file_alloc();
      assert(NULL != new_file);
      new_file->ops = &tar_fs_ops;
      new_file->inode = (u64)cursor;
      new_file->type = FILE_REGULAR;
      return new_file;
    }
    cursor = tar_to_next(cursor);
  }

  if (strequal("/", path)) {
    new_file = file_alloc();
    new_file->ops = &tar_fs_ops;
    new_file->inode = 0;
    new_file->type = FILE_DIRECTORY;
    new_file->pos = 0;
    return new_file;
  }

  return NULL;
}

usize _tar_read(tar_header_t *header, u8 *databuf, usize len, usize offset) {
  assert(header != NULL);
  assert((uintptr_t)header >= (uintptr_t)g_initrd);
  assert((uintptr_t)header < ((uintptr_t)g_initrd + MAX_INITRD_SIZE));

  usize file_size = tar_filesize(header);
  u8 *file_data = (u8*)((uintptr_t)header + TAR_BLOCKSIZE);

  if (0 == file_size) return 0;
  if (offset > file_size) return 0;

  usize bytes_to_read = min(len, (file_size - offset));

  if (0 != bytes_to_read) {
    memcpy(databuf, file_data + offset, bytes_to_read);
  }

  return bytes_to_read;
}

usize tar_read(file_t *file, void *buf, usize len) {
  if (file->type != FILE_REGULAR) return 0;
  usize n = _tar_read((tar_header_t*)file->inode, buf, len, file->pos);
  file->pos += n;
  return n;
}

usize tar_read_at(file_t *file, void *databuf, usize len, usize offset) {
  return _tar_read((tar_header_t*)file->inode, databuf, len, offset);
}

void tar_ls_files() {
  tar_header_t *cursor = (tar_header_t *)g_initrd;

  while (tar_is_valid(cursor)) {
    printf("%s\n", cursor->name);
    cursor = tar_to_next(cursor);
  }
}

usize tar_getdents(file_t *file, dirent_t *dent) {
  usize idx = 0;
  if (file->type != FILE_DIRECTORY) return 0;
  tar_header_t *cursor = (tar_header_t *)g_initrd;

  while (tar_is_valid(cursor)) {
    if (idx == file->pos) {
      strncpy((char*)&dent->name, cursor->name, MAX_PATHLEN);
      dent->filetype = FILE_REGULAR;
      file->pos++;
      return sizeof(*dent);
    }
    cursor = tar_to_next(cursor);
    idx++;
  }

  return 0;
}

usize tar_write(file_t *file, void *data, usize len) {
  panic("tar_write: this filesystem does not support modification");
}

file_t *tar_dup(file_t *file) {
  panic("tar_dup: not implemented");
}

void tar_close(file_t *file) {
  file_free(file);
}

filesys_ops_t tar_fs_ops = {
  .open     = tar_open,
  .read     = tar_read,
  .write    = tar_write,
  .getdents = tar_getdents,
  .dup      = tar_dup,
  .close    = tar_close,
};
