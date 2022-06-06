// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK,GEN3 %s

void main(int dest, int mask, int x, _Bool pred) {
  volatile int5 __local *dest_ptr = (int5 __local *)dest;
  int5 src = *dest_ptr++;
  
// CHECK-DAG: mov{{.*}}	[[PRED:%SP[0-9]+]], %S3

  // i_i32_mov
  {
    int5 res;
    
    res = *dest_ptr++;
    res = i_i32_mov(src, 0b00001, 0, res, pred, 0);
    *dest_ptr++ = res[0];
// CHECK: mov b00001 %I{{[0-9+]+}}, %I{{[0-9]+}}, [[PRED]]
    
    res = *dest_ptr++;
    res = i_i32_mov(x, 0b00010, 0, res, pred, 0);
    *dest_ptr++ = res[0];
// CHECK: mov b00010 %I{{[0-9+]+}}, %S2, [[PRED]]
    
    res = *dest_ptr++;
    res = i_i32_mov(123, 0b11111, 0, res, pred, 1);
    *dest_ptr++ = res[0];
// CHECK: mov b11111 %I{{[0-9+]+}}, 0x7b, ![[PRED]]
  }

#ifdef __goya2__
  // i_i32_mov (mask as variable)
  {
    int5 res;
    
    res = *dest_ptr++;
    res = i_i32_mov(src, mask, 0, res, pred, 0);
    *dest_ptr++ = res[0];
// GEN3: mov %M{{[0-3]}} %I{{[0-9+]+}}, %I{{[0-9]+}}, [[PRED]]
    
    res = *dest_ptr++;
    res = i_i32_mov(x, mask, 0, res, pred, 0);
    *dest_ptr++ = res[0];
// GEN3: mov %M{{[0-3]}} %I{{[0-9+]+}}, %S2, [[PRED]]
    
    res = *dest_ptr++;
    res = i_i32_mov(123, mask, 0, res, pred, 1);
    *dest_ptr++ = res[0];
// GEN3: mov %M{{[0-3]}} %I{{[0-9+]+}}, 0x7b, ![[PRED]]
  }
#endif
}
