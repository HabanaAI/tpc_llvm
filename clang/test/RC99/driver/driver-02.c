// RUN: not %clang -std=rc99 -target tpc -max-tensors 10 -S -emit-llvm %s -o - 2>&1 | FileCheck %s

void main(
  tensor t0, tensor t1, tensor t2, tensor t3, tensor t4, tensor t5, tensor t6, tensor t7,
  tensor t8, tensor t9, tensor t10, tensor t11,  // expected-error{{too many tensors are declared}}
  int arg0, int arg1, int arg2, int arg3, int arg4,
  int arg5, int arg6, int arg7, int arg8, int arg9,
  int arg10, int arg11, int arg12, int arg13, int arg14,
  int arg15, int arg16, int arg17, int arg18, int arg19,
  int arg20, int arg21, int arg22, int arg23, int arg24,
  int arg25, int arg26, int arg27, int arg28, int arg29,
  int arg30, int arg31, int arg32, int arg33, float float34
) {
  int __local *ptr[] = { (int __local *)arg0,(int __local *)arg1 };
  *ptr[0] = arg32;
  *ptr[1] = arg33;
  int __local *ptr1 = (int __local *)arg2;
  *ptr1 = float34;
}

// CHECK: error: too many tensors are declared
