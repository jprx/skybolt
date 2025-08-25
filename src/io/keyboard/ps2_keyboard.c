#include <types.h>
#include <lib.h>
#include <io/ps2.h>
#include <io/display.h>

#define NUM_PS2_CODES    256

#define LSHIFT           0x12
#define RSHIFT           0x59
#define LCTRL            0x14
#define RCTRL            0x58
#define RELEASE_CODE     0xF0

#define TO_LOWERCASE     0x20
#define TO_UPPERCASE     0x20

static bool is_letting_go = false;
static bool lshift = false;
static bool rshift = false;
static bool lctrl  = false;
static bool rctrl  = false;

static inline bool is_upper_alphabet_char(char c) { return (c >= 'A' && c <= 'Z'); }
static inline bool is_lower_alphabet_char(char c) { return (c >= 'a' && c <= 'z'); }
static inline bool is_num(char c) { return (c >= '0' && c <= '9'); }

#define CHAR_TO_SHIFT(c,s) case c: return s; break;

// Scan Code Set 2
// See: https://wiki.osdev.org/PS/2_Keyboard
// See: "Application Note V-107 PS/2 PC Keyboard Scan Sets Translation Table" (http://www.vetra.com/scancodes.html)
static char ps2_to_ascii[NUM_PS2_CODES] = {
  [0x0E] = '`',
  [0x16] = '1',
  [0x1E] = '2',
  [0x26] = '3',
  [0x25] = '4',
  [0x2E] = '5',
  [0x36] = '6',
  [0x3D] = '7',
  [0x3E] = '8',
  [0x46] = '9',
  [0x45] = '0',
  [0x4E] = '-',
  [0x55] = '=',
  [0x66] = '\b',
  [0x0D] = '\t',
  [0x15] = 'q',
  [0x1D] = 'w',
  [0x24] = 'e',
  [0x2D] = 'r',
  [0x2C] = 't',
  [0x35] = 'y',
  [0x3C] = 'u',
  [0x43] = 'i',
  [0x44] = 'o',
  [0x4D] = 'p',
  [0x54] = '[',
  [0x5B] = ']',
  [0x1C] = 'a',
  [0x1B] = 's',
  [0x23] = 'd',
  [0x2B] = 'f',
  [0x34] = 'g',
  [0x33] = 'h',
  [0x3B] = 'j',
  [0x42] = 'k',
  [0x4B] = 'l',
  [0x4C] = ';',
  [0x52] = '\'',
  [0x5A] = '\n',
  [0x1A] = 'z',
  [0x22] = 'x',
  [0x21] = 'c',
  [0x2A] = 'v',
  [0x32] = 'b',
  [0x31] = 'n',
  [0x3A] = 'm',
  [0x41] = ',',
  [0x49] = '.',
  [0x4A] = '/',
  [0x29] = ' ',
  [0x5D] = '\\',
  // [0x6B] = KEY_LEFT,
  // [0x74] = KEY_RIGHT
};

char ps2_decode_keypress(u8 code) {
  char decoded;

  if (RELEASE_CODE == code) {
    is_letting_go = true;
    return '\x00';
  }

  decoded = ps2_to_ascii[code];
  if (code == LSHIFT) lshift = !is_letting_go;
  if (code == RSHIFT) rshift = !is_letting_go;
  if (code == LCTRL)  lctrl  = !is_letting_go;
  if (code == RCTRL)  rctrl  = !is_letting_go;

  if (is_letting_go) {
    is_letting_go = false;
    return '\x00';
  }

  if (lctrl || rctrl) {
    if (is_num(decoded)) {
      display_switch(decoded - '0' - 1);
      return '\x00';
    }

    if ('c' == decoded || 'l' == decoded) {
      return decoded - 'a' + 1;
    }
  }

  if (is_lower_alphabet_char(decoded)) {
    if (lshift || rshift) { decoded -= TO_UPPERCASE; }
    return decoded;
  }

  if (lshift || rshift) {
    switch(decoded) {
      CHAR_TO_SHIFT('1','!')
      CHAR_TO_SHIFT('2','@')
      CHAR_TO_SHIFT('3','#')
      CHAR_TO_SHIFT('4','$')
      CHAR_TO_SHIFT('5','%')
      CHAR_TO_SHIFT('6','^')
      CHAR_TO_SHIFT('7','&')
      CHAR_TO_SHIFT('8','*')
      CHAR_TO_SHIFT('9','(')
      CHAR_TO_SHIFT('0',')')
      CHAR_TO_SHIFT('-','_')
      CHAR_TO_SHIFT('=','+')
      CHAR_TO_SHIFT('[','{')
      CHAR_TO_SHIFT(']','}')
      CHAR_TO_SHIFT(',','<')
      CHAR_TO_SHIFT('.','>')
      CHAR_TO_SHIFT('/','?')
      CHAR_TO_SHIFT('`','~')
      CHAR_TO_SHIFT(';',':')
      CHAR_TO_SHIFT('\'','"')
      CHAR_TO_SHIFT('\\','|')
    }
  }

  return decoded;
}
