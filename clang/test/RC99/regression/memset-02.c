// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -O2 %s -o -

// GAUDI-742 / SW-3949

void main(int dest, int sz) {
  int64 __local *dptr = (int64 __local *) dest;
  for (int i = 0; i < sz; ++i)
    dptr[i] = 0;
}
