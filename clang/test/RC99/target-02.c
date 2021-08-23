// RUN: %clang_cc1 -triple=tpc -S -emit-llvm -std=rc99 -target-cpu dali -verify %s -o /dev/null

void main() {
  require_cpu_goya();
  require_cpu_gaudi(); // expected-error{{needs target feature gaudi}}
}
