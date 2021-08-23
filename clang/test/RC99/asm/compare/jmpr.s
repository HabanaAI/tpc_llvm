// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -filetype obj -target-cpu gaudi %s -o %t.o
// RUN: %disasm -mcpu gaudi %t.o | FileCheck %s




L00:
nop;jmpr L0, SP1; nop; nop

// CHECK: L00:
// CHECK: nop; jmpr L0, SP1; nop; nop

L0:
nop;jmpr L1, !SP1; nop; nop

// CHECK: L0:
// CHECK: nop; jmpr L1, !SP1; nop; nop

L1:
nop;jmpr L2, !SP0; nop; nop

// CHECK: L1:
// CHECK: nop; jmpr L2, !SP0; nop; nop

L2:
nop;jmpr L3, !SP1;nop;nop

// CHECK: L2:
// CHECK: nop; jmpr L3, !SP1; nop; nop

L3:
nop;jmpr L4, SP2;nop;nop

// CHECK: L3:
// CHECK: nop; jmpr L4, SP2; nop; nop

L4:
nop;jmpr L5, !SP3;nop;nop

// CHECK: L4:
// CHECK: nop; jmpr L5, !SP3; nop; nop

L5:
nop; jmpr S20, SP1; nop; nop
nop; jmpr S20, !SP1; nop; nop

// CHECK: L5:
// CHECK: nop; jmpr S20, SP1; nop; nop
// CHECK: nop; jmpr S20, !SP1; nop; nop

