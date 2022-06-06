// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s

void main(int src, int dest) {
  half128 *sptr = (half128 *)src;
  bfloat128 *dptr = (bfloat128 *)dest;
  half128 src_val = *sptr;
  *dptr++ = convert_half128_to_bfloat128(src_val, 0);
  *dptr = convert_half128_to_bfloat128(src_val, SW_RU);
}

// CHECK-IR: call <128 x bfloat> @llvm.tpc.convert.v128bf16.v128f16.i1(<128 x half> {{.*}}, i8 11, i32 256, <128 x bfloat> undef, i1 true, i1 false)
// CHECK-IR: call <128 x bfloat> @llvm.tpc.convert.v128bf16.v128f16.i1(<128 x half> {{.*}}, i8 11, i32 131328, <128 x bfloat> undef, i1 true, i1 false)

// CHECK-ASM: ld_l_v      [[SRC:%V[0-9]+]], %S0
// CHECK-ASM: convert.f16 all_lanes target_type=bf16 rhne %V{{[0-9]+}}, [[SRC]]
// CHECK-ASM: convert.f16 all_lanes target_type=bf16 ru %V{{[0-9]+}}, [[SRC]]
