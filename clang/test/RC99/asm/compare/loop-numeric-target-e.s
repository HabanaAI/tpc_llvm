// RUN: not %tpc_clang -march=gaudi2 -c %s -o /dev/null 2>&1 | FileCheck %s
// RUN: not %tpc_clang -march=gaudi3 -c %s -o /dev/null 2>&1 | FileCheck %s
// RUN: not %tpc_clang -march=gaudi -c %s -o /dev/null 2>&1 | FileCheck %s

// A loop end given as an address (as llvm-objdump prints it) is a missing
// label, as with Habana's assembler. The instruction compressor used to
// assert on it first.

	loop 0, S2, 4, <, 64
	nop
	nop
	nop
	halt
// CHECK-NOT: Assertion
// CHECK: [[@LINE-6]]:20: error: missing label
