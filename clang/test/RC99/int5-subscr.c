// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s

// GAUDI-462: SW-1547: Compiler crash while setting IRF register

void main(int dest, int i) {
  int __local *dptr = (int __local *) dest;
  int5 ndx = get_index_space_offset();

  ndx[i] = 15;  // expected-error{{index of vector type must be a constant integer expression}}
  *dptr++ = ndx[1];

  ndx[-1] = 15;  // expected-error{{index of vector type is out of range}}
  *dptr++ = ndx[1];

  ndx[6] = 15;  // expected-error{{index of vector type is out of range}}
  *dptr++ = ndx[1];
}
