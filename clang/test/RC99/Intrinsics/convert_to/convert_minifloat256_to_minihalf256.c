// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s

void main(int src, int dest) {
  minifloat256 *sptr = (minifloat256 *)src;
  minihalf256 *dptr = (minihalf256 *)dest;
  minifloat256 src_val = *sptr;
  *dptr++ = convert_minifloat256_to_minihalf256(src_val, 0);
  *dptr = convert_minifloat256_to_minihalf256(src_val, SW_RU);
}

// CHECK-IR: call <256 x f8_152> @llvm.tpc.convert.v256f8_152.v256f8_143.i1(<256 x f8_143> {{.*}}, i8 13, i32 3072, <256 x f8_152> undef, i1 true, i1 false)
// CHECK-IR: call <256 x f8_152> @llvm.tpc.convert.v256f8_152.v256f8_143.i1(<256 x f8_143> {{.*}}, i8 13, i32 134144, <256 x f8_152> undef, i1 true, i1 false)

// CHECK-ASM: ld_l_v         [[SRC:%V[0-9]+]], %S0
// CHECK-ASM: convert.f8_143 all_lanes target_type=f8_152 rhne %V{{[0-9]+}}, [[SRC]]
// CHECK-ASM: convert.f8_143 all_lanes target_type=f8_152 ru %V{{[0-9]+}}, [[SRC]]
