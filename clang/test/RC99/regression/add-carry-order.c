// RUN: %tpc_clang -S -O2 -march=gaudi2 -DVECTOR %s -o - | FileCheck %s --check-prefix=VECTOR
// RUN: %tpc_clang -S -O2 -march=gaudi -DVECTOR %s -o - | FileCheck %s --check-prefix=VECTOR
// RUN: %tpc_clang -S -O0 -march=gaudi2 -DVECTOR %s -o - | FileCheck %s --check-prefix=VECTOR
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DSCALAR %s -o - | FileCheck %s --check-prefix=SCALAR
// RUN: %tpc_clang -S -O2 -march=gaudi -DSCALAR %s -o - | FileCheck %s --check-prefix=SCALAR
// RUN: %tpc_clang -S -O2 -march=gaudi2 -DLOOP %s -o - 2>&1 | FileCheck %s --check-prefix=LOOP

// A 64-bit add is a low add, then a high add with SW_CARRY that reads the
// carry the low add wrote; every integer add writes it. The schedulers must
// keep each high add after its own low add, with no integer add of the same
// unit between them (TPCCarryDeps). The post-RA scheduler used to issue the
// second vector high add before its low add (a wrong high word on gaudi2),
// and the scalar ones likewise (on gaudi and gaudi2).

#if defined(VECTOR)
void main(tensor in, tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  c[1] = 0; int64 a1lo = v_i32_ld_tnsr_b(c, in);
  c[1] = 1; int64 a1hi = v_i32_ld_tnsr_b(c, in);
  c[1] = 2; int64 b1lo = v_i32_ld_tnsr_b(c, in);
  c[1] = 3; int64 b1hi = v_i32_ld_tnsr_b(c, in);
  c[1] = 4; int64 a2lo = v_i32_ld_tnsr_b(c, in);
  c[1] = 5; int64 a2hi = v_i32_ld_tnsr_b(c, in);
  c[1] = 6; int64 b2lo = v_i32_ld_tnsr_b(c, in);
  c[1] = 7; int64 b2hi = v_i32_ld_tnsr_b(c, in);
  int64 s1lo = v_i32_add_b(a1lo, b1lo, 0);
  int64 s1hi = v_i32_add_b(a1hi, b1hi, SW_CARRY);
  int64 s2lo = v_i32_add_b(a2lo, b2lo, 0);
  int64 s2hi = v_i32_add_b(a2hi, b2hi, SW_CARRY);
  c[1] = 0; v_i32_st_tnsr(c, out, s1lo);
  c[1] = 1; v_i32_st_tnsr(c, out, s1hi);
  c[1] = 2; v_i32_st_tnsr(c, out, s2lo);
  c[1] = 3; v_i32_st_tnsr(c, out, s2hi);
}

// VECTOR-LABEL: main:
// VECTOR:     add.i32 %V
// VECTOR-NOT: add.i32 {{.*}}%V
// VECTOR:     add.i32 carry %V
// VECTOR-NOT: add.i32 {{.*}}%V
// VECTOR:     add.i32 %V
// VECTOR-NOT: add.i32 {{.*}}%V
// VECTOR:     add.i32 carry %V
// VECTOR-NOT: add.i32 {{.*}}%V
// VECTOR:     halt

#elif defined(SCALAR)
void main(tensor out, int a1lo, int a1hi, int b1lo, int b1hi, int a2lo,
          int a2hi, int b2lo, int b2hi) {
  int5 c = {0, 0, 0, 0, 0};
  int s1lo = s_i32_add(a1lo, b1lo, 0);
  int s1hi = s_i32_add(a1hi, b1hi, SW_CARRY);
  int s2lo = s_i32_add(a2lo, b2lo, 0);
  int s2hi = s_i32_add(a2hi, b2hi, SW_CARRY);
  c[1] = 0; v_i32_st_tnsr(c, out, s1lo);
  c[1] = 1; v_i32_st_tnsr(c, out, s1hi);
  c[1] = 2; v_i32_st_tnsr(c, out, s2lo);
  c[1] = 3; v_i32_st_tnsr(c, out, s2hi);
}

// SCALAR-LABEL: main:
// SCALAR:     add.i32 %S
// SCALAR-NOT: add.i32 {{.*}}%S
// SCALAR:     add.i32 carry %S
// SCALAR-NOT: add.i32 {{.*}}%S
// SCALAR:     add.i32 %S
// SCALAR-NOT: add.i32 {{.*}}%S
// SCALAR:     add.i32 carry %S
// SCALAR-NOT: add.i32 {{.*}}%S
// SCALAR:     halt

#elif defined(LOOP)
// The machine unroller places the copies of each instruction together, so
// it would put all low adds before the first high add (so does Habana's
// compiler, with wrong high words); it refuses the loop.
void main(tensor in, tensor outlo, tensor outhi) {
  const int n = get_dim_size(in, 0);
  int5 c = {0, 0, 0, 0, 0};
  #pragma clang loop machine_unroll_count(4)
  for (int x = 0; x < n; x += 64) {
    c[0] = x;
    c[1] = 0; int64 alo = v_i32_ld_tnsr_b(c, in);
    c[1] = 1; int64 ahi = v_i32_ld_tnsr_b(c, in);
    c[1] = 2; int64 blo = v_i32_ld_tnsr_b(c, in);
    c[1] = 3; int64 bhi = v_i32_ld_tnsr_b(c, in);
    int64 lo = v_i32_add_b(alo, blo, 0);
    int64 hi = v_i32_add_b(ahi, bhi, SW_CARRY);
    c[1] = 0;
    v_i32_st_tnsr(c, outlo, lo);
    v_i32_st_tnsr(c, outhi, hi);
  }
}

// LOOP:      warning: machine_unroll(4) is not applied to the loop
// LOOP-NEXT: Loop contains unsupported instructions for machine unroll
// LOOP:      loop {{.*}}, 64, <,
// LOOP:      add.i32 %V
// LOOP-NOT:  add.i32 {{.*}}%V
// LOOP:      add.i32 carry %V
// LOOP-NOT:  add.i32 {{.*}}%V
// LOOP:      halt
#endif
