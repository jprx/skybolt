.section .text
.global start
start:
  bl main
  mov x0, #0
  mov x1, #0
  svc #0

.global do_syscall
do_syscall:
  svc #0
  ret
