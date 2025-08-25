.section .text

#include <defs.h>

#define KREG_FIELD(i) (8*i)(sp)

.global cswitch
cswitch:
  add sp, sp, -(KREGS_SIZE)
  sd  s0,  KREG_FIELD(0)
  sd  s1,  KREG_FIELD(1)
  sd  s2,  KREG_FIELD(2)
  sd  s3,  KREG_FIELD(3)
  sd  s4,  KREG_FIELD(4)
  sd  s5,  KREG_FIELD(5)
  sd  s6,  KREG_FIELD(6)
  sd  s7,  KREG_FIELD(7)
  sd  s8,  KREG_FIELD(8)
  sd  s9,  KREG_FIELD(9)
  sd  s10, KREG_FIELD(10)
  sd  s11, KREG_FIELD(11)
  sd  ra,  KREG_FIELD(12)

  sd sp, (a0)
  ld sp, (a1)

  ld ra,  KREG_FIELD(12)
  ld s11, KREG_FIELD(11)
  ld s10, KREG_FIELD(10)
  ld s9,  KREG_FIELD(9)
  ld s8,  KREG_FIELD(8)
  ld s7,  KREG_FIELD(7)
  ld s6,  KREG_FIELD(6)
  ld s5,  KREG_FIELD(5)
  ld s4,  KREG_FIELD(4)
  ld s3,  KREG_FIELD(3)
  ld s2,  KREG_FIELD(2)
  ld s1,  KREG_FIELD(1)
  ld s0,  KREG_FIELD(0)
  add sp, sp, KREGS_SIZE
  ret
