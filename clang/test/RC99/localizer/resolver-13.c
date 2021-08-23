// RUN: %tpc_clang -vlm 81 -S -O1 %s -o %t
// expected-no-diagnostics

int64 gval[321];

void main(int x) {
  *(int64 __local *)x = gval[0];
}
