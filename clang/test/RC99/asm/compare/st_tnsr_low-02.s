// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm -mcpu gaudi %t.o | FileCheck %s

// TNSR_ID_REG=0, RMW_SEL=0, PARTIAL=0

NOP; NOP; NOP; ST_TNSR_LOW 0, I3, V4
NOP; NOP; NOP; ST_TNSR_LOW 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_LOW 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_LOW 0, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR_LOW  0, I3, VP4
NOP; NOP; NOP; ST_TNSR_LOW  0, I3, VP4,  SP0
NOP; NOP; NOP; ST_TNSR_LOW  0, I3, VP4,  SP1
NOP; NOP; NOP; ST_TNSR_LOW  0, I3, VP4,  !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, !SP1


NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, V4
NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, VP4
NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, !SP1


// TNSR_ID_REG=1, RMW_SEL=0, PARTIAL=0

NOP; NOP; NOP; ST_TNSR_LOW S28, I3, V4
NOP; NOP; NOP; ST_TNSR_LOW S28, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_LOW S28, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_LOW S28, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR_LOW S28, I3, VP4
NOP; NOP; NOP; ST_TNSR_LOW S28, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR_LOW S28, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR_LOW S28, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, V4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, V4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, V4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, VP4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, VP4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, VP4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, VP4, !SP1


NOP; NOP; NOP; ST_TNSR_LOW.PACK S28, I3, V4
NOP; NOP; NOP; ST_TNSR_LOW.PACK S28, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_LOW.PACK S28, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_LOW.PACK S28, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR_LOW.PACK S28, I3, VP4
NOP; NOP; NOP; ST_TNSR_LOW.PACK S28, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR_LOW.PACK S28, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR_LOW.PACK S28, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, V4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, V4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, V4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, VP4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, VP4, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, VP4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, VP4, !SP1


// TNSR_ID_REG=0, RMW_SEL=1, PARTIAL=0

NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL 0, I3, V4, S29
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL 0, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL 0, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL 0, I3, V4, S29, !SP1

NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL 0, I3, VP4, S29
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL 0, I3, VP4, S29, SP0
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL 0, I3, VP4, S29, SP1
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL 0, I3, VP4, S29, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, S29, !SP1


NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, V4, S29
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, V4, S29, !SP1

NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, VP4, S29
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, VP4, S29, SP0
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, VP4, S29, SP1
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, VP4, S29, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, S29, !SP1


// implied RMW_SEL

NOP; NOP; NOP; ST_TNSR_LOW 0, I3, V4, S29
NOP; NOP; NOP; ST_TNSR_LOW 0, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR_LOW 0, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR_LOW 0, I3, V4, S29, !SP1

NOP; NOP; NOP; ST_TNSR_LOW 0, I3, VP4, S29
NOP; NOP; NOP; ST_TNSR_LOW 0, I3, VP4, S29, SP0
NOP; NOP; NOP; ST_TNSR_LOW 0, I3, VP4, S29, SP1
NOP; NOP; NOP; ST_TNSR_LOW 0, I3, VP4, S29, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, S29, !SP1


NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, V4, S29
NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, V4, S29, !SP1

NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, VP4, S29
NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, VP4, S29, SP0
NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, VP4, S29, SP1
NOP; NOP; NOP; ST_TNSR_LOW.PACK 0, I3, VP4, S29, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, S29, !SP1


// implied S29

NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL 0, I3, V4
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL 0, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL  0, I3, VP4
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL  0, I3, VP4,  SP0
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL  0, I3, VP4,  SP1
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL  0, I3, VP4,  !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  0x0, I3, VP4, S29, !SP1


NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, V4
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, VP4
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK 0, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack 0x0, I3, VP4, S29, !SP1


// TNSR_ID_REG=1, RMW_SEL=1, PARTIAL=0

NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL S28, I3, V4, S29
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL S28, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL S28, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL S28, I3, V4, S29, !SP1

NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL S28, I3, VP4, S29
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL S28, I3, VP4, S29, SP0
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL S28, I3, VP4, S29, SP1
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL S28, I3, VP4, S29, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, VP4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  S28, I3, VP4, S29, !SP1


NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK S28, I3, V4
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK S28, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK S28, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK S28, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK S28, I3, VP4
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK S28, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK S28, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR_LOW.RMW_SEL.PACK S28, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, V4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, VP4, S29, SP0
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, VP4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_low  pack S28, I3, VP4, S29, !SP1
