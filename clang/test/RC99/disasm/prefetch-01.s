// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm -mcpu=gaudi %t.o | FileCheck %s

	.text
	.globl	main
main:
// CHECK: main:

PREFETCH AD0
PREFETCH AD1
PREFETCH AD2
PREFETCH AD3
PREFETCH AD4
PREFETCH AD5
PREFETCH AD6
PREFETCH AD7

// CHECK-NEXT: prefetch AD0, SP0;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD1, SP0;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD2, SP0;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD3, SP0;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD4, SP0;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD5, SP0;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD6, SP0;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD7, SP0;    nop;    nop;    nop


PREFETCH AD0, SP7
PREFETCH AD1, SP6
PREFETCH AD2, SP5
PREFETCH AD3, SP4
PREFETCH AD4, SP3
PREFETCH AD5, SP2
PREFETCH AD6, SP1
PREFETCH AD7, SP0

// CHECK-NEXT: prefetch AD0, SP7;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD1, SP6;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD2, SP5;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD3, SP4;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD4, SP3;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD5, SP2;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD6, SP1;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD7, SP0;    nop;    nop;    nop


PREFETCH AD0, SP15
PREFETCH AD1, SP14
PREFETCH AD2, SP13
PREFETCH AD3, SP12
PREFETCH AD4, SP11
PREFETCH AD5, SP10
PREFETCH AD6, SP9
PREFETCH AD7, SP8

// CHECK-NEXT: prefetch AD0, SP15;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD1, SP14;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD2, SP13;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD3, SP12;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD4, SP11;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD5, SP10;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD6, SP9;    nop;    nop;    nop
// CHECK-NEXT: prefetch AD7, SP8;    nop;    nop;    nop


PREFETCH AD0, !SP7
PREFETCH AD1, !SP6
PREFETCH AD2, !SP5
PREFETCH AD3, !SP4
PREFETCH AD4, !SP3
PREFETCH AD5, !SP2
PREFETCH AD6, !SP1
PREFETCH AD7, !SP0

// CHECK-NEXT: prefetch AD0, !SP7;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD1, !SP6;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD2, !SP5;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD3, !SP4;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD4, !SP3;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD5, !SP2;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD6, !SP1;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD7, !SP0;   nop;    nop;    nop


PREFETCH AD0, !SP15
PREFETCH AD1, !SP14
PREFETCH AD2, !SP13
PREFETCH AD3, !SP12
PREFETCH AD4, !SP11
PREFETCH AD5, !SP10
PREFETCH AD6, !SP9
PREFETCH AD7, !SP8

// CHECK-NEXT: prefetch AD0, !SP15;  nop;    nop;    nop
// CHECK-NEXT: prefetch AD1, !SP14;  nop;    nop;    nop
// CHECK-NEXT: prefetch AD2, !SP13;  nop;    nop;    nop
// CHECK-NEXT: prefetch AD3, !SP12;  nop;    nop;    nop
// CHECK-NEXT: prefetch AD4, !SP11;  nop;    nop;    nop
// CHECK-NEXT: prefetch AD5, !SP10;  nop;    nop;    nop
// CHECK-NEXT: prefetch AD6, !SP9;   nop;    nop;    nop
// CHECK-NEXT: prefetch AD7, !SP8;   nop;    nop;    nop
