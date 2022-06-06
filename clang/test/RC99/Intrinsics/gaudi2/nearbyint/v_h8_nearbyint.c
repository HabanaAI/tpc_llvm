// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -float8 -target-cpu gaudi2 %s -o - | FileCheck %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -float8 -target-cpu doron1 %s -o - | FileCheck %s

void main(int dest, int xx, int vpredp, _Bool pred) {
  volatile int64 __local *vptr = (int64 __local *)dest;
  volatile bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
  bool256 vpred_val = *vpred_ptr;
  *(bool256 __local *)vptr++ = vpred_val;
    volatile minihalf256 __local *fptr = (minihalf256 __local *)vptr;
    minihalf256 res = *fptr++;
    minihalf256 x = *fptr++;
    
    res = v_h8_nearbyint_b(x, SW_RHNE, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_152 rhne %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED:%SP[0-9]+]]
    
    res = v_h8_nearbyint_b(x, SW_RD, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_152 rd %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED:%SP[0-9]+]]
    
    res = v_h8_nearbyint_b(x, SW_RU, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_152 ru %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED:%SP[0-9]+]]
    
    res = v_h8_nearbyint_b(x, SW_RZ, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_152 rz %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED:%SP[0-9]+]]

    res = v_h8_nearbyint_b(x, SW_RHAZ, res, pred, 0);
    *fptr++ = res;
    // CHECK-DAG: nearbyint.f8_152 rhaz %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED:%SP[0-9]+]]

    vptr = (int64 __local *)fptr;
  {
    bool256  vpred = (vpred_val);
    volatile minihalf256 __local *fptr = (minihalf256 __local *)vptr;
    minihalf256 res = *fptr++;
    minihalf256 x = *fptr++;
    
    res = v_h8_nearbyint_vb(x, SW_RHNE, res, vpred, 0);
    *fptr++ = res;
   // CHECK-DAG: nearbyint.f8_152 rhne %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED:%VP[0-9]+]]
    
    res = v_h8_nearbyint_vb(x, SW_RD, res, vpred, 0);
    *fptr++ = res;
   // CHECK-DAG: nearbyint.f8_152 rd %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED:%VP[0-9]+]]
    
    res = v_h8_nearbyint_vb(x, SW_RU, res, vpred, 0);
    *fptr++ = res;
   // CHECK-DAG: nearbyint.f8_152 ru %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED:%VP[0-9]+]]
    
    res = v_h8_nearbyint_vb(x, SW_RZ, res, vpred, 0);
    *fptr++ = res;
   // CHECK-DAG: nearbyint.f8_152 rz %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED:%VP[0-9]+]]

    res = v_h8_nearbyint_vb(x, SW_RHAZ, res, vpred, 0);
    *fptr++ = res;
   // CHECK-DAG: nearbyint.f8_152 rhaz %V{{[0-9]+}}, %V{{[0-9]+}}, [[PRED:%VP[0-9]+]]

    vptr = (int64 __local *)fptr;
  }

  }

