// RUN: %codegen -triple -tpc-none-none -std=rc99 -target-cpu goya2 -S -O1 %s -o - | FileCheck %s

void main(int dest, half x) {
  half *ptr = (half *)dest;

  ptr[0] = x;
  // CHECK: st_l  %S0, %S1
}
