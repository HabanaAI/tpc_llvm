// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int dest, int x, int shift, _Bool pred) {
  volatile char __local *dest_ptr = (char __local *)dest;
  char income = *dest_ptr;

// CHECK-DAG: mov{{.*}} [[PRED:%SP[0-9]+]], %S3
// CHECK-DAG: ld_l  [[DEST:%S[0-9]+]], %S0

  // s_convert_i4_to_i8
  char res = income;
  res = s_convert_i4_to_i8(x, 0, res, 1, 0);
  *dest_ptr++ = res;
// CHECK: convert.i4 target_type=int8 rhne [[DEST]], %S1

  res = s_convert_i4_to_i8(x, 0, res, pred, 0);
  *dest_ptr++ = res;
// CHECK: convert.i4 target_type=int8 rhne [[DEST]], %S1, [[PRED]]

}

