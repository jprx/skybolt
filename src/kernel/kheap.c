#include <defs.h>
#include <lib.h>
#include <types.h>
#include <page.h>
#include <platform.h>

// Space inefficient O(n) heap
// Good enough for our purposes for now though

// #define KHEAP_LOGGING 1

typedef struct {
  u64 start;
  u64 end;
  usize page_size;
  usize num_pages;
  bool *alloc_map;
} kheap_t;

kheap_t small_kheap, large_kheap;

bool large_alloc_map[KHEAP_NUM_LARGE_PAGES];
bool small_alloc_map[KHEAP_NUM_SMALL_PAGES];

kheap_t *get_heap_for_type(pagesize_t kind);

void init_kheap() {
  small_kheap.start = (u64)g_kheap;
  small_kheap.page_size = SMALL_PAGE_SIZE;
  small_kheap.num_pages = KHEAP_NUM_SMALL_PAGES;
  small_kheap.alloc_map = small_alloc_map;
  small_kheap.end = small_kheap.start + (small_kheap.page_size * small_kheap.num_pages);

  large_kheap.start = small_kheap.end;
  large_kheap.page_size = LARGE_PAGE_SIZE;
  large_kheap.num_pages = KHEAP_NUM_LARGE_PAGES;
  large_kheap.alloc_map = large_alloc_map;
  large_kheap.end = large_kheap.start + (large_kheap.page_size * large_kheap.num_pages);

  if (!IS_ALIGNED_SMALL(small_kheap.start)) {
    panic("kheap: small kheap top is not aligned");
  }

  if (!IS_ALIGNED_LARGE(large_kheap.start)) {
    panic("kheap: large kheap top is not aligned");
  }

  for (usize i = 0; i < KHEAP_NUM_LARGE_PAGES; i++) {
    large_alloc_map[i] = false;
  }

  for (usize i = 0; i < KHEAP_NUM_SMALL_PAGES; i++) {
    small_alloc_map[i] = false;
  }
}

kheap_t *get_heap_for_type(pagesize_t kind) {
  switch(kind) {
    case PAGE_SMALL: return &small_kheap;
    case PAGE_LARGE: return &large_kheap;

    default:
      panic("get_heap_for_type: unsupported page kind");
      return NULL;
  }
}

void *alloc_page(pagesize_t kind) {
  kheap_t *kheap = get_heap_for_type(kind);

  for (usize i = 0; i < kheap->num_pages; i++) {
    if (!kheap->alloc_map[i]) {
      kheap->alloc_map[i] = true;
      void *rval = (void*)(kheap->start + (kheap->page_size * i));
#ifdef KHEAP_LOGGING
      printf("kheap: alloc 0x%X\n", rval);
#endif // KHEAP_LOGGING
      return rval;
    }
  }

  panic("alloc_page: out of memory");
  return NULL;
}

void free_page(void *p, pagesize_t kind) {
  kheap_t *kheap = get_heap_for_type(kind);
  u64 page_to_free = (u64)p;
  usize heap_idx = (page_to_free - kheap->start) / kheap->page_size;

#ifdef KHEAP_LOGGING
  printf("kheap: free 0x%X\n", p);
#endif // KHEAP_LOGGING

  if (page_to_free < kheap->start || page_to_free >= kheap->end) {
    panic("free_page: tried to free an out of bounds page");
  }

  if (heap_idx > kheap->num_pages) {
    panic("free_page: heap_idx is out of bounds for the given heap");
  }

  if (!kheap->alloc_map[heap_idx]) {
    panic("free_page: tried to free an already freed page");
  }

  kheap->alloc_map[heap_idx] = false;
}
