// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s

// TNSR_ID_REG=0, RMW_SEL=0, PARTIAL=0

NOP; NOP; NOP; ST_TNSR_HIGH 0, I3, V4
NOP; NOP; NOP; ST_TNSR_HIGH 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_HIGH 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_HIGH 0, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR_HIGH  0, I3, VP4
NOP; NOP; NOP; ST_TNSR_HIGH  0, I3, VP4,  SP0
NOP; NOP; NOP; ST_TNSR_HIGH  0, I3, VP4,  SP1
NOP; NOP; NOP; ST_TNSR_HIGH  0, I3, VP4,  !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, VP4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, VP4, !SP1


NOP; NOP; NOP; ST_TNSR_HIGH.PACK 0, I3, V4
NOP; NOP; NOP; ST_TNSR_HIGH.PACK 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_HIGH.PACK 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_HIGH.PACK 0, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR_HIGH.PACK 0, I3, VP4
NOP; NOP; NOP; ST_TNSR_HIGH.PACK 0, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR_HIGH.PACK 0, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR_HIGH.PACK 0, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, VP4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, VP4, !SP1


// TNSR_ID_REG=1, RMW_SEL=0, PARTIAL=0

NOP; NOP; NOP; ST_TNSR_HIGH S28, I3, V4
NOP; NOP; NOP; ST_TNSR_HIGH S28, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_HIGH S28, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_HIGH S28, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR_HIGH S28, I3, VP4
NOP; NOP; NOP; ST_TNSR_HIGH S28, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR_HIGH S28, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR_HIGH S28, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  S28, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  S28, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  S28, I3, V4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  S28, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  S28, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  S28, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  S28, I3, VP4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  S28, I3, VP4, !SP1


NOP; NOP; NOP; ST_TNSR_HIGH.PACK S28, I3, V4
NOP; NOP; NOP; ST_TNSR_HIGH.PACK S28, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_HIGH.PACK S28, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_HIGH.PACK S28, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR_HIGH.PACK S28, I3, VP4
NOP; NOP; NOP; ST_TNSR_HIGH.PACK S28, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR_HIGH.PACK S28, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR_HIGH.PACK S28, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack S28, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack S28, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack S28, I3, V4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack S28, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack S28, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack S28, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack S28, I3, VP4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack S28, I3, VP4, !SP1


// TNSR_ID_REG=0, RMW_SEL=1, PARTIAL=0

NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL 0, I3, V4, S29
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL 0, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL 0, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL 0, I3, V4, S29, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, S29, !SP1


NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL.PACK 0, I3, V4, S29
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL.PACK 0, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL.PACK 0, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL.PACK 0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, S29, !SP1


// implied RMW_SEL

NOP; NOP; NOP; ST_TNSR_HIGH 0, I3, V4, S29
NOP; NOP; NOP; ST_TNSR_HIGH 0, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR_HIGH 0, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR_HIGH 0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, S29, !SP1

NOP; NOP; NOP; ST_TNSR_HIGH.PACK 0, I3, V4, S29
NOP; NOP; NOP; ST_TNSR_HIGH.PACK 0, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR_HIGH.PACK 0, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR_HIGH.PACK 0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, S29, !SP1


// implied S29

NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL 0, I3, V4
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL 0, I3, V4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  0x0, I3, V4, S29, !SP1


NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL.PACK 0, I3, V4
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL.PACK 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL.PACK 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL.PACK 0, I3, V4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack 0x0, I3, V4, S29, !SP1


// TNSR_ID_REG=1, RMW_SEL=1, PARTIAL=0

NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL S28, I3, V4, S29
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL S28, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL S28, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL S28, I3, V4, S29, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  S28, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  S28, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  S28, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  S28, I3, V4, S29, !SP1

NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL.PACK S28, I3, V4
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL.PACK S28, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL.PACK S28, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR_HIGH.RMW_SEL.PACK S28, I3, V4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack S28, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack S28, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack S28, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr_high  pack S28, I3, V4, S29, !SP1
