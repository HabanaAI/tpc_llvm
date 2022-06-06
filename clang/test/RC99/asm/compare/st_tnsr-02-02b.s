// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudib %s -o %t.o
// RUN: %disasm --mcpu gaudib %t.o | FileCheck %s

// TNSR_ID_REG=0, RMW_SEL=0, PARTIAL=0

NOP; NOP; NOP; ST_TNSR 0, I3, V4
NOP; NOP; NOP; ST_TNSR 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR 0, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR  0, I3, VP4
NOP; NOP; NOP; ST_TNSR  0, I3, VP4,  SP0
NOP; NOP; NOP; ST_TNSR  0, I3, VP4,  SP1
NOP; NOP; NOP; ST_TNSR  0, I3, VP4,  !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, VP4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, VP4, !SP1


NOP; NOP; NOP; ST_TNSR.PACK 0, I3, V4
NOP; NOP; NOP; ST_TNSR.PACK 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR.PACK 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR.PACK 0, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR.PACK 0, I3, VP4
NOP; NOP; NOP; ST_TNSR.PACK 0, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR.PACK 0, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR.PACK 0, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, VP4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, VP4, !SP1


// TNSR_ID_REG=1, RMW_SEL=0, PARTIAL=0

NOP; NOP; NOP; ST_TNSR S28, I3, V4
NOP; NOP; NOP; ST_TNSR S28, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR S28, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR S28, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR S28, I3, VP4
NOP; NOP; NOP; ST_TNSR S28, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR S28, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR S28, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, !SP1


NOP; NOP; NOP; ST_TNSR.PACK S28, I3, V4
NOP; NOP; NOP; ST_TNSR.PACK S28, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR.PACK S28, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR.PACK S28, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR.PACK S28, I3, VP4
NOP; NOP; NOP; ST_TNSR.PACK S28, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR.PACK S28, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR.PACK S28, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, !SP1


// TNSR_ID_REG=0, RMW_SEL=1, PARTIAL=0

NOP; NOP; NOP; ST_TNSR.RMW_SEL 0, I3, V4, S29
NOP; NOP; NOP; ST_TNSR.RMW_SEL 0, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL 0, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL 0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, S29, !SP1


NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK 0, I3, V4, S29
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK 0, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK 0, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK 0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, S29, !SP1


// implied RMW_SEL

NOP; NOP; NOP; ST_TNSR 0, I3, V4, S29
NOP; NOP; NOP; ST_TNSR 0, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR 0, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR 0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, S29, !SP1


NOP; NOP; NOP; ST_TNSR.PACK 0, I3, V4, S29
NOP; NOP; NOP; ST_TNSR.PACK 0, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR.PACK 0, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR.PACK 0, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, S29, !SP1


// implied S29

NOP; NOP; NOP; ST_TNSR.RMW_SEL 0, I3, V4
NOP; NOP; NOP; ST_TNSR.RMW_SEL 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL 0, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x0, I3, V4, S29, !SP1


NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK 0, I3, V4
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK 0, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK 0, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK 0, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x0, I3, V4, S29, !SP1


// TNSR_ID_REG=1, RMW_SEL=1, PARTIAL=0

NOP; NOP; NOP; ST_TNSR.RMW_SEL S28, I3, V4, S29
NOP; NOP; NOP; ST_TNSR.RMW_SEL S28, I3, V4, S29, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL S28, I3, V4, S29, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL S28, I3, V4, S29, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S29, !SP1


NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK S28, I3, V4
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK S28, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK S28, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK S28, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S29
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S29, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S29, !SP1


// TNSR_ID_REG=0, RMW_SEL=0, PARTIAL=1

NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, V4, S31
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, V4, S31, SP0
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, V4, S31, SP1
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, V4, S31, !SP1

NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, VP4, S31
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, VP4, S31, SP0
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, VP4, S31, SP1
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, VP4, S31, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, VP4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, VP4, S31, !SP1


NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, V4, S31
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, V4, S31, SP0
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, V4, S31, SP1
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, V4, S31, !SP1

NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, VP4, S31
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, VP4, S31, SP0
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, VP4, S31, SP1
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, VP4, S31, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, VP4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, VP4, S31, !SP1


// Implied S31

NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, V4
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, VP4
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, VP4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, VP4, S31, !SP1


NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, V4
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, VP4
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, VP4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, VP4, S31, !SP1


// TNSR_ID_REG=1, RMW_SEL=0, PARTIAL=1

NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, V4, S31
NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, V4, S31, SP0
NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, V4, S31, SP1
NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, V4, S31, !SP1

NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, VP4, S31
NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, VP4, S31, SP0
NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, VP4, S31, SP1
NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, VP4, S31, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, S31, !SP1


NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, V4, S31
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, V4, S31, SP0
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, V4, S31, SP1
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, V4, S31, !SP1

NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, VP4, S31
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, VP4, S31, SP0
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, VP4, S31, SP1
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, VP4, S31, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, S31, !SP1

// Implied S31

NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, V4
NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, VP4
NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR.PARTIAL S28, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, S31, !SP1


NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, V4
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, V4, !SP1

NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, VP4
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, VP4, SP0
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, VP4, SP1
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL S28, I3, VP4, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, S31, !SP1


// Implied PARTIAL

