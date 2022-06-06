// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm --mcpu gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu goya2 %s -o %t.o
// RUN: %disasm --mcpu goya2 %t.o | FileCheck %s

NOP; DBG; DBG; NOP
NOP; NOP ; DBG; NOP
NOP; DBG; NOP ; NOP

// CHECK: nop; dbg; dbg; nop
// CHECK: nop; nop; dbg; nop
// CHECK: nop; dbg; nop; nop
