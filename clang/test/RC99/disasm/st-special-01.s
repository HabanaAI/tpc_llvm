// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

	.text
	.globl	main
main:

nop; nop; nop; cache_flush
nop; nop; nop; cache_flush SP0
nop; nop; nop; cache_flush SP1
nop; nop; nop; cache_flush SP2
nop; nop; nop; cache_flush SP13
nop; nop; nop; cache_flush SP14
nop; nop; nop; cache_flush SP15
nop; nop; nop; cache_flush !SP0
nop; nop; nop; cache_flush !SP1
nop; nop; nop; cache_flush !SP2
nop; nop; nop; cache_flush !SP13
nop; nop; nop; cache_flush !SP14
nop; nop; nop; cache_flush !SP15

// CHECK: main:
// CHECK-NEXT: nop;    nop;    nop;    cache_flush
// CHECK-NEXT: nop;    nop;    nop;    cache_flush
// CHECK-NEXT: nop;    nop;    nop;    cache_flush SP1
// CHECK-NEXT: nop;    nop;    nop;    cache_flush SP2
// CHECK-NEXT: nop;    nop;    nop;    cache_flush SP13
// CHECK-NEXT: nop;    nop;    nop;    cache_flush SP14
// CHECK-NEXT: nop;    nop;    nop;    cache_flush SP15
// CHECK-NEXT: nop;    nop;    nop;    cache_flush !SP0
// CHECK-NEXT: nop;    nop;    nop;    cache_flush !SP1
// CHECK-NEXT: nop;    nop;    nop;    cache_flush !SP2
// CHECK-NEXT: nop;    nop;    nop;    cache_flush !SP13
// CHECK-NEXT: nop;    nop;    nop;    cache_flush !SP14
// CHECK-NEXT: nop;    nop;    nop;    cache_flush !SP15


nop; nop; nop; cache_invalidate
nop; nop; nop; cache_invalidate SP0
nop; nop; nop; cache_invalidate SP1
nop; nop; nop; cache_invalidate SP2
nop; nop; nop; cache_invalidate SP13
nop; nop; nop; cache_invalidate SP14
nop; nop; nop; cache_invalidate SP15
nop; nop; nop; cache_invalidate !SP0
nop; nop; nop; cache_invalidate !SP1
nop; nop; nop; cache_invalidate !SP2
nop; nop; nop; cache_invalidate !SP13
nop; nop; nop; cache_invalidate !SP14
nop; nop; nop; cache_invalidate !SP15

// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate
// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate
// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate SP1
// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate SP2
// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate SP13
// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate SP14
// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate SP15
// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate !SP0
// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate !SP1
// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate !SP2
// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate !SP13
// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate !SP14
// CHECK-NEXT: nop;    nop;    nop;    cache_invalidate !SP15
