#pragma once
#include <types.h>
#include <defs.h>
#include <io/serial.h>

#define DISPLAY_DEVICE_PREFIX "/dev/display"

struct tty_t;

typedef struct {
  u32            *framebuffer;
  usize           pxwidth;
  usize           pxheight;
} framebuffer_t;

typedef struct {
  framebuffer_t  *frame;
  struct tty_t   *tty;
  char           *textbuf;
  usize           textwidth;
  usize           textheight;
  usize           cursor;
  bool            is_dirty;
  bool            needs_full_repaint;
  usize           last_clean_line;
} display_t;

// A "virtual UART" bridges the tty and graphics subsystems.
// TTYs send data to a vuart, which forwards it to a display.
void vuart_init(uart_t *port);
void vuart_send(uart_t *port, u8 c);
u8   vuart_recv(uart_t *port);
bool vuart_has_data(uart_t *port);
extern uart_ops_t vuart_ops;

void init_display();
void display_putc(display_t *, u8);
void display_redraw(display_t *);
void active_display_putc(u8);
void display_begin_panic();
void display_finish_panic();
void display_clear(display_t *);
void display_reset_tty(struct tty_t *);

void KTHREAD thread_display_update();
bool is_display_active();
void display_switch(usize idx);
void display_handle_input(char c);
