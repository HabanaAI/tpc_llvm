// RUN: %codegen -S -O1 -triple tpc-none-none -std=rc99 %s -o - | FileCheck %s


void main(int dest, int int5_dest,
          int i1, unsigned u1,
          short s1, unsigned short us1,
          signed char c1, unsigned char uc1) {
  int *ptr = (int __local *)dest;
  
  {
    int __local *d_ptr;
    
    d_ptr = (int __local *)ptr;
    *d_ptr = *d_ptr * i1;
// CHECK: ld_l     %S[[OP_I1:[0-9]+]], %S{{[0-9]+}}
// CHECK: mul.i32  %S[[RES_I1:[0-9]+]], %S[[OP_I1]], %S2
// CHECK: st_l     %S{{[0-9]+}}, %S[[RES_I1]]
    ++ptr;

    d_ptr = (int __local *)ptr;
    *d_ptr = *d_ptr * 123;
// CHECK: ld_l     %S[[OP_I2:[0-9]+]], %S{{[0-9]+}}
// CHECK: mul.i32  %S[[RES_I2:[0-9]+]], %S[[OP_I2:[0-9]+]], 0x7b
// CHECK: st_l     %S{{[0-9]+}}, %S[[RES_I2]]
    ++ptr;

    d_ptr = (int __local *)ptr;
    *d_ptr = 123 * *d_ptr;
// CHECK: ld_l     %S[[OP_I3:[0-9]+]], %S{{[0-9]+}}
// CHECK: mul.i32  %S[[RES_I3:[0-9]+]], %S[[OP_I3:[0-9]+]], 0x7b
// CHECK: st_l     %S{{[0-9]+}}, %S[[RES_I3]]
    ++ptr;
  }

  {
    unsigned *d_ptr;

    d_ptr = (unsigned __local *)ptr;
    *d_ptr = *d_ptr * u1;
// CHECK: ld_l     %S[[OP_U1:[0-9]+]], %S{{[0-9]+}}
// CHECK: mul.i32  %S[[RES_U1:[0-9]+]], %S[[OP_U1]], %S3
// CHECK: st_l     %S{{[0-9]+}}, %S[[RES_U1]]
    ++ptr;

    d_ptr = (unsigned __local *)ptr;
    *d_ptr = *d_ptr * 123U;
// CHECK: ld_l     %S[[OP_U2:[0-9]+]], %S{{[0-9]+}}
// CHECK: mul.i32  %S[[RES_U2:[0-9]+]], %S[[OP_U2:[0-9]+]], 0x7b
// CHECK: st_l     %S{{[0-9]+}}, %S[[RES_U2]]
    ++ptr;

    d_ptr = (unsigned __local *)ptr;
    *d_ptr = 123U * *d_ptr;
// CHECK: ld_l     %S[[OP_U3:[0-9]+]], %S{{[0-9]+}}
// CHECK: mul.i32  %S[[RES_U3:[0-9]+]], %S[[OP_U3:[0-9]+]], 0x7b
// CHECK: st_l     %S{{[0-9]+}}, %S[[RES_U3]]
    ++ptr;
  }

  {
    short *d_ptr;
    short d2;
    short res;

    d_ptr = (short __local *)ptr;
    d2 = *d_ptr;
    res = d2 * s1;
// CHECK: mul.i16  %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
    *d_ptr = res;
    ++ptr;

    d_ptr = (short __local *)ptr;
    d2 = *d_ptr;
    res = d2 * (short)123;
// CHECK: mul.i16  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x7b
    *d_ptr = res;
    ++ptr;

    d_ptr = (short __local *)ptr;
    d2 = *d_ptr;
    res = (short)123 * d2;
// CHECK: mul.i16  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x7b
    *d_ptr = res;
    ++ptr;
  }


  {
    unsigned short *d_ptr;
    unsigned short d2;
    unsigned short res;

    d_ptr = (unsigned short __local *)ptr;
    d2 = *d_ptr;
    res = d2 * us1;
  // CHECK: mul.i16  %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
    *d_ptr = res;
    ++ptr;

    d_ptr = (unsigned short __local *)ptr;
    d2 = *d_ptr;
    res = d2 * (unsigned short)123;
  // CHECK: mul.i16  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x7b
    *d_ptr = res;
    ++ptr;

    d_ptr = (unsigned short __local *)ptr;
    d2 = *d_ptr;
    res = (unsigned short)123 * d2;
  // CHECK: mul.i16  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x7b
    *d_ptr = res;
    ++ptr;
  }

  {
    signed char *d_ptr;
    signed char d2;
    signed char res;

    d_ptr = (signed char __local *)ptr;
    d2 = *d_ptr;
    res = d2 * c1;
  // CHECK: mul.i8  %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
    *d_ptr = res;
    ++ptr;

    d_ptr = (signed char __local *)ptr;
    d2 = *d_ptr;
    res = d2 * (signed char)123;
  // CHECK: mul.i8  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x7b
    *d_ptr = res;
    ++ptr;

    d_ptr = (signed char __local *)ptr;
    d2 = *d_ptr;
    res = (signed char)123 * d2;
  // CHECK: mul.i8  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x7b
    *d_ptr = res;
    ++ptr;
  }

  {
    unsigned char *d_ptr;
    unsigned char d2;
    unsigned char res;

    d_ptr = (unsigned char __local *)ptr;
    d2 = *d_ptr;
    res = d2 * uc1;
  // CHECK: mul.i8  %S{{[0-9]+}}, %S{{[0-9]+}}, %S{{[0-9]+}}
    *d_ptr = res;
    ++ptr;

    d_ptr = (unsigned char __local *)ptr;
    d2 = *d_ptr;
    res = d2 * (unsigned char)123;
  // CHECK: mul.i8  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x7b
    *d_ptr = res;
    ++ptr;

    d_ptr = (unsigned char __local *)ptr;
    d2 = *d_ptr;
    res = (unsigned char)123 * d2;
  // CHECK: mul.i8  %S{{[0-9]+}}, %S{{[0-9]+}}, 0x7b
    *d_ptr = res;
    ++ptr;
  }

  {
    int5 *d_ptr;
    int5 d2;
    int5 res;

    d_ptr = (int5 __local *)int5_dest;
    d2 = *d_ptr++;
    res = d2 * *d_ptr;
  // CHECK: mul.i32  b11111 %I{{[0-9]+}}, %I{{[0-9]+}}, %I{{[0-9]+}}
    *d_ptr = res;
    ++d_ptr;

    d2 = *d_ptr;
    res = d2 * 123;
  // CHECK: mul.i32  b11111 %I{{[0-9]+}}, 0x7b, %I{{[0-9]+}}
    *d_ptr = res;
    ++d_ptr;

    d2 = *d_ptr;
    res = 123 * d2;
  // CHECK: mul.i32  b11111 %I{{[0-9]+}}, 0x7b, %I{{[0-9]+}}
    *d_ptr = res;
    ++d_ptr;
  }
}
