// RUN: not %clang -cc1as -triple tpc-none-none %s 2>&1 | FileCheck -match-full-lines %s

NOP; MOV.I32 S1, 154564541567454564564544
// CHECK: {{.*:3:18: error: Type overflow}}

NOP; MOV.F32 S1, 1e40
// CHECK: {{.*:6:18: error: Type overflow}}
NOP; MOV.F32 S1, 1e40f
// CHECK: {{.*:8:18: error: Type overflow}}
NOP; MOV.F32 S1, 1e-40
// CHECK: {{.*:10:18: error: Type underflow}}
NOP; MOV.F32 S1, 1e-40f
// CHECK: {{.*:12:18: error: Type underflow}}

NOP; MOV.F16 S1, 1e+30h
// CHECK: {{.*:15:18: error: Type overflow}}
NOP; MOV.F16 S1, 1e-30h
// CHECK: {{.*:17:18: error: Type underflow}}

NOP; MOV.I32 S1, -458u
// CHECK: {{.*:20:23: error: the invalid suffix for a negative integer}}
