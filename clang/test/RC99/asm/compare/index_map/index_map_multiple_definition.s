// RUN: not %clang -cc1as -triple tpc-none-none -filetype obj %s -o /dev/null 2>&1 1>/dev/null | FileCheck %s

.section .IndexMap
SCEVBEGIN IndexSpace:[0].[Input].{ 64, 4, 1, 1, 0 }[1].[Input].{ 1, 4, 1, 1, 0 }[2].[Input].{ 64, 4, 1, 1, 0 }[3].[Input].{ 1, 4, 1, 1, 0 }[4].[Output].{ 1, 4, 1, 1, 0 }[5].[Output].{ 64, 4, 1, 1, 0 }[122].[6].{ 64, 4, 1, 1, 0 }[7].[Output].{ 1, 4, 1, 1, 0 } #SCEVEND

.section .IndexMap
SCEVBEGIN IndexSpace:[0].[Input].{ 64, 4, 1, 1, 0 }[1].[Input].{ 1, 4, 1, 1, 0 }[2].[Input].{ 64, 4, 1, 1, 0 }[3].[Input].{ 1, 4, 1, 1, 0 }[4].[Output].{ 1, 4, 1, 1, 0 }[5].[Output].{ 64, 4, 1, 1, 0 }[122].[6].{ 64, 4, 1, 1, 0 }[7].[Output].{ 1, 4, 1, 1, 0 } #SCEVEND
// CHECK: [[@LINE-2]]:1: error: Multi definition .IndexMap is denied

NOP;
