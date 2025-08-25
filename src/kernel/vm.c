#include <types.h>
#include <page.h>
#include <vm.h>
#include <defs.h>
#include <lib.h>
#include <cpu.h>

/*
 * A 48 bit virtual address looks like this:
 * |63    48|47     39|38     30|29     21|20     12|11     0|
 * +---------------------------------------------------------+
 * |  Sign  | Level 4 | Level 3 | Level 2 | Level 1 | Offset |
 * |        | (SUPER) | (GIANT) | (LARGE) | (SMALL) |        |
 * +---------------------------------------------------------+
 *
 * L1 indexes small pages (4KB)
 * L2 indexes large pages (2MB)
 * L3 indexes giant pages (1GB)
 * L4 indexes super pages (512GB)
 */

static inline usize get_pt_index(virt_t va, pagelevel_t level) {
  return (va >> (PAGE_OFFSET_BITS + (PAGE_BITS_PER_LEVEL * level))) & MASK(PAGE_BITS_PER_LEVEL);
}

static inline bool is_aligned(virt_t va, pagesize_t kind) {
  switch(kind) {
    case PAGE_SMALL: return IS_ALIGNED_SMALL(va); break;
    case PAGE_LARGE: return IS_ALIGNED_LARGE(va); break;
    case PAGE_GIANT: return IS_ALIGNED_GIANT(va); break;
    case PAGE_SUPER: return IS_ALIGNED_SUPER(va); break;
    default: panic("is_aligned: unsupported page kind");
  }
}

static inline usize page_size_in_bytes(pagesize_t sz) {
  switch(sz) {
    case PAGE_SMALL: return SMALL_PAGE_SIZE; break;
    case PAGE_LARGE: return LARGE_PAGE_SIZE; break;
    case PAGE_GIANT: return GIANT_PAGE_SIZE; break;
    case PAGE_SUPER: return SUPER_PAGE_SIZE; break;
    default: return 0; break;
  }
}

// Page sizes map to levels 1:1- they're different ways of saying the same thing
static inline pagelevel_t page_size_to_level(pagesize_t sz)   { return (pagelevel_t)sz; }
static inline pagesize_t  page_level_to_size(pagelevel_t lvl) { return (pagesize_t)lvl; }

ptable_t alloc_pagetable() {
  // assumption: a PTE of all zeros is considered invalid
  pte_t *new_ptable = alloc_page(PAGE_SMALL);
  memset(new_ptable, '\x00', SMALL_PAGE_SIZE);
  return new_ptable;
}

void map_page(ptable_t page_table, virt_t va, phys_t pa, pagesize_t sz, cpu_mode_t priv) {
  if (!is_aligned(va, sz)) panic("map_page: misaligned VA");
  if (!is_aligned(pa, sz)) panic("map_page: misaligned PA");

  // fill in any missing pages
  pagelevel_t last_level = page_size_to_level(sz);
  for (usize level = PAGE_L4; level > last_level; level--) {
    pte_t *entry = &page_table[get_pt_index(va, level)];

    if (!pte_is_valid(entry)) {
      u64 *new_ptable = alloc_pagetable();
      pte_set_valid(entry, true);
      pte_set_phys(entry, KERN_V2P((u64)new_ptable));
      pte_set_leaf(entry, level, PTE_POINTS_TO_TABLE);
    }

    page_table = (u64*)KERN_P2V(pte_get_phys(entry));
  }

  // fill in the final leaf page
  // note that this can leak memory if this VA previously pointed to
  // a smaller sized PA (and thus we are about to overwrite an intermediate
  // page table that was previously allocated). So, uh, don't do that
  pte_t *end_entry = &page_table[get_pt_index(va, last_level)];
  if (NULL == end_entry) panic("map_page: failed to allocate a page table entry");
  pte_set_valid(end_entry, true);
  pte_set_phys(end_entry, pa);
  pte_set_priv(end_entry, priv);
  pte_set_leaf(end_entry, page_size_to_level(sz), PTE_POINTS_TO_MEMORY);
}

pte_t *walk_page(ptable_t page_table, virt_t va) {
  for (usize level = PAGE_L4; level >= PAGE_L1; level--) {
    pte_t *entry = &page_table[get_pt_index(va, level)];
    if (!pte_is_valid(entry)) return NULL;
    if (pte_is_leaf(entry, level)) return entry;
    page_table = (u64*)KERN_P2V(pte_get_phys(entry));
  }

  panic("walk_page: no invalid PTEs found, but also no leaves found- this should be impossible");
}

void page_set_cache_policy(ptable_t ptable, virt_t va, cache_policy_t pol) {
  pte_t *entry = walk_page(ptable, va);
  if (!entry) return;
  pte_set_cache_policy(entry, pol);
}

