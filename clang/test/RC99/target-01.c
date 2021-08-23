// RUN: %clang_cc1 -triple=tpc -S -emit-llvm -std=rc99 -target-cpu gaudi -bfloat16 -verify %s -o /dev/null

void main() {
  require_cpu_gaudi();
  require_cpu_goya(); // expected-error{{needs target feature goya}}
}
