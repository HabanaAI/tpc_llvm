//===- AttributeAdjuster.cpp ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass is used solely to remove attribute OptimizeNone from the list of
// function attributes, so that instruction combining can work.
//===----------------------------------------------------------------------===//

#include "TPCTargetMachine.h"
#include "llvm/Pass.h"

using namespace llvm;


namespace {
struct AttributeAdjuster : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  AttributeAdjuster() : FunctionPass(ID) {}

  bool doInitialization(Module &M) override {
    return false;
  }

  bool runOnFunction(Function &F) override {
    if (F.hasFnAttribute(Attribute::OptimizeNone)) {
      F.removeAttribute(AttributeList::FunctionIndex, Attribute::OptimizeNone);
      return true;
    }
    return false;
  }

  bool doFinalization(Module &M) override {
    return false;
  }
};
}

char AttributeAdjuster::ID = 0;
INITIALIZE_PASS(AttributeAdjuster, "adjust-attr",
                "Adjust function attributes", false, false)

  FunctionPass *llvm::createAttributeAdjuster() {
  return new AttributeAdjuster();
}

