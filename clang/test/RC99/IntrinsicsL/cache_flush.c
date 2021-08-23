// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s

void main(_Bool x) {
  cache_flush();
// CHECK: cache_flush

  cache_flush_b(0, 1, 0);
// CHECK:     cache_flush

  cache_flush_b(0, x, 1);
// CHECK: cache_flush !%SP{{[0-9]+}}

  cache_flush_b(0, x, 0);
// CHECK: cache_flush %SP{{[0-9]+}}
}
