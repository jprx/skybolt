#pragma once
#define BIT(n)      (((1ull << (n))))
#define MASK(n)     (((BIT(n) - 1)))

#define KERNEL_NAME "skybolt"
#define KERNEL_VERS "1.0.0"

#ifdef CONFIG_RELEASE
#define KERNEL_VARIANT "release"
#endif // CONFIG_RELEASE

#ifdef CONFIG_DEBUG
#define KERNEL_VARIANT "debug"
#endif // CONFIG_DEBUG

#ifndef KERNEL_VARIANT
#define KERNEL_VARIANT "unknown"
#endif // !defined(KERNEL_VARIANT)

#define KERNEL_LINK_ADDRESS ((0xFFFFFF0000000000ull))
#define KERNEL_DEVICE_MAP   ((0xFFFFDD0000000000ull))
#define WRITE_COMB_MAP      ((0xFFFFCC0000000000ull))

#define NORETURN __attribute__((noreturn))
#define INLINE   __attribute__((always_inline))
#define KTHREAD NORETURN

#define ONE_KB                     BIT(10)
#define ONE_MB                     BIT(20)
#define ONE_GB                     BIT(30)

// For 48 bit 4K paging
// Each size is 2^9 = 512 times larger than the previous
#define SMALL_PAGE_SIZE            ((4 * ONE_KB))
#define LARGE_PAGE_SIZE            ((2 * ONE_MB))
#define GIANT_PAGE_SIZE            ((1 * ONE_GB))
#define SUPER_PAGE_SIZE            ((512 * ONE_GB))

#define KHEAP_NUM_LARGE_PAGES      256
#define KHEAP_NUM_SMALL_PAGES      1024

#define IS_ALIGNED_SMALL(x)        (((0 == ((x) & (SMALL_PAGE_SIZE - 1)))))
#define IS_ALIGNED_LARGE(x)        (((0 == ((x) & (LARGE_PAGE_SIZE - 1)))))
#define IS_ALIGNED_GIANT(x)        (((0 == ((x) & (GIANT_PAGE_SIZE - 1)))))
#define IS_ALIGNED_SUPER(x)        (((0 == ((x) & (SUPER_PAGE_SIZE - 1)))))

#define PAGE_BITS_PER_LEVEL   ((9ull))
#define PAGE_OFFSET_BITS      ((12ull))
#define NUM_PT_ENTRIES        ((SMALL_PAGE_SIZE / sizeof(u64)))

#define KERN_V2P(x) ((((u64)x) & (~(KERNEL_LINK_ADDRESS))))
#define KERN_P2V(x) ((((u64)x) |  ((KERNEL_LINK_ADDRESS))))

#define KERN_DV2P(x) ((((u64)x) & (~(KERNEL_DEVICE_MAP))))
#define KERN_P2DV(x) ((((u64)x) |  ((KERNEL_DEVICE_MAP))))

#define KERN_WC2P(x) ((((u64)x) & (~(WRITE_COMB_MAP))))
#define KERN_P2WC(x) ((((u64)x) |  ((WRITE_COMB_MAP))))

// ROUND_DOWN_POW2: Assuming sz is a power of two, ANDing with ~(sz-1) will
// mask off all bits within a sz-sized block, which will effectively round down
// x to the lowest sz aligned address. If x is already aligned, then this does
// not modify x.
//
// ROUND_UP_POW2: Adding (sz-1) to x before rounding down will push x into the
// next sz aligned block if x is not already aligned. Then we mask off the sz
// bits to fully align the address, just like we did for the round down case.
// If x was already sz aligned, adding (sz-1) followed by ANDing with ~(sz-1)
// does not modify x. For all other cases, this will return the next sz aligned
// address larger than x.
#define ROUND_DOWN_POW2(x,sz)      ((       (x)        & (~((sz) - 1)) ))
#define ROUND_UP_POW2(x,sz)        (( ((x) + ((sz)-1)) & (~((sz) - 1)) ))

#define ALIGN_SMALL_PAGE(x)    ((ROUND_DOWN_POW2(x, SMALL_PAGE_SIZE)))
#define ALIGN_LARGE_PAGE(x)    ((ROUND_DOWN_POW2(x, LARGE_PAGE_SIZE)))
#define ALIGN_GIANT_PAGE(x)    ((ROUND_DOWN_POW2(x, GIANT_PAGE_SIZE)))
#define ALIGN_SUPER_PAGE(x)    ((ROUND_DOWN_POW2(x, SUPER_PAGE_SIZE)))

// Returns 16-byte aligned bottom of a stack page
// 16 byte alignment is required for some ISAs
// Assumption: using a small page for this stack
#define STACK_BOTTOM(x)  (( (x) + (SMALL_PAGE_SIZE - (0x10)) ))

#define MAX_INITRD_SIZE     ((128 * ONE_MB))

#ifdef ARCH_X86_64
#define ARCH_NAME "x86_64"
#include <arch/x86_64/defs.h>
#endif // ARCH_X86_64

#ifdef ARCH_AARCH64
#define ARCH_NAME "aarch64"
#include <arch/aarch64/defs.h>
#endif // ARCH_AARCH64

#ifdef ARCH_RISCV64
#define ARCH_NAME "riscv64"
#include <arch/riscv64/defs.h>
#endif // ARCH_RISCV64
