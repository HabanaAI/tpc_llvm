// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -long-irf -verify %s
// expected-no-diagnostics

void main(int x) {
  int5 ndx1, ndx2;

  ndx1[0] = 0;
  ndx1[0] = 22;
  ndx1[1] = ndx2[1];
  ndx1[1] = ndx2[1] + 1;
  ndx1[3] = ndx1[3] - ndx2[3];

  ndx1[0]++;
  ndx1[1]--;
  ++ndx1[2];
  --ndx1[3];

  ndx1[2] += 44;
  ndx1[2] -= 44;
  ndx1[2] *= 44;
  ndx1[2] += ndx2[2];
  ndx1[2] += (ndx2[2] + 4);

  _Bool res;
  res = ndx1[2] == ndx2[2];
  res = ndx1[2] != ndx2[2];
  res = ndx1[2] <= ndx2[2];
  res = ndx1[2] >= ndx2[2];
  res = ndx1[2] < ndx2[2];
  res = ndx1[2] > ndx2[2];

  res = ndx1[1] < (ndx1[1] - ndx2[1]);
// FIXME: should be accepted as well.
//  res = (ndx1[1] + ndx2[1]) < (ndx1[1] - ndx2[1]);
}
