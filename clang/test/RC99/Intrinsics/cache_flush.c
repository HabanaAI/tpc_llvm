// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK,GOYA2 %s

void main(_Bool x) {
  cache_flush(0, 1, 0);
// CHECK: cache_flush

  cache_flush(0, x, 0);
// CHECK: cache_flush %SP{{[0-9]+}}
}
