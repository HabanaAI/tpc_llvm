// RUN: %disasm -mcpu=gaudi -no-common-header %S/*.o | %tpc_clang -c -x assembler -march=gaudi - -o %ttest.o
// RUN: %disasm -mcpu=gaudi -no-common-header -tpc-encoding-info -ignore-mov-dt %S/*.o   > %tenc1.txt
// RUN: %disasm -mcpu=gaudi -no-common-header -tpc-encoding-info -ignore-mov-dt %ttest.o > %tenc2.txt
// RUN: diff %tenc1.txt %tenc2.txt
// RUN: echo $? | FileCheck %s
// CHECK: 0
