// RUN: %tpc_clang -march=gaudi2 -O2 -S %s -o - | FileCheck %s
// RUN: %tpc_clang -march=gaudi2 -O2 -S -mllvm -tpc-chain-unroll-count=0 %s -o - | FileCheck --check-prefix=ONCE %s
// RUN: %tpc_clang -march=gaudi2 -O2 -S -DPREFIX %s -o - | FileCheck --check-prefix=PREFIX %s

// A loop that carries a value other than an induction variable (a float sum)
// is left alone by the backend unroller, which gives every copy its own
// version of each carried value. TPCTTIImpl::getUnrollingPreferences has the
// IR unroller unroll it 4 times at run time instead, in order: the adds stay
// in the source's order and the loads overlap. A remainder loop runs the
// rest.
//
// With a store at the coordinate (-DPREFIX, a running sum), each copy sets
// the lane from the coordinate before the loop body instead of from the
// previous copy's, so the copies use different index registers; in one
// register each copy's SET_INDX waited for the previous copy's store.

void main(tensor a, tensor out) {
  const int rows = get_dim_size(a, 1);
  int5 i = 0;
  float64 sum = 0.0f;
  for (int y = 0; y < rows; y++) {
    i[1] = y;
    sum += v_f32_ld_tnsr_b(i, a);
#if defined(PREFIX)
    v_f32_st_tnsr(i, out, sum);
#endif
  }
#if !defined(PREFIX)
  i[1] = 0;
  v_f32_st_tnsr(i, out, sum);
#endif
}

// CHECK:         loop {{.*}}, 4, <, [[REST:\.LBB0_[0-9]+]]
// CHECK-COUNT-4: add.f32
// CHECK-NOT:     add.f32
// CHECK:         [[REST]]:
// CHECK:         loop
// CHECK:         add.f32
// CHECK-NOT:     add.f32
// CHECK:         halt

// ONCE:          loop {{.*}}, 1, <,
// ONCE:          add.f32
// ONCE-NOT:      add.f32
// ONCE:          halt

// PREFIX:        loop {{.*}}, 4, <, [[REST:\.LBB0_[0-9]+]]
// PREFIX:        st_tnsr 0x1, %I[[R:[0-9]+]],
// PREFIX-NOT:    st_tnsr 0x1, %I[[R]],
// PREFIX:        [[REST]]:
