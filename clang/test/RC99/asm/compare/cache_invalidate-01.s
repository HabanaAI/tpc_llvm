// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s

NOP; NOP; NOP; CACHE_INVALIDATE
NOP; NOP; NOP; CACHE_INVALIDATE SP0
NOP; NOP; NOP; CACHE_INVALIDATE SP1
NOP; NOP; NOP; CACHE_INVALIDATE !SP1

// CHECK: nop;    nop;    nop;    cache_invalidate
// CHECK: nop;    nop;    nop;    cache_invalidate
// CHECK: nop;    nop;    nop;    cache_invalidate SP1
// CHECK: nop;    nop;    nop;    cache_invalidate !SP1