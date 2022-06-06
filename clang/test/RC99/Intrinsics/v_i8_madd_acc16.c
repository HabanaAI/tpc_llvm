// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -target-cpu goya2 -bfloat16 %s -o - | FileCheck %s

void main(int x0a, int x1a, int x2a, char xs, int dest, _Bool pred, int vpreda) {
  char256 __local *x0ptr = (char256 __local *)x0a;
  char256 __local *x1ptr = (char256 __local *)x1a;
  short256 __local *x2ptr = (short256 __local *)x2a;
  short256  __local *dptr  = (short256 __local *)dest;
  bool256 __local *vpptr  = (bool256 __local *)vpreda;
  short256 res = { 0 };
  char256 x0 = *x0ptr;
  char256 x1 = *x1ptr;
  short256 x2 = *x2ptr;
  bool256 vpred = *vpptr;

  // Vector + Vector

  res = v_i8_madd_acc16_b(x0, x1, x2, 0,  res, 1, 0);
  *dptr++ = res;
  // CHECK: madd.i8 acc_i16 %D{{[0-9]+}}, %V{{[0-9]+}}, %V{{[0-9]+}}, %D{{[0-9]+}}
}
