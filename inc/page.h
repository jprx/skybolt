#pragma once
#include <types.h>
#include <defs.h>

typedef enum {
  PAGE_SMALL = 0,
  PAGE_LARGE = 1,
  PAGE_GIANT = 2,
  PAGE_SUPER = 3,
  N_PAGETABLE_LEVELS,
} pagesize_t;

// A pagelevel_t (PAGE_L1, etc) really is just another way of saying
// pagesize_t. However, it is helpful to make a distinction between a page's
// size and a page table entry's level in the page hierarchy.
typedef pagesize_t pagelevel_t;
#define PAGE_L1     PAGE_SMALL
#define PAGE_L2     PAGE_LARGE
#define PAGE_L3     PAGE_GIANT
#define PAGE_L4     PAGE_SUPER

typedef enum {
  CACHE_WRITEBACK, // default
  CACHE_WRITECOMB,
  CACHE_DISABLE,
} cache_policy_t;

// human-readable defs for use with pte_set_leaf
enum {
  PTE_POINTS_TO_TABLE = false,
  PTE_POINTS_TO_MEMORY = true,
};

typedef u64 pte_t;

// page allocation functions
void init_kheap();
void *alloc_page(pagesize_t);
void free_page(void *, pagesize_t);

// allocate and zero init a small page for use as a page table
pte_t *alloc_pagetable(void);
void free_pagetable(pte_t *);
