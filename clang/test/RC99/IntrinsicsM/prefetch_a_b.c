// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s

void main(tensor input, int pred) {
  int5 indx = {0, 0, 0, 0, 0};
  int __global *ptr;
  ptr = a_gen_addr_i(indx, input);
  prefetch_a_b(ptr, pred, 0);
  ++indx[0];    
  ptr = a_gen_addr_i(indx, input);
  prefetch_a_b(ptr, pred, 1);
  ++indx[0];    
  ptr = a_gen_addr_i(indx, input);
  prefetch_a(ptr);
}

// CHECK: prefetch %AD{{[0-9]+}}, %SP{{[0-9]+}}
// CHECK: prefetch %AD{{[0-9]+}}, !%SP{{[0-9]+}}
// CHECK: prefetch %AD{{[0-9]+}}, %SP0
