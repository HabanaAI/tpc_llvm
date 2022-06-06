// RUN: %codegen  -O0 -triple tpc-none-none -std=rc99 -mllvm -tpc-nearbyint-workaround=0  -target-cpu gaudi2 -o - %s
// RUN: %codegen  -O0 -triple tpc-none-none -std=rc99 -mllvm -tpc-nearbyint-workaround=0  -target-cpu doron1 -o - %s

void main(int dest, _Bool pred) {
  int  *vptr = (int  *)dest;
  vptr++;
  minifloat  *fptr = (minifloat  *)vptr;
  //pred=1;
  *fptr = s_f8_abs(*fptr, SW_RHNE, 0, pred, 0);
}
