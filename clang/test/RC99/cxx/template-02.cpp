// RUN: %clang_cc1 -triple tpc -std=rc++ -S -O1 %s -o - | FileCheck %s

template<typename T>
constexpr T add(T x1, T x2) {
  return x1 + x2;
}

void main(int dest1, int src1, int dest2, int src2) {
  auto dptr1  = (int64 __local *)dest1;
  auto sptr1 = (int64 __local *)src1;
  *dptr1 = add(sptr1[0], sptr1[1]);
// CHECK: add.i32 %V

  ++dptr1;
  *dptr1 = add(int64(12), int64(66));
// CHECK-NOT: add.i32 %V

  auto dptr2  = (short128 __local *)dest2;
  auto sptr2 = (short128 __local *)src2;
  *dptr2 = add(sptr2[0], sptr2[1]);
// CHECK: add.i16 %V

  ++dptr2;
  *dptr2 = add(short128(12), short128(66));
// CHECK-NOT: add.i16 %V
}
