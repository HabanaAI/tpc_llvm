// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %disasm %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=gaudi %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu goya2 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=goya2 %t.o | FileCheck %s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu doron1 -filetype obj %s -o %t.o
// RUN: %disasm --mcpu=doron1 %t.o | FileCheck %s

loop S32, S33, S0, <, End0
NOP
End0:
NOP
// CHECK: loop S32, S33, S0, <, End0
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End0:
// CHECK-NEXT: nop; nop; nop; nop

loop S1, S2, S3, <, End1
NOP
End1:
NOP
// CHECK: loop S1, S2, S3, <, End1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End1:
// CHECK-NEXT: nop; nop; nop; nop

loop S1, S3, 123, >, End2
NOP
End2:
NOP

// CHECK: loop S1, S3, 123, >, End2
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End2:
// CHECK-NEXT: nop; nop; nop; nop


loop S1, 123, S3, <=, End3
NOP
End3:
NOP

// CHECK: loop S1, 123, S3, <=, End3
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End3:
// CHECK-NEXT: nop; nop; nop; nop


loop 123, S2, S3, >=, End4
NOP
End4:
NOP

// CHECK: loop 123, S2, S3, >=, End4
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End4:
// CHECK-NEXT: nop; nop; nop; nop


loop S1, 123, 456, ==, End5
NOP
End5:
NOP

// CHECK: loop S1, 123, 456, ==, End5
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End5:
// CHECK-NEXT: nop; nop; nop; nop


loop 123, S2, 456, !=, End6
NOP
End6:
NOP

// CHECK: loop 123, S2, 456, !=, End6
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End6:
// CHECK-NEXT: nop; nop; nop; nop


loop 123, 456, S3, >, End7
NOP
End7:
NOP

// CHECK: loop 123, 456, S3, >, End7
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End7:
// CHECK-NEXT: nop; nop; nop; nop

loop 123, 456, 789, >, End8
NOP
End8:
NOP

// CHECK: loop 123, 456, 789, >, End8
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End8:
// CHECK-NEXT: nop; nop; nop; nop



loop S1, S2, S3, <, End11, SP1
NOP
End11:
NOP
// CHECK: loop S1, S2, S3, <, End11, SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End11:
// CHECK-NEXT: nop; nop; nop; nop

loop S1, S3, 123, >, End12, !SP1
NOP
End12:
NOP
// CHECK: loop S1, S3, 123, >, End12, !SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End12:
// CHECK-NEXT: nop; nop; nop; nop

loop S1, 123, S3, <=, End13, SP1
NOP
End13:
NOP
// CHECK: loop S1, 123, S3, <=, End13, SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End13:
// CHECK-NEXT: nop; nop; nop; nop

loop 123, S2, S3, >=, End14, !SP1
NOP
End14:
NOP
// CHECK: loop 123, S2, S3, >=, End14, !SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End14:
// CHECK-NEXT: nop; nop; nop; nop

loop S1, 123, 456, ==, End15, SP1
NOP
End15:
NOP
// CHECK: loop S1, 123, 456, ==, End15, SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End15:
// CHECK-NEXT: nop; nop; nop; nop

loop 123, S2, 456, !=, End16, !SP1
NOP
End16:
NOP
// CHECK: loop 123, S2, 456, !=, End16, !SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End16:
// CHECK-NEXT: nop; nop; nop; nop

loop 123, 456, S3, >, End17, SP1
NOP
End17:
NOP
// CHECK: loop 123, 456, S3, >, End17, SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End17:
// CHECK-NEXT: nop; nop; nop; nop

loop 123, 456, 789, >, End18, !SP1
NOP
End18:
NOP
// CHECK: loop 123, 456, 789, >, End18, !SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End18:
// CHECK-NEXT: nop; nop; nop; nop


loop S1, S2, S3, <, End21, 0x1
NOP
End21:
NOP

