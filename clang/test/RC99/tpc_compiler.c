// RUN: %tpc_clang -x c -march=gaudi2 %s -c -o %t.o
// RUN1: %llvm-objdump -s -j .tpc_compiler %t.o | FileCheck %s

void main() {
	return;
}

// CHECK1: Contents of section .tpc_compiler:
// CHECK-NEXT1:  0000 222d6363 3161{{[a-z0-9]+}} {{[a-z0-9]+}} {{[a-z0-9]+}}  "-cc1"


