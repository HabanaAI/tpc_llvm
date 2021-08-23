// RUN: %clang_cc1 -fsyntax-only -std=rc99 -triple tpc-none-none -verify %s
// RUN: %tpc_clang -c %s
// expected-no-diagnostics

bool256   b256;
char256   ch256;
uchar256  uch256;
int64     i64;
uint64    ui64;
float64   f64;

void main() {
}
