#pragma once
#include <types.h>
#include <io/display.h>

#define NUM_DISPLAYS   9

#define VGAFONT_WIDTH  8
#define VGAFONT_HEIGHT 16

extern const u8 vgafont16[256 * 16];

void init_display();
