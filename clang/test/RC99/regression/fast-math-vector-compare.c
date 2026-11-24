// RUN: %tpc_clang -march=gaudi2 -O2 -S -ffast-math -DVTYPE=float64 -DVSFX=f32 %s -o - | FileCheck %s -DS=f32
// RUN: %tpc_clang -march=gaudi2 -O2 -S -fno-honor-nans -DVTYPE=float64 -DVSFX=f32 %s -o - | FileCheck %s -DS=f32
// RUN: %tpc_clang -march=gaudi2 -O2 -S -ffast-math -DVTYPE=bfloat128 -DVSFX=bf16 %s -o - | FileCheck %s -DS=bf16
// RUN: %tpc_clang -march=gaudi2 -O2 -S -fno-honor-nans -DVTYPE=bfloat128 -DVSFX=bf16 %s -o - | FileCheck %s -DS=bf16
// RUN: %tpc_clang -march=gaudi2 -O2 -S -ffast-math -DVTYPE=half128 -DVSFX=f16 %s -o - | FileCheck %s -DS=f16
// RUN: %tpc_clang -march=gaudi2 -O2 -S -fno-honor-nans -DVTYPE=half128 -DVSFX=f16 %s -o - | FileCheck %s -DS=f16

// Without NaNs LLVM drops the ordered part of float predicates (olt becomes
// lt), and vector float compares had patterns only for the ordered ones: a
// compare-and-select stopped with "Cannot select: setcc" (f32 too; Habana's
// compiler stops there as well). With bf16 and f16, a < b ? a : b becomes
// fminnum, which asserted in LowerSELECT_CC.

#define CAT_(a, b) a##b
#define CAT(a, b) CAT_(a, b)
#define LD(p) CAT(CAT(v_, p), _ld_tnsr_b)
#define ST(p) CAT(CAT(v_, p), _st_tnsr)

void main(tensor in, tensor out) {
  int5 c = 0;
  VTYPE a = LD(VSFX)(c, in); c[1] = 1;
  VTYPE b = LD(VSFX)(c, in); c[1] = 2;
  VTYPE x = LD(VSFX)(c, in); c[1] = 3;
  VTYPE y = LD(VSFX)(c, in);
  VTYPE r = a < b ? x : y;
  VTYPE m = a < b ? a : b;
  VTYPE n = a > b ? a : b;
  c[1] = 0; ST(VSFX)(c, out, r);
  c[1] = 1; ST(VSFX)(c, out, m);
  c[1] = 2; ST(VSFX)(c, out, n);
}

// CHECK-DAG: sel_less.[[S]]
// CHECK-DAG: min.[[S]]
// CHECK-DAG: max.[[S]]
