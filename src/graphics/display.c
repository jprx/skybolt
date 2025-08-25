#include <graphics.h>
#include <io/display.h>
#include <platform.h>
#include <lib.h>
#include <defs.h>
#include <vm.h>
#include <tty.h>
#include <task.h>

#define COLOR_WHITE 0xFFFFFF
#define COLOR_BLACK 0x000000

uart_t    g_vuart[NUM_DISPLAYS];
tty_t     g_display_tty[NUM_DISPLAYS];
display_t g_display[NUM_DISPLAYS];

display_t *active_display = NULL;
static usize active_display_idx = 0;
static bool is_gui_enabled = false;

void draw_char(display_t *d, usize x, usize y, char c, u32 fg_col, u32 bg_col) {
  usize screen_idx;
  u8 pxdata;
  bool should_color;
  if (x >= (d->textwidth)) return;
  if (y >= (d->textheight)) return;

  for (usize j = 0; j < VGAFONT_HEIGHT; j++) {
    pxdata = vgafont16[(VGAFONT_HEIGHT*c)+j];
    for (usize i = 0; i < VGAFONT_WIDTH; i++) {
      should_color = (pxdata >> i) & BIT(0);
      screen_idx = (((VGAFONT_HEIGHT*y)+j)*d->frame->pxwidth)+((VGAFONT_WIDTH*x)+i);
      d->frame->framebuffer[screen_idx] = should_color ? fg_col : bg_col;
    }
  }
}

void draw_str(display_t *d, usize x, usize y, char *s, u32 fg_col, u32 bg_col) {
  char *cursor = s;

  while (*cursor) {
    draw_char(d, x+(cursor-s), y, *cursor, fg_col, bg_col);
    cursor++;
  }
}

void draw_block(display_t *d, usize x, usize y, u32 color) {
  usize screen_idx;
  bool should_color;
  if (x >= (d->textwidth)) return;
  if (y >= (d->textheight)) return;

  for (usize j = 0; j < VGAFONT_HEIGHT; j++) {
    for (usize i = 0; i < VGAFONT_WIDTH; i++) {
      should_color = true;
      screen_idx = (((VGAFONT_HEIGHT*y)+j)*d->frame->pxwidth)+((VGAFONT_WIDTH*x)+i);
      d->frame->framebuffer[screen_idx] = should_color ? color : 0;
    }
  }
}

void draw_statusline(display_t *d) {
  for (usize i = 0; i < d->textwidth; i++)
    draw_block(d, i, d->textheight - 1, COLOR_WHITE);
  draw_str(d, 1, d->textheight - 1, "Skybolt", COLOR_BLACK, COLOR_WHITE);
  draw_char(d, d->textwidth - 2, d->textheight - 1, active_display_idx + '0' + 1, COLOR_BLACK, COLOR_WHITE);
}

void draw_line(display_t *d, usize line_idx, u32 fg_col, u32 bg_col) {
  bool should_color;
  u8 pxdata;
  usize screen_idx;
  usize txwidth;
  usize pxwidth;

  txwidth  = d->textwidth;
  pxwidth  = d->frame->pxwidth;

  for (usize j = 0; j < VGAFONT_HEIGHT; j++) {
    for (usize char_idx = 0; char_idx < txwidth; char_idx++) {
      pxdata = vgafont16[(VGAFONT_HEIGHT * d->textbuf[(line_idx * txwidth) + char_idx]) + j];
      for (usize i = 0; i < VGAFONT_WIDTH; i++) {
        should_color = (pxdata >> i) & BIT(0);
        screen_idx = (((VGAFONT_HEIGHT*line_idx)+j)*pxwidth)+((VGAFONT_WIDTH*char_idx)+i);
        d->frame->framebuffer[screen_idx] = should_color ? fg_col : bg_col;
      }
    }
  }
}

void display_redraw(display_t *d) {
  if (NULL == d)
    return;
  if (d->needs_full_repaint) {
    memset(d->frame->framebuffer, '\x00', d->frame->pxwidth * d->frame->pxheight * sizeof(u32));
    d->needs_full_repaint = false;
  }
  if (!d->is_dirty) return;

  for (usize cur_line = d->last_clean_line; cur_line < 1 + (d->cursor / d->textwidth); cur_line++) {
    draw_line(d, cur_line, COLOR_WHITE, COLOR_BLACK);
  }
  draw_block(d, d->cursor % d->textwidth, d->cursor / d->textwidth, COLOR_WHITE);
  draw_statusline(d);
  d->is_dirty = false;
}

