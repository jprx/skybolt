#pragma once
#include <types.h>

typedef struct {
  u32 flags;
  u32 mem_low;
  u32 mem_hi;
  u32 boot_dev;
  u32 cmdline;
  u32 mods_count;
  u32 mods_addr;
  u32 syms[4];
  u32 mmap_len;
  u32 mmap_addr;
  u32 drives_len;
  u32 drives_addr;
  u32 config_table;
  u32 bootloader_name;
  u32 apm_table;
  u32 vbe_control_info, vbe_mode_info;
  u16 vbe_mode, vbe_int_seg, vbe_int_off, vbe_int_len;
  u64 framebuffer_addr;
  u32 framebuffer_pitch, framebuffer_width, framebuffer_height;
  u8  framebuffer_bpp;
  u8  framebuffer_type;
} multiboot_info_t;

typedef struct {
  u32 mod_start;
  u32 mod_end;
  u32 mod_name;
  u32 reserved;
} multiboot_mod_t;
