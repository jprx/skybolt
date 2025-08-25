.section .text.start
.code32
.intel_syntax noprefix

#include <defs.h>

// Entrypoint to the kernel:
// Multiboot stores a pointer to the multiboot info tables in ebx, so make sure
// not to touch ebx during early init until we can write it to memory when we
// get into the higher half!
.global bootup
bootup:
  cli // Make sure interrupts are OFF!

  // AMD Manual Vol 2: System Programming, Section 14.6:
  // Enabling and Activating Long Mode

  // 1. Set CR4.PAE to 1 to enable PAE paging
  mov eax, cr4
  bts eax, CR4_PAE_BIT
  mov cr4, eax

  // 2. Load our bringup page table
  lea eax, [bringup_pagetable]
  mov cr3, eax

  // 3. Enable long mode (this is not the same thing as activating it)
  mov ecx, MSR_EFER
  rdmsr
  bts eax, EFER_LME_BIT
  bts eax, EFER_SCE_BIT
  wrmsr

  // 4. Enable paging- this activates long mode
  mov eax, cr0
  bts eax, CR0_PG_BIT
  mov cr0, eax

  // 5. Now, we need a GDT with 64 bit code and data segments
  lgdt [gdt_ptr]
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  // This is a "far jump" that reloads the CS selector with GDT entry 0x08.
  // This is what causes us to switch the CPU from 32 bit mode -> 64 bit mode.
  // Can't use a far return or far call because we don't have a stack yet.
  ljmp 0x0008, enter_long_mode

// We're in long (64 bit) mode, so we use .code64 to tell the assembler
// to use 64 bit instructions from here on out.
.code64
enter_long_mode:
  // Setup higher half stack
  lea rax, bringup_stack_top
  mov rcx, KERNEL_LINK_ADDRESS
  add rax, rcx
  mov rsp, rax

  // Jump to the higher half of the address space
  // Can't just lea start_higher_half because it's too far away
  // Instead, we use the _higher_half_load_addr load memory address from the linker,
  // and manually add the higher half offset to it.
  lea rax, _higher_half_load_addr
  mov rcx, KERNEL_LINK_ADDRESS
  add rax, rcx
  jmp rax

.balign 8
bringup_stack:
  .rept 2048
  .quad 0
  .endr
bringup_stack_top:
  .quad 0

.balign SMALL_PAGE_SIZE
bringup_pagetable:
  .rept 512
  .quad (bringup_pgtable_l3 + 0x03)
  .endr

.balign SMALL_PAGE_SIZE
bringup_pgtable_l3:
  .rept 512
  .quad 0x83
  .endr

bringup_gdt:
  .quad 0x00000000000000 // 0x00: NULL entry (first entry must be zero)
  .quad 0x20980000000000 // 0x08: Code segment (P | L | DPL = 00)
  .quad 0x00920000000000 // 0x10: Data segment (P | W | DPL = 00)
  // Note that the W bit is ignored according to the AMD manual, but
  // in practice on Intel CPUs I've found that you need W set when setting
  // up a stack selector. The Intel manual says that W must be set for ss as well.
gdt_end:
  .quad 0

gdt_ptr:
  .word (gdt_end - bringup_gdt - 1)
  .quad bringup_gdt

.section .text.start64
.code64

.global start_higher_half
start_higher_half:
  lea rax, [rip + multiboot_info_ptr]
  mov [rax], ebx
  call kmain
  hlt

.section .data
.global multiboot_info_ptr
multiboot_info_ptr:
  .quad 0

.section .text
// reload_segments(rdi: new ds selector, rsi: new cs selector)
// call this whenever the gdt or what's in it changes
.global reload_segments
reload_segments:
  mov ax, di
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  lea rax, [rip + 1f]
  // this is a "far return" and it works just like a normal ret
  // except it also pops a new cs off the stack.
  // we use this as a mechanism for reloading cs since you
  // can't just write to it.
  push rsi
  push rax
  retfq
1:
  ret
