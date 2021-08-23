// RUN: %clang_cc1 -fsyntax-only -ast-dump -std=rc99 -triple tpc %s | FileCheck %s
// RUN: %clang_cc1 -std=rc99 -triple tpc -S -emit-llvm -O2 %s -o - | FileCheck -check-prefix=LLVMIR %s

void main(int dest, float value, int start, int end, int stride) {
  __local__ float* res=(__local float *)dest;

  #pragma loop_unroll_divide(4) taken
  for (int x = start; x < end; x += stride) {
    *res = value;
    value += 2.0;
    ++res;
  }
}

// CHECK: AttributedStmt
// CHECK-NEXT: LoopHintAttr {{.*}} loop_unroll_divide MachineUnrollCount Numeric
// CHECK-NEXT: IntegerLiteral {{.*}} 'int' 4
// CHECK-NEXT: LoopHintAttr {{.*}} loop Divide Enable
// CHECK-NEXT: <<<NULL>>>
// CHECK-NEXT: LoopHintAttr {{.*}} loop Taken Enable
// CHECK-NEXT: <<<NULL>>>
// CHECK-NEXT: ForStmt

// LLVMIR: br {{.*}} !llvm.loop [[LOOP:![0-9]+]]
// LLVMIR: [[LOOP]] = distinct !{[[LOOP]], [[COUNT:![0-9]+]], [[SKIPR:![0-9]+]], [[TAKEN:![0-9]+]]}
// LLVMIR: [[COUNT]] = !{!"llvm.loop.machine.unroll.count", i32 4}
// LLVMIR: [[SKIPR]] = !{!"llvm.loop.unroll.divide", i1 true}
// LLVMIR: [[TAKEN]] = !{!"llvm.loop.taken", i1 true}
