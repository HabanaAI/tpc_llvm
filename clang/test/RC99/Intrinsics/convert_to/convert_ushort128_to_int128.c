// RUN: %clang_cc1 -S -emit-llvm -O1 -triple tpc-none-none -std=rc99 -target-cpu gaudi %s -o - | FileCheck --check-prefixes=CHECK-IR %s

void main(int src, int dest) {
  ushort128 *sptr = (ushort128 *)src;
  int128 *dptr = (int128 *)dest;
  ushort128 src_val = *sptr;
  *dptr = convert_ushort128_to_int128(src_val, 0);
}

// CHECK-IR: call <128 x i32> @llvm.tpc.convert.v128i32.v128i16.i1(<128 x i16> {{.*}}, i8 8, i32 512, <128 x i32> undef, i1 true, i1 false)
