#include <types.h>
#include <stdarg.h>
#include <io/serial.h>
#include <io/display.h>

#define CHARS_PER_U64 16

const char *digits = "0123456789ABCDEF";

void _printf_internal(char *fmt, va_list *args);

void printf(char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  _printf_internal(fmt, &args);
  va_end(args);
}

void _printf_putc(char c) {
  serial_putc(c);
  active_display_putc(c);
}

void _print_hex(u64 val) {
  for (usize i = 0; i < CHARS_PER_U64; i++) {
    _printf_putc(digits[val >> 60ull]);
    val <<= 4ull;
  }
}

void _print_str(char *s) {
  char *cursor = s;
  while (*cursor) {
    _printf_putc(*cursor);
    cursor++;
  }
}

void _printf_internal(char *fmt, va_list *args) {
  for (char *i = fmt; *i != 0; i++) {
    if (*i != '%') {
      _printf_putc(*i);
      continue;
    }

    i++;

    switch (*i) {
      case 'x':
      case 'X':
        _print_hex(va_arg(*args, u64));
        break;

      case 's':
        _print_str(va_arg(*args, char*));
        break;

      default:
        _printf_putc('%');
        _printf_putc(*i);
        break;
    }
  }
}
