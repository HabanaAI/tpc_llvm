// RUN: %tpc_clang -march=gaudi2 -S -emit-llvm %s -o %t.ll
// RUN: %tpc_clang -march=gaudi2 -S %t.ll

void main(int dest, int src) {
  int64 __local *dptr = (int64 __local *)dest;
  int64 __local *sptr = (int64 __local *)src;
  *dptr = *sptr;
}
