// RUN: %tpc_clang -x c -march=dali %s -c -o %t.o
// RUN: %llvm-objdump --triple tpc -s -j .tpc_compiler %t.o | FileCheck %s

void main() {
	return;
}

// CHECK: Contents of section .tpc_compiler:
// CHECK-NEXT:  0000 222d6363 3122{{[a-f0-9]+}} {{[a-f0-9]+}} {{[a-f0-9]+}}  "-cc1"


