// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -float8 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -float8 -target-cpu doron1 %s -o - | FileCheck %s


void main(int dest, int x, int vpredp, _Bool pred) {
  volatile int __local *dptr = (int __local *)dest;


  // s_f8_nearbyint
    volatile minifloat __local *fptr = (minifloat __local *)dptr;
    minifloat res = *fptr++;
    short sx = x;
    minifloat xf = (minifloat)(sx);

    res = s_f8_nearbyint(xf, SW_RHNE, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_143 rhne %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}

    res = s_f8_nearbyint(xf, SW_RD, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_143 rd %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}

    res = s_f8_nearbyint(xf, SW_RU, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_143 ru %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}

    res = s_f8_nearbyint(xf, SW_RZ, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_143 rz %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}

    res = s_f8_nearbyint(xf, SW_RHAZ, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_143 rhaz %S{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
}
