// RUN: %clang_cc1 -fsyntax-only -ast-dump -std=rc99 -triple tpc %s | FileCheck %s
// RUN: %clang_cc1 -std=rc99 -S -emit-llvm -triple tpc -O2 %s -o - | FileCheck -check-prefix=LLVMIR %s

void main(int dest, float value, int start, int end, int stride) {
  __local__ float* res=(__local float *)dest;

  #pragma loop_taken
  for (int x = start; x < end; x += stride) {
    *res = value;
    value += 2.0;
    ++res;
  }
}

// CHECK: AttributedStmt
// CHECK-NEXT: LoopHintAttr {{.*}} loop_taken Taken Enable
// CHECK: ForStmt

// LLVMIR: br {{.*}} !llvm.loop [[LOOP:![0-9]+]]
// LLVMIR: [[LOOP]] = distinct !{[[LOOP]], [[TAKEN:![0-9]+]]}
// LLVMIR: [[TAKEN]] = !{!"llvm.loop.taken", i1 true}