// RUN: %codegen -O0 -triple tpc-none-none -std=rc99 -mllvm -tpc-nearbyint-workaround=0  -target-cpu gaudi2 -o - %s
// RUN: %codegen -O0 -triple tpc-none-none -std=rc99 -mllvm -tpc-nearbyint-workaround=0  -target-cpu doron1 -o - %s

void main(int dest, _Bool pred) {
  int64  *vptr = (int64  *)dest;
  vptr++;
  half128  *fptr = (half128  *)vptr;
 // pred=1;   pass with it
  *fptr = v_f16_nearbyint_b(*fptr, SW_RHNE, 0, pred, 0);
 
}
