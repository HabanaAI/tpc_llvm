// RUN: not %clang -cc1as -triple tpc-none-none -target-cpu gaudi2 -filetype obj %s -o /dev/null 2>&1 | FileCheck %s

// An instruction outside a bundle goes to the load slot. ADD is not a load
// slot instruction; the parser used to assert on its operands before saying so.
	add.i32 %S1, %S0, 0x1
// CHECK: [[@LINE-1]]:2: error: Instruction mnemonic 'add' is invalid in load slot
