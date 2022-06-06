// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 %s -o - | FileCheck --check-prefix=CHECK-GAUDI %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck --check-prefix=CHECK-GAUDI %s

void main(minifloat x0, int dest, _Bool pred) {
  int __local *res_ptr = (int __local *)dest;
  char cr;
  *res_ptr++ = cr =s_f8_extract_exp(x0, 1,0,1,0);
  // CHECK-GAUDI: extract_exp.f8_143 biased %S{{[0-9]+}}, %S{{[0-9]+}}
  x0 = (minifloat)cr;
  *res_ptr++ = cr =s_f8_extract_exp(x0, 0,0,1,0);
  // CHECK-GAUDI: extract_exp.f8_143 %S{{[0-9]+}}, %S{{[0-9]+}}
  x0 = (minifloat)cr;

  char res = 0;

  *res_ptr++ = cr =s_f8_extract_exp(x0, 1, res, pred, 0);
  // CHECK-GAUDI: extract_exp.f8_143 biased %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = (minifloat)cr;
  *res_ptr++ =cr = s_f8_extract_exp(x0, 1,res,  pred, 0);
  // CHECK-GAUDI: extract_exp.f8_143 biased  %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
  x0 = (minifloat)cr;
}
