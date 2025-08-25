#include <defs.h>
#include <page.h>
#include <lib.h>
#include <vm.h>

#define PTE_V    BIT(0)
#define PTE_R    BIT(1)
#define PTE_W    BIT(2)
#define PTE_X    BIT(3)
#define PTE_U    BIT(4)
#define PTE_A    BIT(6)
#define PTE_D    BIT(7)

// bits [53:10] define the PA field of a PTE
#define PTE_PA_MASK   ((MASK(43) << 10ull))
#define PTE_PHYS_SHIFT 2ull

bool pte_is_valid(pte_t *entry) {
  return (*entry & PTE_V) != 0;
}

void pte_set_valid(pte_t *entry, bool valid) {
  if (valid) *entry |= PTE_V;
  else *entry &= ~(PTE_V);
}

bool pte_is_leaf(pte_t *entry, pagelevel_t level) {
  return (*entry & PTE_A) != 0;
}

void pte_set_leaf(pte_t *entry, pagelevel_t level, bool is_leaf) {
  if (is_leaf) *entry |= (PTE_R | PTE_W | PTE_X | PTE_A | PTE_D);
  else *entry &= ~(PTE_R | PTE_W | PTE_X | PTE_A | PTE_D);
}

phys_t pte_get_phys(pte_t *entry) {
  return ((*entry) & PTE_PA_MASK) << PTE_PHYS_SHIFT;
}

void pte_set_phys(pte_t *entry, phys_t pa) {
  if (!IS_ALIGNED_SMALL(pa)) panic("pte_set_phys: misaligned physical address");
  *entry &= ~(PTE_PA_MASK);
  *entry |= (pa >> PTE_PHYS_SHIFT);
}

void pte_set_priv(pte_t *entry, cpu_mode_t priv) {
  switch(priv) {
    case MODE_USER: *entry |= PTE_U;    break;
    case MODE_KERN: *entry &= ~(PTE_U); break;
    default: panic("pte_set_priv: unknown privilege");
  }
}

void pte_set_cache_policy(pte_t *entry, cache_policy_t pol) {
  return;
}
