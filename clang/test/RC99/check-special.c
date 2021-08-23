// RUN: %tpc_clang -fsyntax-only %s
// expected-no-diagnostics

#ifndef TPC_SPECIAL_H_INCLUDED
#error "tpc-special.h file must be auto-included"
#endif

void main() {
}
