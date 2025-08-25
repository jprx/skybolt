.section .text
.global start
start:
  call main
  li a0, 0
  li a1, 0
  ecall

.global do_syscall
do_syscall:
  ecall
  ret
