// RUN: not %tpc_clang -march=gaudi2 -O0 -c %s -o /dev/null 2>&1 | FileCheck %s
// RUN: not %tpc_clang -march=gaudi2 -O2 -c %s -o /dev/null 2>&1 | FileCheck %s

// A recursive function cannot be inlined into the kernel. At -O0 it is still
// defined when GlobalResolver runs, which asserted that the module has only
// one function; now -O0 gives the error of -O2 and Habana's compiler.

int factorial(int n) {
  if (n == 1)
    return 1;
  return n * factorial(n - 1);
}

void main(tensor out) {
  int5 c = {0, 0, 0, 0, 0};
  v_i32_st_tnsr(c, out, factorial(7));
}

// CHECK-NOT: Assertion
// CHECK: error in backend: Function 'factorial' is used but not defined
