// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck %s

void main(int dest, int src, _Bool src2, _Bool pred) {
  volatile int __local *dest_ptr = (int __local *)dest;
  
  // s_i1_mov_b
  {
    _Bool res = src & 0x01;
    _Bool x = src2 > src;
// CHECK-DAG: mov          [[DEST:%SP[0-9]+]], %S1
// CHECK-DAG: cmp_grt.i32  [[X:%SP[0-9]+]], %S2, %S1
// CHECK-DAG: mov          [[PRED:%SP[0-9]+]], %S3
    
    res = s_i1_mov(x, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov [[DEST]], [[X]], [[PRED]]
    
    res = s_i1_mov(x, 0, res, pred, 1);
    *dest_ptr++ = res;
// CHECK: mov [[DEST]], [[X]], ![[PRED]]
    
    res = s_i1_mov(0, 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: mov [[DEST]], 0x0, [[PRED]]
    
    res = s_i1_mov(1, 0, res, pred, 1);
    *dest_ptr++ = res;
// CHECK: mov [[DEST]], 0x1, ![[PRED]]

    int xi = *dest_ptr++;
    res = s_i1_mov((xi & 0x01), 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: ld_l [[SRC1:%S[0-9]+]], %S{{[0-9]+}}
// CHECK: mov  [[DEST]], [[SRC1]], [[PRED]]

    short xs = *dest_ptr++;
    res = s_i1_mov((xs & 0x01), 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: ld_l [[SRC2:%S[0-9]+]], %S{{[0-9]+}}
// CHECK: mov  [[DEST]], [[SRC2]], [[PRED]]

    char xc = *dest_ptr++;
    res = s_i1_mov((xc & 0x01), 0, res, pred, 0);
    *dest_ptr++ = res;
// CHECK: ld_l [[SRC3:%S[0-9]+]], %S{{[0-9]+}}
// CHECK: mov  [[DEST]], [[SRC3]], [[PRED]]
  }
}
