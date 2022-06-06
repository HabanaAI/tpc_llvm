// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -verify -target-cpu gaudi2 -long-irf -DLONG_IRF %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -verify -target-cpu gaudi2 %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -verify -target-cpu doron1 -long-irf -DLONG_IRF %s
// RUN: %clang_cc1 -triple tpc-none-none -std=rc99 -verify -target-cpu doron1 %s
// expected-no-diagnostics

#ifdef LONG_IRF
# ifndef __LONG_IRF__
#   error "Expected macro __LONG_IRF__ set"
# endif
#else
# ifdef __LONG_IRF__
#   error "Expected macro __LONG_IRF__ unset"
# endif
#endif

void main() {}
