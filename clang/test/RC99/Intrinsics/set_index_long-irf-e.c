// RUN: not %tpc_clang -c -x c -march=gaudi2 %s -o - 2>&1 | FileCheck %s

void main(int dest, int value, _Bool pred) {
  int5 indx = { 0, 0, 0, 0, 0 };

  indx = set_indx(value, indx, 1, SW_IRF44_HIGH, pred, 0);
  // CHECK: fatal error: error in backend: set_indx with high switch relevants only if -long-irf specified
}
