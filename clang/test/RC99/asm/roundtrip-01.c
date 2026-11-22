// The compiler's assembly reads back into the same code: formats 1 (a bundle
// per brace block) and 3 (a bundle per line), without and with compression.
// The assembler used to keep the kernel's first instruction uncompressed on
// every target, where the compiler (and Habana's) does so only on doron1.
//
// RUN: %tpc_clang -march=gaudi2 -O2 -no-instr-compress -c %s -o %t.o
// RUN: llvm-objcopy -O binary -j .text %t.o %t.text
// RUN: %tpc_clang -march=gaudi2 -O2 -no-instr-compress -mllvm -tpc-asm-format=1 -S %s -o %t1.s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi2 -filetype obj %t1.s -o %t1.o
// RUN: llvm-objcopy -O binary -j .text %t1.o %t1.text
// RUN: cmp %t.text %t1.text
// RUN: %tpc_clang -march=gaudi2 -O2 -no-instr-compress -mllvm -tpc-asm-format=3 -S %s -o %t3.s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi2 -filetype obj %t3.s -o %t3.o
// RUN: llvm-objcopy -O binary -j .text %t3.o %t3.text
// RUN: cmp %t.text %t3.text
// RUN: llvm-objcopy -O binary -j .tpc_metadata %t3.o %t3.meta
// RUN: %python -c "import ast, re, sys; s = open(sys.argv[1]).read(); m = re.search(r'tpc_metadata:\s*\.asciz\s+(\".*\")', s); sys.exit(ast.literal_eval('b' + m.group(1)) + b'\0' != open(sys.argv[2], 'rb').read())" %t3.s %t3.meta
//
// RUN: %tpc_clang -march=gaudi2 -O2 -c %s -o %tc.o
// RUN: llvm-objcopy -O binary -j .text %tc.o %tc.text
// RUN: %tpc_clang -march=gaudi2 -O2 -mllvm -tpc-asm-format=1 -S %s -o %t1c.s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi2 -filetype obj -instr-compress %t1c.s -o %t1c.o
// RUN: llvm-objcopy -O binary -j .text %t1c.o %t1c.text
// RUN: cmp %tc.text %t1c.text
// RUN: %tpc_clang -march=gaudi2 -O2 -mllvm -tpc-asm-format=3 -S %s -o %t3c.s
// RUN: %clang -cc1as -triple tpc-none-none -target-cpu gaudi2 -filetype obj -instr-compress %t3c.s -o %t3c.o
// RUN: llvm-objcopy -O binary -j .text %t3c.o %t3c.text
// RUN: cmp %tc.text %t3c.text
//
// RUN: FileCheck --check-prefix=FMT1 %s < %t1.s
// FMT1: READ %V{{[0-9]+}}, V_LANE_ID_32
// FMT1: .section .tpc_compiler,"a",@progbits
// FMT1: .section .tpc_metadata,"aw",@progbits
// FMT1: .asciz

void main(tensor in, tensor out, float k) {
  int5 c = 0;
  float64 acc = 0;
  for (int i = 0; i < 1024; i += 64) {
    c[0] = i;
    float64 v = v_f32_ld_tnsr_b(c, in) * k;
    acc += v;
    v_f32_st_tnsr(c, out, v);
  }
  c[0] = 0;
  v_f32_st_tnsr(c, out, v_f32_reduce_add(acc));
}
