// RUN: %clang_cc1 -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s

// A half256 takes two vector registers, and convert has no form for a
// register pair: each half is converted on its own, as Habana's compiler does.

void main(int src, int dest) {
  half256 *sptr = (half256 *)src;
  bfloat256 *dptr = (bfloat256 *)dest;
  half256 src_val = *sptr;
  *dptr++ = convert_half256_to_bfloat256(src_val, 0);
  *dptr = convert_half256_to_bfloat256(src_val, SW_RU);
}

// CHECK-IR: call <256 x bfloat> @llvm.tpc.convert.v256bf16.v256f16.i1(<256 x half> {{.*}}, i8 11, i32 256, <256 x bfloat> undef, i1 true, i1 false)
// CHECK-IR: call <256 x bfloat> @llvm.tpc.convert.v256bf16.v256f16.i1(<256 x half> {{.*}}, i8 11, i32 131328, <256 x bfloat> undef, i1 true, i1 false)

// CHECK-ASM-DAG: ld_l_v %V[[A:[0-9]+]], %S{{[0-9]+}}
// CHECK-ASM-DAG: ld_l_v %V[[B:[0-9]+]], %S{{[0-9]+}}
// CHECK-ASM-DAG: convert.f16 all_lanes target_type=bf16 rhne %V{{[0-9]+}}, %V[[A]]
// CHECK-ASM-DAG: convert.f16 all_lanes target_type=bf16 ru %V{{[0-9]+}}, %V[[A]]
// CHECK-ASM-DAG: convert.f16 all_lanes target_type=bf16 rhne %V{{[0-9]+}}, %V[[B]]
// CHECK-ASM-DAG: convert.f16 all_lanes target_type=bf16 ru %V{{[0-9]+}}, %V[[B]]
