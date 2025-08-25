#pragma once
#include <types.h>

void ps2_intr();
char ps2_decode_keypress(u8);
bool init_i8042();
