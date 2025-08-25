#pragma once
#include <vm.h>
#include <file.h>
#include <types.h>

#define ELF_MAGIC 0x464C457Full

typedef struct {
  u8   e_ident[16];
  u16  e_type;
  u16  e_machine;
  u32  e_version;
  u64  e_entrypoint;
  u64  e_phoff;
  u64  e_shoff;
  u32  e_flags;
  u16  e_header_size;
  u16  e_ph_entsize;
  u16  e_ph_num;
  u16  e_sh_entsize;
  u16  e_sh_num;
  u16  e_stringtable_idx;
} elfhdr_t;

typedef struct {
  u32  p_type;
  u32  p_flags;
  u64  p_offset;
  u64  p_vaddr;
  u64  p_paddr;
  u64  p_filesz;
  u64  p_memsz;
  u64  p_align;
} proghdr_t;

enum {
  PT_NULL = 0,
  PT_LOAD = 1,
};

bool   load_elf(ptable_t, file_t*);
virt_t elf_entrypoint(file_t*);
