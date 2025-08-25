.section .text
.intel_syntax noprefix

.global start
start:
  call main

  // Call exit(0)
  mov rax, 0
  mov rdi, 0
  syscall

.global do_syscall
// syscall ABI is (rax: sys num, rdi: arg0, rsi: arg1, rdx: arg2, r10: arg3, r8: arg4)
do_syscall:
  mov rax, rdi
  mov rdi, rsi
  mov rsi, rdx
  mov rdx, rcx
  mov r10, r8
  mov r8, r9
  syscall
  ret
