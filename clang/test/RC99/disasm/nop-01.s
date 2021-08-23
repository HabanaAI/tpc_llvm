// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

	.text
	.globl	main
main:
	nop; nop; nop; nop

// CHECK: nop; nop; nop; nop
