// RUN: %clang_cc1 -std=rc99 -triple tpc -S -emit-llvm -O2 %s -o - | FileCheck %s

void main(int dest, float value, int start, int end, int stride) {
  __local__ float* res=(__local float *)dest;

  #pragma loop_unroll(4) pipelined
  for (int x = start; x < end; x += stride) {
    *res = value;
    value += 2.0;
    ++res;
  }
}

// CHECK: br {{.*}} !llvm.loop [[LOOP:![0-9]+]]
// CHECK: [[LOOP]] = distinct !{[[LOOP]], [[COUNT:![0-9]+]], [[PIPELINED:![0-9]+]]}
// CHECK: [[COUNT]] = !{!"llvm.loop.machine.unroll.count", i32 4}
// CHECK: [[PIPELINED]] = !{!"llvm.loop.pipelined", i1 true}
