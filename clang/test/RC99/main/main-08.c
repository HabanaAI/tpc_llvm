// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -emit-llvm -O1 %s -o - | FileCheck %s
// XFAIL: *
// GAUDI-2311
// TODO rewrite as runtime test
void main(
  int arg0, int arg1, int arg2, int arg3, int arg4,
  int arg5, int arg6, int arg7, int arg8, int arg9, 
  int arg10, int arg11, int arg12, int arg13, int arg14,
  int arg15, int arg16, int arg17, int arg18, int arg19, 
  int arg20, int arg21, int arg22, int arg23, int arg24,
  int arg25, int arg26, int arg27, int arg28, int arg29, 
  int arg30, int arg31, int arg32, int arg33, float float34
) {  
  int __local *ptr[] = { (int __local *)arg0,(int __local *)arg1};
  *ptr[0] = arg32;
  *ptr[1] = arg33;
  int __local *ptr1 = (int __local *)arg2;
  *ptr1 = float34;
}

// CHECK: %{{[0-9]+}} = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> zeroinitializer, i8 7)
// CHECK: %{{[0-9]+}} = bitcast i8 addrspace(3)* %{{[0-9]+}} to i32 addrspace(3)*
// CHECK: %{{[0-9]+}} = load i32, i32 addrspace(3)* %{{[0-9]+}}, align 4
// CHECK: %{{[0-9]+}} = tail call <5 x i32> @llvm.tpc.i.i32.add.s.i(i32 1, <5 x i32> zeroinitializer, <5 x i32> undef, i8 1)
// CHECK: %{{[0-9]+}} = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %{{[0-9]+}}, i8 7)
// CHECK: %{{[0-9]+}} = bitcast i8 addrspace(3)* %{{[0-9]+}} to i32 addrspace(3)*
// CHECK: %{{[0-9]+}} = load i32, i32 addrspace(3)* %{{[0-9]+}}, align 4
// CHECK: %{{[0-9]+}} = tail call <5 x i32> @llvm.tpc.i.i32.add.s.i(i32 1, <5 x i32> %{{[0-9]+}}, <5 x i32> undef, i8 1)
// CHECK: %{{[0-9]+}} = tail call i8 addrspace(3)* @llvm.tpc.gen.addr(<5 x i32> %{{[0-9]+}}, i8 7)
// CHECK: %{{[0-9]+}} = bitcast i8 addrspace(3)* %{{[0-9]+}} to float addrspace(3)*
// CHECK: %{{[0-9]+}} = load float, float addrspace(3)* %{{[0-9]+}}, align 4
// CHECK: %{{[0-9]+}} = inttoptr i32 %arg0 to i32 addrspace(1)*
// CHECK: %{{[0-9]+}} = inttoptr i32 %arg1 to i32 addrspace(1)*
// CHECK: store i32 %{{[0-9]+}}, i32 addrspace(1)* %{{[0-9]+}}, align 4
// CHECK: store i32 %{{[0-9]+}}, i32 addrspace(1)* %{{[0-9]+}}, align 4
// CHECK: %{{[0-9]+}} = inttoptr i32 %arg2 to i32 addrspace(1)*
// CHECK: %conv = fptosi float %{{[0-9]+}} to i32
// CHECK: store i32 %conv, i32 addrspace(1)* %{{[0-9]+}}, align 4
// CHECK: ret void
