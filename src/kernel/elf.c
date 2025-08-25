#include <file.h>
#include <cpu.h>
#include <lib.h>
#include <page.h>
#include <fs/tar.h>
#include <loader/elf.h>
#include <copyio.h>

void load_seg(ptable_t ptable, file_t *elf, proghdr_t *lc) {
  usize n_pages;
  virt_t map_va, to_va;
  phys_t to_pa;
  assert(lc->p_memsz > 0);
  assert(IS_ALIGNED_SMALL(lc->p_vaddr));

  n_pages = ROUND_UP_POW2(lc->p_memsz, SMALL_PAGE_SIZE) / SMALL_PAGE_SIZE;

  for (usize i = 0; i < n_pages; i++) {
    map_va = lc->p_vaddr + (SMALL_PAGE_SIZE * i);
    to_va = (virt_t)alloc_page(PAGE_SMALL);
    to_pa = KERN_V2P(to_va);
    map_page(ptable, map_va, to_pa, PAGE_SMALL, MODE_USER);
    memset((u8*)to_va, '\x00', SMALL_PAGE_SIZE);
    tar_read_at(elf, (u8*)to_va, SMALL_PAGE_SIZE, lc->p_offset + (SMALL_PAGE_SIZE * i));
    cpu_flush_dcache(to_va, to_va + SMALL_PAGE_SIZE);
  }
}

bool load_elf(ptable_t ptable, file_t *elf) {
  elfhdr_t header;
  proghdr_t loadcmd;
  usize bytes_read;
  bytes_read = tar_read_at(elf, (u8*)&header, sizeof(header), 0);

  if (bytes_read != sizeof(header)) return false;
  if (*(u32 *)&header.e_ident != ELF_MAGIC) return false;
  if (header.e_machine != EXPECTED_ELF_MACHINE_CODE) return false;
  if (sizeof(proghdr_t) != header.e_ph_entsize) return false;

  for (usize i = 0; i < header.e_ph_num; i++) {
    bytes_read = tar_read_at(elf, (u8*)&loadcmd, header.e_ph_entsize, header.e_phoff + (i * header.e_ph_entsize));
    assert(bytes_read == sizeof(loadcmd));

    switch(loadcmd.p_type) {
      case PT_LOAD:
        load_seg(ptable, elf, &loadcmd);
        break;

      default:
        continue;
    }
  }

  cpu_flush_icache();
  return true;
}

virt_t elf_entrypoint(file_t *elf) {
  u64 entrypoint;
  tar_read_at(elf, &entrypoint, sizeof(entrypoint), offsetof(elfhdr_t, e_entrypoint));
  return entrypoint;
}
