// RUN: %tpc_clang -x assembler -march=gaudi2 %s -c -o %t.o
// RUN: %llvm-objdump --triple tpc -s -j .KernelInfo %t.o | FileCheck %s

NOP;

// CHECK: Contents of section .KernelInfo:
// CHECK-NEXT: 0000 4b45524e 454c494e 464f4245 47494e20  KERNELINFOBEGIN
// CHECK-NEXT: 0010 4b65726e 656c4e61 6d653a5b 6b65726e  KernelName:[kern
// CHECK-NEXT: 0020 656c2d69 6e666f5d 20234b45 524e454c  el-info] #KERNEL
// CHECK-NEXT: 0030 494e464f 454e44                      INFOEND
