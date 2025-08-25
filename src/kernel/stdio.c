#include <stdio.h>
#include <io/serial.h>
#include <tty.h>
#include <lib.h>
#include <graphics.h>

static file_t *stdio_serial = NULL;
static file_t *stdio_display[NUM_DISPLAYS];

void init_stdio() {
  stdio_serial = file_alloc();
  assert(NULL != stdio_serial);
  stdio_serial->ops = &stdio_fs_ops;
  stdio_serial->type = FILE_CONSOLE;
  stdio_serial->tty = &g_serial_tty;

  if (is_display_active()) {
    for (usize i = 0; i < NUM_DISPLAYS; i++) {
      stdio_display[i] = file_alloc();
      assert(NULL != stdio_display[i]);
      stdio_display[i]->ops = &stdio_fs_ops;
      stdio_display[i]->type = FILE_CONSOLE;
      stdio_display[i]->tty = &g_display_tty[i];
    }
  } else {
    for (usize i = 0; i < NUM_DISPLAYS; i++) {
      stdio_display[i] = NULL;
    }
  }
}

// Find display file for "/dev/displayX" or "/dev/display"
file_t *find_display_file(char *path) {
  usize display_idx = 0;

  if (!is_display_active())
    return NULL;

  assert(strprefix(path, DISPLAY_DEVICE_PREFIX));
  char display_num = path[strlen(DISPLAY_DEVICE_PREFIX)];

  // A null byte here means the user passed in "/dev/display"- default to display 0
  if ('\x00' == display_num)
    display_idx = 0;
  else
    display_idx = display_num - '0';

  assert(display_idx < NUM_DISPLAYS);
  return stdio_display[display_idx];
}

file_t *stdio_open(char *path) {
  if (strequal(path, "/dev/null"))
    return NULL;

  if (strequal(path, "/dev/serial"))
    return stdio_serial;

  if (strprefix(path, "/dev/display"))
    return find_display_file(path);

  panic("stdio_open: unknown stdio device");
}

usize stdio_read(file_t *file, void *out, usize len) {
  return tty_readin(file->tty, (u8*)out, len);
}

usize stdio_write(file_t *file, void *data, usize len) {
  return tty_writeout(file->tty, (u8*)data, len);
}

usize stdio_getdents(file_t *file, dirent_t *d) {
  return 0; // Unsupported
}

bool file_is_stdio(file_t *file) {
  if (file == stdio_serial) return true;

  for (usize i = 0; i < NUM_DISPLAYS; i++)
    if (file == stdio_display[i]) return true;

  return false;
}

file_t *stdio_dup(file_t *file) {
  assert(file_is_stdio(file));
  return file;
}

void stdio_close(file_t *file) {
  // All tasks share stdio files so we should never free them
  assert(file_is_stdio(file));
  return;
}

filesys_ops_t stdio_fs_ops = {
  .open     = stdio_open,
  .read     = stdio_read,
  .write    = stdio_write,
  .getdents = stdio_getdents,
  .dup      = stdio_dup,
  .close    = stdio_close,
};
