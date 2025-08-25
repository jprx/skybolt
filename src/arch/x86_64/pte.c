#include <defs.h>
#include <page.h>
#include <lib.h>
#include <vm.h>

#define PTE_P         BIT(0)
#define PTE_RW        BIT(1)
#define PTE_U         BIT(2)
#define PTE_PWT       BIT(3)
#define PTE_PCD       BIT(4)
#define PTE_PS        BIT(7)

// bits [52:12] define the PA field of a PTE
#define PTE_PA_MASK   ((MASK(40) << 12ull))

bool pte_is_valid(pte_t *entry) {
  return (*entry & PTE_P) != 0;
}

void pte_set_valid(pte_t *entry, bool valid) {
  if (valid) *entry |= (PTE_P | PTE_RW | PTE_U);
  else *entry &= ~(PTE_P);
}

bool pte_is_leaf(pte_t *entry, pagelevel_t level) {
  if (level == PAGE_L1) return true;
  return (*entry & PTE_PS) != 0;
}

void pte_set_leaf(pte_t *entry, pagelevel_t level, bool is_leaf) {
  if (level == PAGE_L1) return;

  // If U/S is set to 0 (supervisor) for any intermediate pages,
  // everything is considered a supervisor page. So, set all
  // non-leaf pages to always be considered user pages, and
  // enforce U/S at the leaf.
  if (is_leaf) *entry |= (PTE_PS);
  else *entry &= ~(PTE_PS) | PTE_U;
}

phys_t pte_get_phys(pte_t *entry) {
  return ((*entry) & PTE_PA_MASK);
}

void pte_set_phys(pte_t *entry, phys_t pa) {
  if (!IS_ALIGNED_SMALL(pa)) panic("pte_set_phys: misaligned physical address");
  *entry &= ~(PTE_PA_MASK);
  *entry |= pa;
}

void pte_set_priv(pte_t *entry, cpu_mode_t priv) {
  switch(priv) {
    case MODE_USER: *entry |= PTE_U;    break;
    case MODE_KERN: *entry &= ~(PTE_U); break;
    default: panic("pte_set_priv: unknown privilege");
  }
}

void pte_set_cache_policy(pte_t *entry, cache_policy_t pol) {
  // Ignore the "PAT" bit of a PTE since we don't need it (only use PA0-PA2)
  *entry &= ~(PTE_PCD);
  *entry &= ~(PTE_PWT);

  switch (pol) {
    // PA0: PAT = 0, PCD = 0, PWT = 0
    case CACHE_WRITEBACK:
      break;

    // PA1: PAT = 0, PCD = 0, PWT = 1
    case CACHE_WRITECOMB:
      *entry |= PTE_PWT;
      break;

    // PA2: PAT = 0, PCD = 1, PWT = 0
    case CACHE_DISABLE:
      *entry |= PTE_PCD;
      break;
  }
}
