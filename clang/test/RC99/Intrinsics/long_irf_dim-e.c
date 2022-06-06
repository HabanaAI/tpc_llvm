// RUN: not %tpc_clang -c -x c -march=gaudi2 %s -o - 2>&1 | FileCheck %s

void main(int dest, int value, _Bool pred) {
  int __local *dptr = (int __local *) dest;
  int5 indx = { 0, 0, 0, 0, 0 };

  int32_t_pair_t res = {0, 0};
  res = long_irf_dim(indx, 0, 0, res, pred, 0);
  // CHECK: fatal error: error in backend: long_irf_dim relevants only if -long-irf specified
  *dptr++ = res.v1;
  *dptr++ = res.v2;
}
