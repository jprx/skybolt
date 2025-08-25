#pragma once
#include <graphics.h>
#include <types.h>
#include <defs.h>

#define TTY_BUFIO_LEN    ((SMALL_PAGE_SIZE))

#define ASCII_DELETE      0x7F

// Ctrl + C (^C):
#define ASCII_EOT         0x03

// Ctrl + L:
#define ASCII_CLEAR       0x0C

struct thread_t;
struct uart_t;

/*
 * Overview of the IO architecture.
 * In general, it looks like file -> tty -> uart -> device.
 *
 *                   ┌──────────────────┐
 *                   │  Stdio file_t[]  │
 *                   └────────┬─────────┘
 *              ┌─────────────┴──────────────┐
 *              │                            │
 *     ┌────────┴────────┐       ┌───────────┴──────┐
 *     │   Serial TTY    │       │ ┌────────────────┴─┐
 *     └────────┬────────┘       └─┤ ┌────────────────┴─┐
 *              │                  └─┤   Display TTYs   │
 *              │                    └─────┬─┬─┬────────┘
 *              │                          │ │ │
 *     ┌────────┴────────┐       ┌─────────┴─┴─┴────┐
 *     │   Serial UART   │       │ ┌────────────────┴─┐
 *     └────────┬────────┘       └─┤ ┌────────────────┴─┐
 *              │                  └─┤  Virtual UARTs   │
 *              │                    └─────┬─┬─┬────────┘
 *              │                          │ │ │
 *     ┌────────┴────────┐       ┌─────────┴─┴─┴────────┐
 *     │  Physical UART  │       │ ┌────────────────────┴─┐
 *     │     Adapter     │       │ │ ┌────────────────────┴─┐
 *     └────────┬────────┘       │ │ │                      │
 *            ┌─┴─┐              │ │ │   Virtual Desktops   │
 *            │   │              └─┤ │                      │
 *            │USB│                └─┤                      │
 *            │   │                  └─────────┬────────────┘
 *            └─┬─┘                            │
 * ┌────────────┴────────────┐    ┌────────────┴────────────┐
 * │                         │    │                         │
 * │                         │    │                         │
 * │                         │    │                         │
 * │   Your other computer   │    │       Framebuffer       │
 * │                         │    │                         │
 * │                         │    │                         │
 * │                         │    │                         │
 * │                         │    │                         │
 * └─────────────────────────┘    └─────────────────────────┘
 */

typedef struct tty_t {
  u8              *buf;
  struct uart_t   *port;
  usize            cursor;
  bool             data_ready;
} tty_t;

void  init_tty();
void  tty_attach(tty_t*, struct uart_t *);
usize tty_writeout(tty_t*, u8 *data, usize len);
usize tty_readin(tty_t*, u8 *data, usize len);
void  tty_new_data(tty_t*, u8);

extern tty_t g_serial_tty;
extern tty_t g_display_tty[NUM_DISPLAYS];
