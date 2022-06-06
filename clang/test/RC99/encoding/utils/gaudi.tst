// RUN: %disasm -mcpu=gaudi --tpc-for-assembler %S/*.o | %tpc_clang -c -x assembler -march=gaudi - -o %ttest.o
// RUN: %disasm -mcpu=gaudi --tpc-for-assembler -tpc-encoding-info -ignore-mov-dt %S/*.o   > %tenc1.txt
// RUN: %disasm -mcpu=gaudi --tpc-for-assembler -tpc-encoding-info -ignore-mov-dt %ttest.o > %tenc2.txt
// RUN: diff %tenc1.txt %tenc2.txt
// RUN: echo $? | FileCheck %s
// CHECK: 0
