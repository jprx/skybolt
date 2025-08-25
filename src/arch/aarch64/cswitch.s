.section .text

#include <defs.h>

#define KREG_FIELD(i) [sp,#(8*i)]

.global cswitch
cswitch:
  add sp, sp, -(KREGS_SIZE)
  str x18, KREG_FIELD(0)
  str x19, KREG_FIELD(1)
  str x20, KREG_FIELD(2)
  str x21, KREG_FIELD(3)
  str x22, KREG_FIELD(4)
  str x23, KREG_FIELD(5)
  str x24, KREG_FIELD(6)
  str x25, KREG_FIELD(7)
  str x26, KREG_FIELD(8)
  str x27, KREG_FIELD(9)
  str x28, KREG_FIELD(10)
  str x29, KREG_FIELD(11)
  str x30, KREG_FIELD(12)

  mov x18, sp
  str x18, [x0]
  ldr x18, [x1]
  mov sp, x18

  ldr x30, KREG_FIELD(12)
  ldr x29, KREG_FIELD(11)
  ldr x28, KREG_FIELD(10)
  ldr x27, KREG_FIELD(9)
  ldr x26, KREG_FIELD(8)
  ldr x25, KREG_FIELD(7)
  ldr x24, KREG_FIELD(6)
  ldr x23, KREG_FIELD(5)
  ldr x22, KREG_FIELD(4)
  ldr x21, KREG_FIELD(3)
  ldr x20, KREG_FIELD(2)
  ldr x19, KREG_FIELD(1)
  ldr x18, KREG_FIELD(0)
  add sp, sp, KREGS_SIZE
  ret
