// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -emit-llvm -max-tensors 4 -O1 %s -o - | FileCheck %s
//
void main(
  tensor t0, tensor t1, tensor t2, 
  int arg0, int arg1, int arg2, int arg3, int arg4,
  int arg5, int arg6, int arg7, int arg8, int arg9,
  int arg10, int arg11, int arg12, int arg13, int arg14,
  int arg15, int arg16, int arg17, int arg18, int arg19,
  int arg20, int arg21, int arg22, int arg23, int arg24,
  int arg25, int arg26, int arg27, int arg28, int arg29,
  int arg30, float float34, int arg31
) {
  int5 offset;
  __global__ float64* ptr1;
  offset[0] = 0;offset[1] = 1;offset[2] = 2;offset[3] = offset[4] = 3;
  ptr1 = a_gen_addr_i(offset, t2);
  *ptr1 = float34;
  int __local *ptr = (int __local *)arg31;
  *ptr = arg30;

}
// float34 - is extra argument
// tensor 2 reserved for printf and assigned to "t2"
// tensor 3 used for extra arg
// CHECK: %{{[0-9]+}} = {{.*}}call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 3, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
// CHECK: %{{[0-9]+}} = {{.*}}call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> <i32 0, i32 1, i32 2, i32 3, i32 3>, i8 2, i32 0, i8 addrspace(3)* undef, i1 true, i1 false)
