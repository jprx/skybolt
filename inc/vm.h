#pragma once
#include <types.h>
#include <page.h>
#include <cpu.h>

typedef pte_t* ptable_t;

#define PHYS_ADDR_INVALID ((0xAAAAAAAAAAAAAAAAull))

static inline bool is_user_va(uintptr_t va) { return va < BIT(48); }

void   init_vm();
void   map_page(ptable_t page_table, virt_t va, phys_t pa, pagesize_t sz, cpu_mode_t priv);
void   page_set_cache_policy(ptable_t page_table, virt_t va, cache_policy_t pol);
pte_t *walk_page(ptable_t page_table, virt_t va);
void   unmap_page(ptable_t page_table, virt_t va);
phys_t translate_page(ptable_t ptable, virt_t va);
usize  get_page_size(ptable_t ptable, virt_t va);
bool   is_page_mapped(ptable_t ptable, virt_t va);

void map_kernel(ptable_t page_table);
void unmap_kernel(ptable_t page_table);
