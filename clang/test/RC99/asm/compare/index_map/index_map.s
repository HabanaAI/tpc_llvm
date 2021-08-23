// RUN: %clang -cc1as -triple tpc-none-none -filetype obj %s -o %t.o
// RUN: %llvm-objdump --triple tpc -d -j .IndexMap %t.o | FileCheck %s

.section .IndexMap
SCEVBEGIN IndexSpace:[0].[Input].{ 64, 4, 1, 1, 0 }[1].[Input].{ 1, 4, 1, 1, 0 }[2].[Input].{ 64, 4, 1, 1, 0 }[3].[Input].{ 1, 4, 1, 1, 0 }[4].[Output].{ 1, 4, 1, 1, 0 }[5].[Output].{ 64, 4, 1, 1, 0 }[122].[6].{ 64, 4, 1, 1, 0 }[7].[Output].{ 1, 4, 1, 1, 0 } #SCEVEND

// CHECK: .section .IndexMap
// CHECK-NEXT: SCEVBEGIN IndexSpace:[0].[Input].{ 64, 4, 1, 1, 0 }[1].[Input].{ 1, 4, 1, 1, 0 }[2].[Input].{ 64, 4, 1, 1, 0 }[3].[Input].{ 1, 4, 1, 1, 0 }[4].[Output].{ 1, 4, 1, 1, 0 }[5].[Output].{ 64, 4, 1, 1, 0 }[122].[6].{ 64, 4, 1, 1, 0 }[7].[Output].{ 1, 4, 1, 1, 0 } #SCEVEND

NOP;
