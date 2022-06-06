// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o - | FileCheck %s


void main(int dest, int x, int vpredp, _Bool pred) {
  volatile int __local *dptr = (int __local *)dest;


  // s_h8_nearbyint
    volatile minihalf __local *fptr = (minihalf __local *)dptr;
    minihalf res = *fptr++;
    short sx = x;
    minihalf xf = (minihalf)(sx);

    res = s_h8_nearbyint(xf, SW_RHNE, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_152 rhne %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED:%SP[0-9]+]]

    res = s_h8_nearbyint(xf, SW_RD, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_152 rd %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED:%SP[0-9]+]]

    res = s_h8_nearbyint(xf, SW_RU, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_152 ru %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED:%SP[0-9]+]]

    res = s_h8_nearbyint(xf, SW_RZ, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_152 rz %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED:%SP[0-9]+]]

    res = s_h8_nearbyint(xf, SW_RHAZ, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_152 rhaz %S{{[0-9]+}}, %S{{[0-9]+}}, [[PRED:%SP[0-9]+]]
}