phys_t translate_page(ptable_t ptable, virt_t va) {
  pte_t *entry = walk_page(ptable, va);
  if (!entry) return PHYS_ADDR_INVALID;
  return pte_get_phys(entry) | (va & MASK(PAGE_OFFSET_BITS));
}

usize get_page_size(ptable_t ptable, virt_t va) {
  for (usize level = PAGE_L4; level >= PAGE_L1; level--) {
    pte_t *entry = &ptable[get_pt_index(va, level)];
    if (!pte_is_valid(entry)) return 0;
    if (pte_is_leaf(entry, level))
      return page_size_in_bytes(page_level_to_size(level));
    ptable = (u64*)KERN_P2V(pte_get_phys(entry));
  }

  panic("get_page_size: no invalid PTEs found, but also no leaves found- this should be impossible");
}

bool is_page_mapped(ptable_t ptable, virt_t va) {
  return (PHYS_ADDR_INVALID != translate_page(ptable, va));
}

void unmap_page(ptable_t page_table, virt_t va) {
  pte_t *entry = walk_page(page_table, va);
  if (!entry) return;
  pte_set_valid(entry, false);
}

void _free_pagetable_recursive(pte_t *page_table, pagelevel_t level) {
  if (PAGE_L1 == level) {
    // Everything in here must be a leaf page- free anything we find
    for (usize i = 0; i < NUM_PT_ENTRIES; i++) {
      pte_t *entry = &page_table[i];
      if (pte_is_valid(entry)) {
        virt_t backing_pg = (virt_t)KERN_P2V(pte_get_phys(entry));
        free_page((void*)backing_pg, PAGE_SMALL);
      }
    }
    free_page(page_table, PAGE_SMALL);
    return;
  }

  for (usize i = 0; i < NUM_PT_ENTRIES; i++) {
    pte_t *entry = &page_table[i];
    if (pte_is_valid(entry) && !pte_is_leaf(entry, level)) {
      pte_t *next_table = (pte_t*)KERN_P2V(pte_get_phys(entry));
      _free_pagetable_recursive(next_table, level - 1);
    }
  }

  free_page(page_table, PAGE_SMALL);
}

void free_pagetable(ptable_t page_table) {
  _free_pagetable_recursive(page_table, N_PAGETABLE_LEVELS - 1);
}

void map_kernel(ptable_t page_table) {
  for (usize i = 0; i < NUM_PT_ENTRIES; i++) {
    // main cacheable memory
    map_page(
      page_table,
      KERNEL_LINK_ADDRESS + (i * GIANT_PAGE_SIZE),
      i * GIANT_PAGE_SIZE,
      PAGE_GIANT,
      MODE_KERN
    );

    page_set_cache_policy(
      page_table,
      KERNEL_LINK_ADDRESS + (i * GIANT_PAGE_SIZE),
      CACHE_WRITEBACK
    );

    // device memory
    map_page(
      page_table,
      KERNEL_DEVICE_MAP + (i * GIANT_PAGE_SIZE),
      i * GIANT_PAGE_SIZE,
      PAGE_GIANT,
      MODE_KERN
    );

    page_set_cache_policy(
      page_table,
      KERNEL_DEVICE_MAP + (i * GIANT_PAGE_SIZE),
      CACHE_DISABLE
    );

    // write combining memory (framebuffer)
    map_page(
      page_table,
      WRITE_COMB_MAP + (i * GIANT_PAGE_SIZE),
      i * GIANT_PAGE_SIZE,
      PAGE_GIANT,
      MODE_KERN
    );

    page_set_cache_policy(
      page_table,
      WRITE_COMB_MAP + (i * GIANT_PAGE_SIZE),
      CACHE_WRITECOMB
    );
  }
}

void unmap_kernel(pte_t *page_table) {
  for (usize i = 0; i < NUM_PT_ENTRIES; i++) {
    unmap_page(
      page_table,
      KERNEL_LINK_ADDRESS + (i * GIANT_PAGE_SIZE)
    );

    unmap_page(
      page_table,
      KERNEL_DEVICE_MAP + (i * GIANT_PAGE_SIZE)
    );

    unmap_page(
      page_table,
      WRITE_COMB_MAP + (i * GIANT_PAGE_SIZE)
    );
  }
}

void init_vm() {
  // This memory will be lost when we switch to the first task.
  // @TODO: reclaim this page table after launching the initial task.
  ptable_t init_ptable = alloc_pagetable();
  map_kernel(init_ptable);
  cpu_set_pagetable(init_ptable);
  cpu_flush_tlb();
}