// CHECK: loop S1, S2, S3, <, End21, 0x1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End21:
// CHECK-NEXT: nop; nop; nop; nop

loop S1, S3, 123, >, End22, 0x1
NOP
End22:
NOP

// CHECK: loop S1, S3, 123, >, End22, 0x1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End22:
// CHECK-NEXT: nop; nop; nop; nop


loop S1, 123, S3, <=, End23, 0x1
NOP
End23:
NOP

// CHECK: loop S1, 123, S3, <=, End23, 0x1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End23:
// CHECK-NEXT: nop; nop; nop; nop


loop 123, S2, S3, >=, End24, 0x1
NOP
End24:
NOP

// CHECK: loop 123, S2, S3, >=, End24, 0x1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End24:
// CHECK-NEXT: nop; nop; nop; nop


loop S1, 123, 456, ==, End25, 0x1
NOP
End25:
NOP

// CHECK: loop S1, 123, 456, ==, End25, 0x1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End25:
// CHECK-NEXT: nop; nop; nop; nop


loop 123, S2, 456, !=, End26, 0x1
NOP
End26:
NOP

// CHECK: loop 123, S2, 456, !=, End26, 0x1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End26:
// CHECK-NEXT: nop; nop; nop; nop


loop 123, 456, S3, >, End27, 0x1
NOP
End27:
NOP

// CHECK: loop 123, 456, S3, >, End27, 0x1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End27:
// CHECK-NEXT: nop; nop; nop; nop

loop 123, 456, 789, >, End28, 0x1
NOP
End28:
NOP

// CHECK: loop 123, 456, 789, >, End28, 0x1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End28:
// CHECK-NEXT: nop; nop; nop; nop



loop S1, S2, S3, <, End211, 0x1, SP1
NOP
End211:
NOP
// CHECK: loop S1, S2, S3, <, End211, 0x1, SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End211:
// CHECK-NEXT: nop; nop; nop; nop

loop S1, S3, 123, >, End212, 0x1, !SP1
NOP
End212:
NOP
// CHECK: loop S1, S3, 123, >, End212, 0x1, !SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End212:
// CHECK-NEXT: nop; nop; nop; nop

loop S1, 123, S3, <=, End213, 0x1, SP1
NOP
End213:
NOP
// CHECK: loop S1, 123, S3, <=, End213, 0x1, SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End213:
// CHECK-NEXT: nop; nop; nop; nop

loop 123, S2, S3, >=, End214, 0x1, !SP1
NOP
End214:
NOP
// CHECK: loop 123, S2, S3, >=, End214, 0x1, !SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End214:
// CHECK-NEXT: nop; nop; nop; nop

loop S1, 123, 456, ==, End215, 0x1, SP1
NOP
End215:
NOP
// CHECK: loop S1, 123, 456, ==, End215, 0x1, SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End215:
// CHECK-NEXT: nop; nop; nop; nop

loop 123, S2, 456, !=, End216, 0x1, !SP1
NOP
End216:
NOP
// CHECK: loop 123, S2, 456, !=, End216, 0x1, !SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End216:
// CHECK-NEXT: nop; nop; nop; nop

loop 123, 456, S3, >, End217, 0x1, SP1
NOP
End217:
NOP
// CHECK: loop 123, 456, S3, >, End217, 0x1, SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End217:
// CHECK-NEXT: nop; nop; nop; nop

loop 123, 456, 789, >, End218, 0x1, !SP1
NOP
End218:
NOP
// CHECK: loop 123, 456, 789, >, End218, 0x1, !SP1
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End218:
// CHECK-NEXT: nop; nop; nop; nop

loop 123, 456, 789, >, End219 // Some comment 
NOP
End219:
NOP
// CHECK: loop 123, 456, 789, >, End219
// CHECK-NEXT: nop; nop; nop; nop
// CHECK: End219:
// CHECK-NEXT: nop; nop; nop; nop
