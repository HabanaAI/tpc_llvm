// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck %s


void main(int dest, int x, int vpredp, _Bool pred) {
  volatile int5 __local *dest_ptr = (int5 __local *)dest;
  int5 src = *dest_ptr++;
  
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // i_i32_mov_s_b
  {
    int5 res;
    
    res = *dest_ptr++;
    res = i_i32_mov_s_b(x, res, 0b00001, pred, 0);
    *dest_ptr++ = res[0];
// CHECK: mov b00001 %I{{[0-9+]+}}, %S{{[0-9]+}}, [[PRED]]
    
    res = *dest_ptr++;
    res = i_i32_mov_s_b(123, res, 0b11111, pred, 1);
    *dest_ptr++ = res[0];
// CHECK: mov b11111 %I{{[0-9+]+}}, 0x7b, ![[PRED]]
  }

  // i_i32_mov_s
  {
    int5 res;
    
    res = *dest_ptr++;
    res = i_i32_mov_s(x, res, 0b00001);
    *dest_ptr++ = res[0];
// CHECK: mov b00001 %I{{[0-9+]+}}, %S{{[0-9]+}}
    
    res = *dest_ptr++;
    res = i_i32_mov_s(123, res, 0b11111);
    *dest_ptr++ = res[0];
// CHECK: mov b11111 %I{{[0-9+]+}}, 0x7b
  }

  // i_i32_mov_i_b
  {
    int5 res;
    
    res = *dest_ptr++;
    res = i_i32_mov_i_b(src, res, 0b00001, pred, 0);
    *dest_ptr++ = res[0];
// CHECK: mov b00001 %I{{[0-9+]+}}, %I{{[0-9]+}}, [[PRED]]
  }

  // i_i32_mov_i
  {
    int5 res;
    
    res = *dest_ptr++;
    res = i_i32_mov_s(src, res, 0b00001);
    *dest_ptr++ = res[0];
// CHECK: mov b00001 %I{{[0-9+]+}}, %I{{[0-9]+}}
  }
}
