// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s

void main(int src, int dest) {
  bfloat128 *sptr = (bfloat128 *)src;
  half128 *dptr = (half128 *)dest;
  bfloat128 src_val = *sptr;
  *dptr++ = convert_bfloat128_to_half128(src_val, 0);
  *dptr = convert_bfloat128_to_half128(src_val, SW_RD);
}

// CHECK-IR: call <128 x half> @llvm.tpc.convert.v128f16.v128bf16.i1(<128 x bfloat> {{.*}}, i8 1, i32 2816, <128 x half> undef, i1 true, i1 false)
// CHECK-IR: call <128 x half> @llvm.tpc.convert.v128f16.v128bf16.i1(<128 x bfloat> {{.*}}, i8 1, i32 199424, <128 x half> undef, i1 true, i1 false)

// CHECK-ASM: ld_l_v       [[SRC:%V[0-9]+]], %S0
// CHECK-ASM: convert.bf16 all_lanes target_type=f16 rhne %V{{[0-9]+}}, [[SRC]]
// CHECK-ASM: convert.bf16 all_lanes target_type=f16 rd %V{{[0-9]+}}, [[SRC]]
