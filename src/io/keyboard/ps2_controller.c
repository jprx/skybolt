#include <cpu.h>
#include <lib.h>
#include <io/ps2.h>
#include <io/intctl.h>
#include <platform.h>
#include <tty.h>
#include <io/display.h>

#define PS2_IO_DATA                        0x60
#define PS2_IO_CMD                         0x64
#define PS2_IO_STATUS                      0x64

#define PS2_KEYBOARD_LEGACY_IRQNUM         1

#define STATUS_OUTPUT_FULL                 BIT(0)
#define STATUS_INPUT_FULL                  BIT(1)
#define STATUS_DATA_FOR_CONTROLLER         BIT(3)

#define CONFIG_DISABLE_INTERRUPTS          0
#define CONFIG_PORT1_INTERRUPTS            BIT(0)
#define CONFIG_DISABLE_PORT2_CLOCK         BIT(5)

#define SELFTEST_PASS                      0x55
#define SELFTEST_FAIL                      0xFC

#define PORTTEST_PASS                      0

#define PS2_TIMEOUT                        100000

#define CMD_READ_CONFIG                    0x20
#define CMD_WRITE_CONFIG                   0x60
#define CMD_SELFTEST                       0xAA
#define CMD_TEST_PORT1                     0xAB
#define CMD_DISABLE_PORT1                  0xAD
#define CMD_ENABLE_PORT1                   0xAE
#define CMD_DISABLE_PORT2                  0xA7

#define DEV_CMD_RESET                      0xFF

void ps2_intr() {
  u8 key = ps2_decode_keypress(inb(PS2_IO_DATA));
  if ('\x00' != key) {
    display_handle_input(key);
  }
}

u8 i8042_read(u8 cmd) {
  usize retries = 0;
  outb(PS2_IO_CMD, cmd);
  while ((inb(PS2_IO_STATUS) & STATUS_OUTPUT_FULL) == 0) {
    if (retries++ > PS2_TIMEOUT) {
      panic("i8042_read: failed");
    }
  }
  return inb(PS2_IO_DATA);
}

void i8042_write(u8 cmd, u8 val) {
  usize retries = 0;
  outb(PS2_IO_CMD, cmd);
  while((inb(PS2_IO_STATUS) & STATUS_INPUT_FULL) != 0) {
    if (retries++ > PS2_TIMEOUT) {
      panic("i8042_write: failed");
    }
  }

  outb(PS2_IO_DATA, val);
}

void i8042_cmd(u8 cmd) {
  outb(PS2_IO_CMD, cmd);
}

void i8042_flush() {
  inb(PS2_IO_DATA);
}

void ps2_device_cmd(u8 cmd) {
  usize retries = 0;
  while((inb(PS2_IO_STATUS) & STATUS_INPUT_FULL) != 0) {
    if (retries++ > PS2_TIMEOUT) {
      panic("ps2_device_cmd: failed");
    }
  }
  outb(PS2_IO_DATA, cmd);
}

void ps2_reset_device1() {
  usize retries = 0;
  ps2_device_cmd(DEV_CMD_RESET);

  // We don't parse the reset response data; we just receive until the device
  // stops sending info.
  while (retries++ < PS2_TIMEOUT) {
    if (0 != (inb(PS2_IO_STATUS) & STATUS_OUTPUT_FULL)) {
      inb(PS2_IO_DATA);
      retries = 0;
    }
  }
}

bool init_i8042() {
  // Only initializes first PS/2 port, assumed to be the keyboard.
  // In the future mouse support could be added by utilizing the second port.
  volatile u8 tmp;
  i8042_cmd(CMD_DISABLE_PORT1);
  i8042_cmd(CMD_DISABLE_PORT2);
  i8042_flush();
  i8042_write(CMD_WRITE_CONFIG, CONFIG_DISABLE_INTERRUPTS | CONFIG_DISABLE_PORT2_CLOCK);
  tmp = i8042_read(CMD_SELFTEST);
  if (SELFTEST_PASS != tmp)
    return false;

  tmp = i8042_read(CMD_TEST_PORT1);
  if (PORTTEST_PASS != tmp)
    return false;

  i8042_cmd(CMD_ENABLE_PORT1);
  ps2_reset_device1();
  g_intc.ops->map(&g_intc, PS2_KEYBOARD_LEGACY_IRQNUM, PS2_KEYBOARD_IRQ);
  i8042_write(CMD_WRITE_CONFIG, CONFIG_PORT1_INTERRUPTS | CONFIG_DISABLE_PORT2_CLOCK);
  return true;
}
