// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

void main(int dest, int src) {
  char256 __local *cptr = (char256 __local *)src;
  char256 cres = *cptr * *cptr; // expected-error{{ultiplication is not supported for this operand type}}
  char256 cres2 = *cptr;
  cres2 *= *cptr; // expected-error{{ultiplication is not supported for this operand type}}

  uchar256 __local *ucptr = (uchar256 __local *)src;
  uchar256 ucres = *ucptr * *ucptr; // expected-error{{ultiplication is not supported for this operand type}}
  uchar256 ucres2 = *ucptr;
  ucres2 *= *ucptr; // expected-error{{ultiplication is not supported for this operand type}}

  short128 __local *sptr = (short128 __local *)src;
  short128 sres = *sptr * *sptr; // expected-error{{ultiplication is not supported for this operand type}}
  short128 sres2 = *sptr;
  sres2 *= *sptr; // expected-error{{ultiplication is not supported for this operand type}}

  ushort128 __local *usptr = (ushort128 __local *)src;
  ushort128 usres = *usptr * *usptr; // expected-error{{ultiplication is not supported for this operand type}}
  ushort128 usres2 = *usptr;
  usres2 *= *usptr; // expected-error{{ultiplication is not supported for this operand type}}
}
