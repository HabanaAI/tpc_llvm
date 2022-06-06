// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck %s

void main(tensor input, int pred) {
  int5 indx = {0, 0, 0, 0, 0};
  int __global *ptr;
  ptr = gen_addr(indx, input, 0, 0, 1, 0);
  prefetch(ptr, SW_L2, pred, 0);
  ++indx[1];    
  ptr = gen_addr(indx, input, 0, 0, 1, 0);
  prefetch(ptr, SW_L2, pred, 1);
  ++indx[1];    
  ptr = gen_addr(indx, input, 0, 0, 1, 0);
  prefetch(ptr, SW_L2, 1, 0);
}

// CHECK: prefetch l2 %AD{{[0-9]+}}, %SP{{[0-9]+}}
// CHECK: prefetch l2 %AD{{[0-9]+}}, !%SP{{[0-9]+}}
// CHECK: prefetch l2 %AD{{[0-9]+}}
