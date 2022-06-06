// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-IR %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=CHECK-ASM %s

void main(int src, int dest) {
  minihalf256 *sptr = (minihalf256 *)src;
  minifloat256 *dptr = (minifloat256 *)dest;
  minihalf256 src_val = *sptr;
  *dptr++ = convert_minihalf256_to_minifloat256(src_val, 0);
  *dptr = convert_minihalf256_to_minifloat256(src_val, SW_RU);
}

// CHECK-IR: call <256 x f8_143> @llvm.tpc.convert.v256f8_143.v256f8_152.i1(<256 x f8_152> {{.*}}, i8 12, i32 3328, <256 x f8_143> undef, i1 true, i1 false)
// CHECK-IR: call <256 x f8_143> @llvm.tpc.convert.v256f8_143.v256f8_152.i1(<256 x f8_152> {{.*}}, i8 12, i32 134400, <256 x f8_143> undef, i1 true, i1 false)

// CHECK-ASM: ld_l_v         [[SRC:%V[0-9]+]], %S0
// CHECK-ASM: convert.f8_152 all_lanes target_type=f8_143 rhne %V{{[0-9]+}}, [[SRC]]
// CHECK-ASM: convert.f8_152 all_lanes target_type=f8_143 ru %V{{[0-9]+}}, [[SRC]]
