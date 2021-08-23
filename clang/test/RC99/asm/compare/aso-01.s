// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

	.text
	.globl	main
main:
// CHECK: main:


NOP; NOP; NOP; ASO
NOP; NOP; NOP; ASO VPU
NOP; NOP; NOP; ASO VPU SP4
NOP; NOP; NOP; ASO VPU !SP4

// CHECK-NEXT: nop;    nop;    nop;    aso  SP0
// CHECK-NEXT: nop;    nop;    nop;    aso  vpu SP0
// CHECK-NEXT: nop;    nop;    nop;    aso  vpu SP4
// CHECK-NEXT: nop;    nop;    nop;    aso  vpu !SP4


NOP; NOP; NOP; ASO SP0
NOP; NOP; NOP; ASO SP1
NOP; NOP; NOP; ASO SP2
NOP; NOP; NOP; ASO SP13
NOP; NOP; NOP; ASO SP14
NOP; NOP; NOP; ASO SP15

// CHECK-NEXT: nop;    nop;    nop;    aso  SP0
// CHECK-NEXT: nop;    nop;    nop;    aso  SP1
// CHECK-NEXT: nop;    nop;    nop;    aso  SP2
// CHECK-NEXT: nop;    nop;    nop;    aso  SP13
// CHECK-NEXT: nop;    nop;    nop;    aso  SP14
// CHECK-NEXT: nop;    nop;    nop;    aso  SP15


NOP; NOP; NOP; ASO !SP0
NOP; NOP; NOP; ASO !SP1
NOP; NOP; NOP; ASO !SP2
NOP; NOP; NOP; ASO !SP13
NOP; NOP; NOP; ASO !SP14
NOP; NOP; NOP; ASO !SP15

// CHECK-NEXT: nop;    nop;    nop;    aso  !SP0
// CHECK-NEXT: nop;    nop;    nop;    aso  !SP1
// CHECK-NEXT: nop;    nop;    nop;    aso  !SP2
// CHECK-NEXT: nop;    nop;    nop;    aso  !SP13
// CHECK-NEXT: nop;    nop;    nop;    aso  !SP14
// CHECK-NEXT: nop;    nop;    nop;    aso  !SP15


