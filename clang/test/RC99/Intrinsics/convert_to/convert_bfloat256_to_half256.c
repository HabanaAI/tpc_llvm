// RUN: %clang_cc1 -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s

// A bfloat256 takes two vector registers, and convert has no form for a
// register pair: each half is converted on its own, as Habana's compiler does.

void main(int src, int dest) {
  bfloat256 *sptr = (bfloat256 *)src;
  half256 *dptr = (half256 *)dest;
  bfloat256 src_val = *sptr;
  *dptr++ = convert_bfloat256_to_half256(src_val, 0);
  *dptr = convert_bfloat256_to_half256(src_val, SW_RU);
}

// CHECK-IR: call <256 x half> @llvm.tpc.convert.v256f16.v256bf16.i1(<256 x bfloat> {{.*}}, i8 1, i32 2816, <256 x half> undef, i1 true, i1 false)
// CHECK-IR: call <256 x half> @llvm.tpc.convert.v256f16.v256bf16.i1(<256 x bfloat> {{.*}}, i8 1, i32 133888, <256 x half> undef, i1 true, i1 false)

// CHECK-ASM-DAG: ld_l_v %V[[A:[0-9]+]], %S{{[0-9]+}}
// CHECK-ASM-DAG: ld_l_v %V[[B:[0-9]+]], %S{{[0-9]+}}
// CHECK-ASM-DAG: convert.bf16 all_lanes target_type=f16 rhne %V{{[0-9]+}}, %V[[A]]
// CHECK-ASM-DAG: convert.bf16 all_lanes target_type=f16 ru %V{{[0-9]+}}, %V[[A]]
// CHECK-ASM-DAG: convert.bf16 all_lanes target_type=f16 rhne %V{{[0-9]+}}, %V[[B]]
// CHECK-ASM-DAG: convert.bf16 all_lanes target_type=f16 ru %V{{[0-9]+}}, %V[[B]]
