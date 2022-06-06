// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=GEN3 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck --check-prefixes=GEN3 %s


void main(int dest, float xf, int xi, short xs, char xc) {
  _Bool pred = xi < dest;
  // CHECK-DAG: cmp_less.i32 [[PRED:%SP[0-9]+]], %S2, %S0

  volatile int __local *dptr = (int __local *)dest;

  // s_f8_mov
  {
    minifloat res = (minifloat)((unsigned short)*dptr++);
    
    res = s_f8_mov((minifloat)(xs), 0, res, pred, 0);
    // GEN3: mov.f8_143  [[RES:%S[0-9]+]], %S{{[0-9]+}}, %SP{{[0-9]+}}
    *dptr++ = (int)(res);
    
    res = s_f8_mov((minifloat)1.0, 0, res, pred, 0);
    // GEN3: mov.f8_143  [[RES]], 0x38, %SP{{[0-9]+}}
    *dptr++ = (int)(res);
  }
  // s_h8_mov
  {
    minihalf res = (minihalf)((unsigned short)*dptr++);
    
    res = s_h8_mov((minihalf)(xs), 0, res, pred, 0);
    // GEN3: mov.f8_152  [[RES:%S[0-9]+]], %S{{[0-9]+}}, %SP{{[0-9]+}}
    *dptr++ = (int)(res);
    
    res = s_h8_mov((minihalf)1.0, 0, res, pred, 0);
    // GEN3: mov.f8_152  [[RES]], 0x3c, %SP{{[0-9]+}}
    *dptr++ = (int)(res);
  }
}
