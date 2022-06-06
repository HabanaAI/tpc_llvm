#ifndef LLVM_TRANSFORMS_SCALAR_EVALSPECIALFUNCTIONPASS_H
#define LLVM_TRANSFORMS_SCALAR_EVALSPECIALFUNCTIONPASS_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <unordered_map>

using namespace llvm;

namespace {
class EvalSpecialFunctionPass : public ModulePass {
public:
  static char ID;

  EvalSpecialFunctionPass() : ModulePass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
    AU.setPreservesCFG();
  }

private:
  IntegerType *I32Type, *I1Type, *I8Type, *I16Type;
  Type *F32Type, *BF16Type;
  VectorType *Int64Type, *Int128Type, *Float64Type, *Char256Type, *Short128Type,
      *Bfloat128Type, *Float128Type, *I8256Type, *Short256Type, *Bfloat256Type;
  void replaceExpWithTPCIntrinsics(Module &M, Instruction *InstrToReplace);
  void replaceSinCosWithTPCIntrinsics(Module &M, Instruction *InstrToReplace,
                                      int SinCond);
  void replaceLogWithTPCIntrinsics(Module &M, Instruction *InstrToReplace);
  void replaceSqrtWithTPCIntrinsics(Module &M, Instruction *InstrToReplace);
  void replaceReciprocalSqrtWithTPCIntrinsics(Module &M,
                                              Instruction *InstrToReplace);
  void replaceBF16ReciprocalSqrtWithTPCIntrinsics(Module &M,
                                                  Instruction *InstrToReplace);
  void replaceTanhWithTPCIntrinsics(Module &M, Instruction *InstrToReplace);
  void replaceReciprocalWithTPCIntrinsics(Module &M,
                                          Instruction *InstrToReplace);

  void replaceBF16TanhWithTPCIntrinsics(Module &M, Instruction *InstrToReplace);

  void replaceBF16ReciprocalWithTPCIntrinsics(Module &M,
                                              Instruction *InstrToReplace);

  void replaceBF16SinWithTPCIntrinsics(Module &M, Instruction *InstrToReplace);
  void replaceBF16CosWithTPCIntrinsics(Module &M, Instruction *InstrToReplace);

  void replaceBF16LogWithTPCIntrinsics(Module &M, Instruction *InstrToReplace);
  void replaceBF16SqrtWithTPCIntrinsics(Module &M, Instruction *InstrToReplace);
  void replaceBF16ExpWithTPCIntrinsics(Module &M, Instruction *InstrToReplace);
  Constant *getBfloatValue(double V);
  void expandFDiv(Module &M, Instruction *I);
  void expandSpecialCaseLLVMIR(Module &M);
  void expandSpecialFunction(Module &M);

  bool runOnModule(Module &M) override;
};
} // end anonymous namespace
#endif
