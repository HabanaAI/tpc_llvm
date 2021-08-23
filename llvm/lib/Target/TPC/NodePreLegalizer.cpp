//===- NodePreLegalizer.cpp - replaces unsupported nodes ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass replaces unsupported nodes (like DIV, REM) with library function
// calls.
//
//===----------------------------------------------------------------------===//

#include "TPCTargetMachine.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include <set>
using namespace llvm;

static Function *findFunction(Module &M, StringRef FName) {
  for (Function &F : M.functions()) {
    if (!F.isDeclaration() && F.getName() == FName)
      return &F;
  }
  return nullptr;
}

static void replaceNodeWithFunctionCall(Instruction &I, Function *F) {
  IRBuilder<> Builder(&I);
  // For now only binary operators are processed.
  assert(I.getNumOperands() == 2);
  Instruction *Call = Builder.CreateCall(F, { I.getOperand(0),
                                              I.getOperand(1) });
  assert(Call->getType() == I.getType());
  I.replaceAllUsesWith(Call);
}

namespace {
struct NodePreLegalizer : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  Function *FDivFunc;
  Function *FRemFunc;
  Function *UDivFunc;
  Function *URemFunc;
  Function *SDivFunc;
  Function *SRemFunc;

  NodePreLegalizer()
    : FunctionPass(ID),
      FDivFunc(nullptr),
      FRemFunc(nullptr),
      UDivFunc(nullptr),
      URemFunc(nullptr),
      SDivFunc(nullptr),
      SRemFunc(nullptr) {}

  bool doInitialization(Module &M) override {
    // Find functions implementing replaced binary operators.
    FDivFunc = findFunction(M, "__fdiv");
    FRemFunc = findFunction(M, "__frem");
    UDivFunc = findFunction(M, "__udiv");
    URemFunc = findFunction(M, "__urem");
    SDivFunc = findFunction(M, "__sdiv");
    SRemFunc = findFunction(M, "__srem");

    if (FDivFunc)
      FDivFunc->setUnnamedAddr(GlobalValue::UnnamedAddr::Local);
    if (FRemFunc)
      FRemFunc->setUnnamedAddr(GlobalValue::UnnamedAddr::Local);
    if (UDivFunc)
      UDivFunc->setUnnamedAddr(GlobalValue::UnnamedAddr::Local);
    if (URemFunc)
      URemFunc->setUnnamedAddr(GlobalValue::UnnamedAddr::Local);
    if (SDivFunc)
      SDivFunc->setUnnamedAddr(GlobalValue::UnnamedAddr::Local);
    if (SRemFunc)
      SRemFunc->setUnnamedAddr(GlobalValue::UnnamedAddr::Local);

    return false;
  }

  bool replaceDivisions(Instruction &I) {
    switch (I.getOpcode()) {
    case Instruction::FDiv:
      if (!FDivFunc)
        report_fatal_error("__fdiv is not defined");
      replaceNodeWithFunctionCall(I, FDivFunc);
      return true;
    case Instruction::FRem:
      if (!FRemFunc)
        report_fatal_error("__frem is not defined");
      replaceNodeWithFunctionCall(I, FRemFunc);
      return true;
    case Instruction::UDiv:
      if (!UDivFunc)
        report_fatal_error("__udiv is not defined");
      replaceNodeWithFunctionCall(I, UDivFunc);
      return true;
    case Instruction::URem:
      if (!URemFunc)
        report_fatal_error("__urem is not defined");
      replaceNodeWithFunctionCall(I, URemFunc);
      return true;
    case Instruction::SDiv:
      if (!SDivFunc)
        report_fatal_error("__sdiv is not defined");
      replaceNodeWithFunctionCall(I, SDivFunc);
      return true;
    case Instruction::SRem:
      if (!SRemFunc)
        report_fatal_error("__srem is not defined");
      replaceNodeWithFunctionCall(I, SRemFunc);
      return true;
    }
    return false;
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    bool Changed = false;

    // Initialize the worklist to all of the instructions ready to process.
    std::set<Instruction*> WorkList;
    for (Instruction &I : instructions(&F))
      WorkList.insert(&I);

    while (!WorkList.empty()) {
      Instruction *I = *WorkList.begin();
      WorkList.erase(WorkList.begin());
      if (!I->use_empty()) {
        // Do not replace division operations with function calls, as we do not
        // have those functions.
//        if (replaceDivisions(*I)) {
//          Changed = true;
//        }
      }
    }

    return Changed;
  }

  bool doFinalization(Module &M) override {
    for (Function &F : M.functions()) {
      if (F.getName().startswith("__")) {
        if (!F.hasFnAttribute(Attribute::AlwaysInline))
          F.addFnAttr(Attribute::AlwaysInline);
        F.setLinkage(GlobalValue::LinkageTypes::AvailableExternallyLinkage);
      }
    }
    return false;
  }

  StringRef getPassName() const override {
    return "Unsupported Node Replacement";
  }
};
}

char NodePreLegalizer::ID = 0;
INITIALIZE_PASS(NodePreLegalizer, "nodeprelegal",
                "Unsupported Node Replacement", false, false)

FunctionPass *llvm::createNodePreLegalizer() {
  return new NodePreLegalizer();
}

