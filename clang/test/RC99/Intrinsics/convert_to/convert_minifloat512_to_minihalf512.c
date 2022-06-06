// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// FIXME: (GAUDI-1733) R*U*N: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// FIXME: (GAUDI-1733) R*U*N: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s

void main(int src, int dest) {
  minifloat512 *sptr = (minifloat512 *)src;
  minihalf512 *dptr = (minihalf512 *)dest;
  minifloat512 src_val = *sptr;
  *dptr++ = convert_minifloat512_to_minihalf512(src_val, 0);
  *dptr = convert_minifloat512_to_minihalf512(src_val, SW_RU);
}

// CHECK-IR: call <512 x f8_152> @llvm.tpc.convert.v512f8_152.v512f8_143.i1(<512 x f8_143> {{.*}}, i8 13, i32 3072, <512 x f8_152> undef, i1 true, i1 false)
// CHECK-IR: call <512 x f8_152> @llvm.tpc.convert.v512f8_152.v512f8_143.i1(<512 x f8_143> {{.*}}, i8 13, i32 134144, <512 x f8_152> undef, i1 true, i1 false)

// CHECK-ASM: ld_l_v         %V[[SRC:[0-9]+]], %S0
// CHECK-ASM: convert.f8_143 all_lanes target_type=f8_152 rhne %D{{[0-9]+}}, %D[[SRC]]
// CHECK-ASM: convert.f8_143 all_lanes target_type=f8_152 ru %D{{[0-9]+}}, %D[[SRC]]