NOP; NOP; NOP; ST_TNSR S28, I3, V4, S31
NOP; NOP; NOP; ST_TNSR S28, I3, V4, S31, SP0
NOP; NOP; NOP; ST_TNSR S28, I3, V4, S31, SP1
NOP; NOP; NOP; ST_TNSR S28, I3, V4, S31, !SP1

NOP; NOP; NOP; ST_TNSR S28, I3, VP4, S31
NOP; NOP; NOP; ST_TNSR S28, I3, VP4, S31, SP0
NOP; NOP; NOP; ST_TNSR S28, I3, VP4, S31, SP1
NOP; NOP; NOP; ST_TNSR S28, I3, VP4, S31, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, VP4, S31, !SP1


NOP; NOP; NOP; ST_TNSR.PACK S28, I3, V4, S31
NOP; NOP; NOP; ST_TNSR.PACK S28, I3, V4, S31, SP0
NOP; NOP; NOP; ST_TNSR.PACK S28, I3, V4, S31, SP1
NOP; NOP; NOP; ST_TNSR.PACK S28, I3, V4, S31, !SP1

NOP; NOP; NOP; ST_TNSR.PACK S28, I3, VP4, S31
NOP; NOP; NOP; ST_TNSR.PACK S28, I3, VP4, S31, SP0
NOP; NOP; NOP; ST_TNSR.PACK S28, I3, VP4, S31, SP1
NOP; NOP; NOP; ST_TNSR.PACK S28, I3, VP4, S31, !SP1

// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, VP4, S31, !SP1


// TNSR_ID_REG=0, RMW_SEL=1, PARTIAL=1

NOP; NOP; NOP; ST_TNSR.RMW_SEL.PARTIAL 1, I3, V4, S29, S31
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PARTIAL 1, I3, V4, S29, S31, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PARTIAL 1, I3, V4, S29, S31, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PARTIAL 1, I3, V4, S29, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31, !SP1


NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK.PARTIAL 1, I3, V4, S29, S31
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK.PARTIAL 1, I3, V4, S29, S31, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK.PARTIAL 1, I3, V4, S29, S31, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK.PARTIAL 1, I3, V4, S29, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31, !SP1


// Implied PARTIAL

NOP; NOP; NOP; ST_TNSR.RMW_SEL 1, I3, V4, S29, S31
NOP; NOP; NOP; ST_TNSR.RMW_SEL 1, I3, V4, S29, S31, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL 1, I3, V4, S29, S31, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL 1, I3, V4, S29, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31, !SP1


NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK 1, I3, V4, S29, S31
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK 1, I3, V4, S29, S31, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK 1, I3, V4, S29, S31, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK 1, I3, V4, S29, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31, !SP1

// Implied RMW_SEL

NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, V4, S29, S31
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, V4, S29, S31, SP0
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, V4, S29, S31, SP1
NOP; NOP; NOP; ST_TNSR.PARTIAL 1, I3, V4, S29, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31, !SP1


NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, V4, S29, S31
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, V4, S29, S31, SP0
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, V4, S29, S31, SP1
NOP; NOP; NOP; ST_TNSR.PACK.PARTIAL 1, I3, V4, S29, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31, !SP1


// Implied PARTIAL and RMW_SEL

NOP; NOP; NOP; ST_TNSR 1, I3, V4, S29, S31
NOP; NOP; NOP; ST_TNSR 1, I3, V4, S29, S31, SP0
NOP; NOP; NOP; ST_TNSR 1, I3, V4, S29, S31, SP1
NOP; NOP; NOP; ST_TNSR 1, I3, V4, S29, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31, !SP1


NOP; NOP; NOP; ST_TNSR.PACK 1, I3, V4, S29, S31
NOP; NOP; NOP; ST_TNSR.PACK 1, I3, V4, S29, S31, SP0
NOP; NOP; NOP; ST_TNSR.PACK 1, I3, V4, S29, S31, SP1
NOP; NOP; NOP; ST_TNSR.PACK 1, I3, V4, S29, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31, !SP1


// Implied S29 and S31

NOP; NOP; NOP; ST_TNSR.RMW_SEL.PARTIAL 1, I3, V4
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PARTIAL 1, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PARTIAL 1, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PARTIAL 1, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  0x1, I3, V4, S29, S31, !SP1

NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK.PARTIAL 1, I3, V4
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK.PARTIAL 1, I3, V4, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK.PARTIAL 1, I3, V4, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK.PARTIAL 1, I3, V4, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack 0x1, I3, V4, S29, S31, !SP1


// TNSR_ID_REG=0, RMW_SEL=1, PARTIAL=1

NOP; NOP; NOP; ST_TNSR.RMW_SEL.PARTIAL S28, I3, V4, S29, S31
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PARTIAL S28, I3, V4, S29, S31, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PARTIAL S28, I3, V4, S29, S31, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PARTIAL S28, I3, V4, S29, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S29, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  S28, I3, V4, S29, S31, !SP1


NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK.PARTIAL S28, I3, V4, S29, S31
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK.PARTIAL S28, I3, V4, S29, S31, SP0
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK.PARTIAL S28, I3, V4, S29, S31, SP1
NOP; NOP; NOP; ST_TNSR.RMW_SEL.PACK.PARTIAL S28, I3, V4, S29, S31, !SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S29, S31
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S29, S31, SP1
// CHECK: nop; 	nop; 	nop; 	st_tnsr  pack S28, I3, V4, S29, S31, !SP1
