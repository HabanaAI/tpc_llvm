// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -mllvm -tpc-nearbyint-workaround=0    -target-cpu gaudi2 %s -o - | FileCheck %s

void main(int dest, int x_par, int vpredp, _Bool pred) {
  volatile int64 __local *vptr = (int64 __local *)dest;
  volatile bool256 __local *vpred_ptr = (bool256 __local *)vpredp;
  bool256 vpred_val = *vpred_ptr;
  *(bool256 __local *)vptr++ = vpred_val;
  // v_f8_nearbyint_cnvrt_b
  {
    volatile minifloat256 __local *fptr = (minifloat256 __local *)vptr;
    minifloat256_char256_pair_t res;
    res.v1 = *fptr++;
    minifloat256 x = *fptr++;
    res = v_f8_nearbyint_cnvrt_b(x, SW_RHNE, res, pred, 0);
    // CHECK-DAG: nearbyint.f8_143 cnvrt target_type=int8 rhne %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
     res = v_f8_nearbyint_cnvrt_b(x, SW_RD, res, pred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_143 cnvrt target_type=int8 rd %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    
    res = v_f8_nearbyint_cnvrt_b(x, SW_RU, res, pred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_143 cnvrt target_type=int8 ru %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    
    res = v_f8_nearbyint_cnvrt_b(x, SW_RZ, res, pred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_143 cnvrt target_type=int8 rz %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    res = v_f8_nearbyint_cnvrt_b(x, SW_RHAZ, res, pred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_143 cnvrt target_type=int8 rhaz %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    vptr = (int64 __local *)fptr;
  }
  // v_f8_nearbyint_cnvrt_vb
  {
    bool256  vpred = (vpred_val);
    volatile minifloat256 __local *fptr = (minifloat256 __local *)vptr;
    minifloat256_char256_pair_t res;
    res.v1 = *fptr++;
    res.v2 = 0;
    minifloat256 x = *fptr++;
    
    res = v_f8_nearbyint_cnvrt_vb(x, SW_RHNE, res, vpred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_143 cnvrt target_type=int8 rhne %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    res = v_f8_nearbyint_cnvrt_vb(x, SW_RD, res, vpred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_143 cnvrt target_type=int8 rd %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    res = v_f8_nearbyint_cnvrt_vb(x, SW_RU, res, vpred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_143 cnvrt target_type=int8 ru %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    res = v_f8_nearbyint_cnvrt_vb(x, SW_RZ, res, vpred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_143 cnvrt target_type=int8 rz %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_f8_nearbyint_cnvrt_vb(x, SW_RHAZ, res, vpred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_143 cnvrt target_type=int8 rhaz %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    vptr = (int64 __local *)fptr;
  }
  // v_h8_nearbyint_cnvrt_b
  {
    volatile minihalf256 __local *fptr = (minihalf256 __local *)vptr;
    minihalf256_char256_pair_t res;
    res.v1 = *fptr++;
    res.v2 = 0;
    minihalf256 x = *fptr++;
    
    res = v_h8_nearbyint_cnvrt_b(x, SW_RHNE, res, pred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_152 cnvrt target_type=int8 rhne %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    
    res = v_h8_nearbyint_cnvrt_b(x, SW_RD, res, pred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_152 cnvrt target_type=int8 rd %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    
    res = v_h8_nearbyint_cnvrt_b(x, SW_RU, res, pred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_152 cnvrt target_type=int8 ru %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    
    res = v_h8_nearbyint_cnvrt_b(x, SW_RZ, res, pred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_152 cnvrt target_type=int8 rz %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    res = v_h8_nearbyint_cnvrt_b(x, SW_RHAZ, res, pred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_152 cnvrt target_type=int8 rhaz %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    vptr = (int64 __local *)fptr;
  }

  // v_h8_nearbyint_cnvrt_vb
  {
    bool256  vpred = (vpred_val);
    volatile minihalf256 __local *fptr = (minihalf256 __local *)vptr;
    minihalf256_char256_pair_t res;
    res.v1 = *fptr++;
    res.v2 = 0;
    minihalf256 x = *fptr++;
    
    res = v_h8_nearbyint_cnvrt_vb(x, SW_RHNE, res, vpred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_152 cnvrt target_type=int8 rhne %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    res = v_h8_nearbyint_cnvrt_vb(x, SW_RD, res, vpred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_152 cnvrt target_type=int8 rd %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    res = v_h8_nearbyint_cnvrt_vb(x, SW_RU, res, vpred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_152 cnvrt target_type=int8 ru %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
    
    res = v_h8_nearbyint_cnvrt_vb(x, SW_RZ, res, vpred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_152 cnvrt target_type=int8 rz %D{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}

    res = v_h8_nearbyint_cnvrt_b(x, SW_RHAZ, res, pred, 0);
    *fptr++ = res.v1;
    *(char256 __local *)fptr++ = res.v2;
    // CHECK-DAG: nearbyint.f8_152 cnvrt target_type=int8 rhaz %D{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}

    vptr = (int64 __local *)fptr;
  }
}
