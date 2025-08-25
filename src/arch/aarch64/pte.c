#include <defs.h>
#include <page.h>
#include <lib.h>
#include <vm.h>

#define PTE_P         BIT(0)
#define PTE_SIZE      BIT(1)
#define PTE_U         BIT(6)
#define PTE_AF        BIT(10)

// bits [52:12] define the PA field of a PTE
#define PTE_PA_MASK   ((MASK(40) << 12ull))

#define MEMATTR0      ((0 << 2))
#define MEMATTR1      ((1 << 2))
#define ATTRMASK      ((MASK(3) << 2))

bool pte_is_valid(pte_t *entry) {
  return *entry & PTE_P;
}

void pte_set_valid(pte_t *entry, bool valid) {
  if (valid) *entry |= (PTE_P | PTE_AF);
  else *entry &= ~(PTE_P);
}

bool pte_is_leaf(pte_t *entry, pagelevel_t level) {
  if (level == PAGE_L1) return true;
  return ((*entry) & PTE_SIZE) == 0;
}

void pte_set_leaf(pte_t *entry, pagelevel_t level, bool is_leaf) {
  if (level == PAGE_L1 || !is_leaf) *entry |= PTE_SIZE;
  else *entry &= ~(PTE_SIZE);
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
  *entry &= ~(ATTRMASK);
  switch(pol) {
    // MAIR0: Normal (Writeback cacheable)
    case CACHE_WRITEBACK:
      *entry |= MEMATTR0;
      break;

    // MAIR1: Device-nGnRnE
    case CACHE_DISABLE:
    case CACHE_WRITECOMB:
      *entry |= MEMATTR1;
      break;
  }
}
