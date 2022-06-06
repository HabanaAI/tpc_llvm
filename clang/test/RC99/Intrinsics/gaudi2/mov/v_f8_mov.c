// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu gaudi2 %s -o - | FileCheck --check-prefixes=GEN3 %s
// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 -bfloat16 -target-cpu doron1 %s -o - | FileCheck --check-prefixes=GEN3 %s


void main(int dest0, int dest1, int xi, short xs, char xc) {
  _Bool pred = xi < dest1;
  // CHECK-DAG: cmp_less.i32 [[PRED:%SP[0-9]+]], %S2, %S1

  volatile int64 __local *dptr = (int64 __local *)dest0;
  volatile int64 __local *sptr = (int64 __local *)dest1;


  // v_f8_mov_b
  {
    volatile minifloat256 __local *dfptr = (minifloat256 __local *)dptr;
    volatile minifloat256 __local *sfptr = (minifloat256 __local *)sptr;
    minifloat256 res = *sfptr++;
    minifloat256 x = *sfptr++;
    minifloat xscal = (minifloat)(xs);
    
    res = v_f8_mov_b(x, 0, res, pred, 0);
    // GEN3: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, %SP{{[0-9]+}}
    *dfptr++ = res;
    
    res = v_f8_mov_b(xscal, 0, res, pred, 0);
    // GEN3: mov.f8_143  %V{{[0-9]+}}, %S{{[0-9]+}}, %SP{{[0-9]+}}
    *dfptr++ = res;
    
    res = v_f8_mov_b((minifloat)1.0, 0, res, pred, 0);
    // GEN3: mov.f8_143  %V{{[0-9]+}}, 0x38, %SP{{[0-9]+}}
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }

  // v_f8_mov_vb
  {
    volatile minifloat256 __local *dfptr = (minifloat256 __local *)dptr;
    volatile minifloat256 __local *sfptr = (minifloat256 __local *)sptr;
    bool256 vpred = v_f8_cmp_grt_b(*sfptr++, 0.0, 0, (bool256){0}, 1, 0);

    minifloat256 res = *sfptr++;
    minifloat256 x = *sfptr++;
    minifloat xscal = (minifloat)(xs);
   
    res = v_f8_mov_vb(x, 0, res, vpred, 0);
    // GEN3: mov  %V{{[0-9]+}}, %V{{[0-9]+}}, %VP{{[0-9]+}}
    *dfptr++ = res;
   
    res = v_f8_mov_vb(xscal, 0, res, vpred, 0);
    // GEN3: mov.f8_143  %V{{[0-9]+}}, %S{{[0-9]+}}, %VP{{[0-9]+}}
    *dfptr++ = res;
   
    res = v_f8_mov_vb((minifloat)1.0, 0, res, vpred, 0);
    // GEN3: mov.f8_143  %V{{[0-9]+}}, 0x38, %VP{{[0-9]+}}
    *dfptr++ = res;

    dptr = (int64 __local *)dfptr;
    sptr = (int64 __local *)sfptr;
  }
}
