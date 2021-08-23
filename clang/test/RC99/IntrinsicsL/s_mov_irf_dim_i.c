// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s

void main(int dest, _Bool pred) {
  int __local *dptr = (int __local *) dest;
  int src0 = *dptr++;
  int src1 = *dptr++;
  int src2 = *dptr++;
  int src3 = *dptr++;
  int src4 = *dptr++;
  int5 indx = { src0, src1, src2, src3, src4 };
    
  int res = *dptr++;
  res = s_i32_mov_irf_dim_i_b(indx, res, 1, pred, 0);
  *dptr++ = res;
// CHECK: mov_irf_dim 0x1 %S{{[0-9]+}}, %I{{[0-9]+}}, %SP{{[0-9]+}}

  indx[3] = *dptr++;
  res = s_i32_mov_irf_dim_i(indx, 2);
  *dptr++ = res;
// CHECK: mov_irf_dim 0x2 %S{{[0-9]+}}, %I{{[0-9]+}}
}
