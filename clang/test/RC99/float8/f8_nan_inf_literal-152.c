// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu gaudi2 -float8 %s -o - | FileCheck %s
// RUN: %codegen -S -triple tpc-none-none -std=rc99 -O1 -target-cpu doron1 -float8 %s -o - | FileCheck %s

// A NaN folded to f8_152 needs the 5-bit all-ones exponent, like infinity.
// It used to get 0x0f and come out as 0x3e, the finite value 1.5.
void main(int dest) {
  minihalf __local *h = (minihalf __local *)dest;
  h[0] = (minihalf)(0.0f / 0.0f);
  h[1] = (minihalf)(1.0f / 0.0f);
  h[2] = (minihalf)(-1.0f / 0.0f);
}

// CHECK-DAG: mov.f8_152 {{%S[0-9]+}}, 0x7e
// CHECK-DAG: mov.f8_152 {{%S[0-9]+}}, 0x7c
// CHECK-DAG: mov.f8_152 {{%S[0-9]+}}, 0xfc
