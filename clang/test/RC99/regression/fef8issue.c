// RUN: %codegen -S -O2 -triple tpc-none-none -std=rc99  -target-cpu gaudi2 %s -o -
// RUN: %codegen -S -O2 -triple tpc-none-none -std=rc99  -target-cpu doron1 %s -o -

typedef struct _minifloat256_minihalf256_pair_t minifloat256_minihalf256;
typedef struct _minihalf256_minifloat256_pair_t minihalf256_minifloat256;

typedef struct _int64_pair_t  int64_int64;

typedef struct _minifloat256_char256_pair_t minifloat256_char256;
typedef struct _minifloat256_uchar256_pair_t minifloat256_uchar256;

typedef struct _minihalf256_char256_pair_t minihalf256_char256;
typedef struct _minihalf256_uchar256_pair_t minihalf256_uchar256;

typedef struct _char256_minifloat256_pair_t char256_minifloat256;
typedef struct _char256_minihalf256_pair_t  char256_minihalf256;

typedef struct _uchar256_minifloat256_pair_t uchar256_minifloat256;
typedef struct _uchar256_minihalf256_pair_t uchar256_minihalf256;

typedef struct _minifloat256_pair_t minifloat256_minifloat256;

void main(int dest, int src1, int src2, int pred_val, int vect_pred,
          float af32, int ai32, unsigned au32, short ai16, unsigned short au16,
          char ai8, unsigned char au8) {
  int64_int64 __local *dest_ptr = (int64_int64 __local *)dest;          
  int64 __local *src_ptr = (int64 __local *)src1;          
  int64 __local *res_ptr = (int64 __local *)src2;
  bool256 __local *vpred_ptr = (bool256 __local *)vect_pred;
  minifloat af8 = (minifloat)ai8;
  _Bool spred = src1 < src2; 
  bool256 vpred = *vpred_ptr;
    volatile minihalf256_minifloat256 __local* dptr = (minihalf256_minifloat256 __local*)dest_ptr;
    volatile minihalf256_minifloat256 result = *dptr++;
}