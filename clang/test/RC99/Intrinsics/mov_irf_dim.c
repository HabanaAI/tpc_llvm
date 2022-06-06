// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu goya2 %s -o - | FileCheck %s

void main(int dest, _Bool pred) {
  int __local *dptr = (int __local *) dest;
  int src0 = *dptr++;
  int src1 = *dptr++;
  int src2 = *dptr++;
  int src3 = *dptr++;
  int src4 = *dptr++;
  int5 indx = { src0, src1, src2, src3, src4 };
    
  int res = *dptr++;
  res = mov_irf_dim(indx, 0, 0, res, pred, 0);
  *dptr++ = res;
// CHECK: mov_irf_dim 0x0 %S{{[0-9]+}}, %I{{[0-9]+}}, %SP{{[0-9]+}}
    
  res = mov_irf_dim(indx, 1, 0, res, pred, 0);
  *dptr++ = res;
// CHECK: mov_irf_dim 0x1 %S{{[0-9]+}}, %I{{[0-9]+}}, %SP{{[0-9]+}}
    
  res = mov_irf_dim(indx, 2, 0, res, pred, 0);
  *dptr++ = res;
// CHECK: mov_irf_dim 0x2 %S{{[0-9]+}}, %I{{[0-9]+}}, %SP{{[0-9]+}}
    
  res = mov_irf_dim(indx, 3, 0, res, pred, 0);
  *dptr++ = res;
// CHECK: mov_irf_dim 0x3 %S{{[0-9]+}}, %I{{[0-9]+}}, %SP{{[0-9]+}}
    
  res = mov_irf_dim(indx, 4, 0, res, pred, 0);
  *dptr++ = res;
// CHECK: mov_irf_dim 0x4 %S{{[0-9]+}}, %I{{[0-9]+}}, %SP{{[0-9]+}}
}