void display_begin_panic() {
  display_t *d = active_display;
  if (NULL == d)
    return;

  display_clear(d);
}

void display_finish_panic() {
  if (NULL == active_display)
    return;
  active_display->is_dirty = true;
  active_display->last_clean_line = 0;
  active_display->needs_full_repaint = true;
  display_redraw(active_display);
}

void display_scroll_up(display_t *d) {
  if (NULL == d)
    return;
  memcpy(&d->textbuf[0], &d->textbuf[d->textwidth], (d->textwidth * (d->textheight-1)));
  d->cursor -= d->textwidth;
  d->last_clean_line = 0;
}

void display_putc(display_t *d, u8 c) {
  usize cur_line;
  if (NULL == d)
    return;

  cur_line = d->cursor / d->textwidth;
  if ('\r' == c) return;
  if ('\x00' == c) return;

  if (!d->is_dirty)
    d->last_clean_line = cur_line;

  d->is_dirty = true;

  if ('\b' == c) {
    if (d->cursor > 0) d->cursor--;
    goto out;
  }

  if ('\n' == c) {
    d->cursor = d->textwidth * (1 + cur_line);
    goto out;
  }

  while (d->cursor / d->textwidth >= d->textheight - 1)
    display_scroll_up(d);

  d->textbuf[d->cursor++] = c;

out:
  if (d == active_display)
    wakeup(&active_display);
}

void active_display_putc(u8 c) {
  if (NULL == active_display)
    return;
  display_putc(active_display, c);
}

void display_switch(usize idx) {
  if (!is_display_active())
    return;
  if (idx >= NUM_DISPLAYS) return;
  if (active_display_idx == idx) return;
  active_display_idx = idx;
  active_display = &g_display[idx];
  active_display->is_dirty = true;
  active_display->needs_full_repaint = true;
  active_display->last_clean_line = 0;
  wakeup(&active_display);
}

bool is_display_active() {
  return is_gui_enabled;
}

void display_clear(display_t *d) {
  if (NULL == d)
    return;
  memset(d->textbuf, '\x00', d->textwidth * d->textheight);
  d->needs_full_repaint = true;
  d->is_dirty = true;
  d->cursor = 0;
  wakeup(&active_display);
}

void display_handle_input(char c) {
  if (NULL == active_display)
    return;

  if (ASCII_CLEAR == c) {
    display_clear(active_display);
    return;
  }
  tty_new_data(active_display->tty, c);
}

void display_reset_tty(tty_t *t) {
  for (usize i = 0; i < NUM_DISPLAYS; i++) {
    if (t == &g_display_tty[i]) {
      display_clear(&g_display[i]);
      if (active_display == &g_display[i])
        wakeup(&active_display);
    }
  }
}

void display_setup(display_t *d, uart_t *vuart, tty_t *tty, framebuffer_t *f) {
  d->frame = f;
  d->textwidth  = f->pxwidth  / VGAFONT_WIDTH;
  d->textheight = f->pxheight / VGAFONT_HEIGHT;
  assert(d->textwidth * d->textheight < LARGE_PAGE_SIZE);

  d->textbuf = alloc_page(PAGE_LARGE);
  memset(d->textbuf, '\x00', (d->textwidth * d->textheight));
  d->cursor = 0;
  d->is_dirty = false;
  d->last_clean_line = 0;
  d->needs_full_repaint = false;
  d->tty = tty;

  vuart->ops = &vuart_ops;
  vuart->core_irq = 0;
  vuart->ext_irq = 0;
  vuart->baseaddr = (uintptr_t)d;
  vuart->ops->init(vuart);

  tty_attach(tty, vuart);
}

void KTHREAD thread_display_update() {
  // No interrupts, as keycodes that switch screens during a repaint can cause
  // issues. Repaints are really fast anyways so ints won't be off for long.
  cpu_disable_ints();
  while (true) {
    assert(!cpu_ints_enabled());
    display_redraw(active_display);
    sleep(&active_display);
  }
}

void init_display() {
  is_gui_enabled = false;
  if (0 == g_framebuffer.framebuffer) return;
  if (0 == g_framebuffer.pxwidth || 0 == g_framebuffer.pxheight) return;

  for (usize i = 0; i < NUM_DISPLAYS; i++)
    display_setup(
      &g_display[i],
      &g_vuart[i],
      &g_display_tty[i],
      &g_framebuffer
    );

  active_display_idx = 0;
  active_display = &g_display[active_display_idx];
  is_gui_enabled = true;
}
