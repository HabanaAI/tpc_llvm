// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s

	.text
	.globl	main

NOP; NOP; NOP; ASO
NOP; NOP; NOP; ASO DEC
NOP; NOP; NOP; ASO VPU
NOP; NOP; NOP; ASO DEC, VPU

// CHECK:      nop;    nop;    nop;    aso 
// CHECK-NEXT: nop;    nop;    nop;    aso  dec
// CHECK-NEXT: nop;    nop;    nop;    aso  vpu
// CHECK-NEXT: nop;    nop;    nop;    aso  dec vpu

NOP; NOP; NOP; ASO SP0
NOP; NOP; NOP; ASO SP1
NOP; NOP; NOP; ASO SP2
NOP; NOP; NOP; ASO SP13
NOP; NOP; NOP; ASO SP14
NOP; NOP; NOP; ASO SP15

// CHECK-NEXT: nop;    nop;    nop;    aso 
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

NOP; NOP; NOP; ASO DEC SP4
NOP; NOP; NOP; ASO VPU SP4
NOP; NOP; NOP; ASO DEC, VPU, SP4

// CHECK-NEXT: nop;    nop;    nop;    aso  dec SP4
// CHECK-NEXT: nop;    nop;    nop;    aso  vpu SP4
// CHECK-NEXT: nop;    nop;    nop;    aso  dec vpu SP4

NOP; NOP; NOP; ASO DEC !SP4
NOP; NOP; NOP; ASO VPU !SP4
NOP; NOP; NOP; ASO DEC, VPU, !SP4

// CHECK-NEXT: nop;    nop;    nop;    aso  dec !SP4
// CHECK-NEXT: nop;    nop;    nop;    aso  vpu !SP4
// CHECK-NEXT: nop;    nop;    nop;    aso  dec vpu !SP4
