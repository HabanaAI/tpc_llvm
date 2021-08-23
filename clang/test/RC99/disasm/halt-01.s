// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

	.text
	.globl	main
main:
	nop; halt; halt; nop
	nop; nop ; halt; nop
	nop; halt; nop ; nop

// CHECK: nop; halt; halt; nop
// CHECK: nop; nop; halt; nop
// CHECK: nop; halt; nop; nop
