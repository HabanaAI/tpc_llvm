// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 -long-irf %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 -long-irf %s -o - | FileCheck %s

void main(int dest, int value, _Bool pred) {
  int __local *dptr = (int __local *) dest;
  int5 indx = { 0, 0, 0, 0, 0 };

  indx = set_indx(value, indx, 1, SW_IRF44_HIGH, pred, 0);
  // CHECK: set_indx high %I{{[0-9]+}}, b00001, %S{{[0-9]+}}, %SP{{[0-9]+}}

  int32_t_pair_t res = {0, 0};
  res = long_irf_dim(indx, 0, 0, res, pred, 0);
  // CHECK: mov_irf_dim both 0x0 %Z{{[0-9]+}}, %I{{[0-9]+}}, %SP{{[0-9]+}}
  *dptr++ = res.v1;
  *dptr++ = res.v2;
}
