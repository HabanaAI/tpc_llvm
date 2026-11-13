// RUN: %tpc_clang -S -O2 -march=gaudi2 %s -o - | FileCheck %s
// RUN: %tpc_clang -S -O2 -march=gaudi %s -o - | FileCheck %s

// The second loop continues the pointer of the first. Loop strength reduction
// used to rewrite its exit compare as `start + iv < end`, and TPCHardwareLoops
// then found no loop counter, so the loop kept a compare and a jump on every
// iteration. TPCTTIImpl::canSaveCmp asks LSR to leave the compare alone, and
// both loops become hardware loops, as with LLVM 12 and Habana's compiler.

void main(int start, int end, int out) {
  int __local *ptr_dest = (int __local *)out;
  for (int addr = start; addr < end; addr += 4)
    *ptr_dest++ = s_i32_ld_l(addr, 0, 0, 1, 0);
  for (int addr = start; addr < end; addr += 4)
    *ptr_dest++ = s_i32_ld_l(addr, 0, 0, 1, 0) + 1;
}

// CHECK: loop %S{{[0-9]+}}, %S{{[0-9]+}}, 4, <
// CHECK: loop %S{{[0-9]+}}, %S{{[0-9]+}}, 4, <
// CHECK-NOT: jmpr
// CHECK: halt
