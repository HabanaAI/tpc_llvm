// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOP; NOP; CACHE_FLUSH
NOP; NOP; NOP; CACHE_FLUSH SP0
NOP; NOP; NOP; CACHE_FLUSH SP1
NOP; NOP; NOP; CACHE_FLUSH !SP1

// CHECK: nop;    nop;    nop;    cache_flush SP0
// CHECK: nop;    nop;    nop;    cache_flush SP0
// CHECK: nop;    nop;    nop;    cache_flush SP1
// CHECK: nop;    nop;    nop;    cache_flush !SP1