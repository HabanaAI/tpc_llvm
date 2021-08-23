//===- Globalizer.cpp --- Transform allocas to globals ------- ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass transforms allocas to global variables.
//
//===----------------------------------------------------------------------===//
#include <iostream>
#include "TPCTargetMachine.h"
#include "TPCTools.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Pass.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/ADT/StringRef.h"
using namespace llvm;

#define DEBUG_TYPE "globalizer"


namespace {
struct Globalizer : public ModulePass {
  static char ID; // Pass identification, replacement for typeid
  SmallVector<Instruction *, 32> WorkList;

  Globalizer() : ModulePass(ID) {}

  /* for low debuging
  static bool check_if_debug(Module &M)
  {
    NamedMDNode *CUs = M.getNamedMetadata("llvm.dbg.cu");
    return CUs != nullptr;
  }
*/

  bool runOnModule(Module &M) override {
    WorkList.clear();

    if (skipModule(M))
      return false;

    bool Changed = false;
    ValueReplacer Replacer;
    DIBuilder DIB(M, /*AllowUnresolved*/ false);
    // Allocas live in functions.
    for (auto &F : M.functions()) {
      for (auto &BB : F) {
        for (auto &I : BB) {
          if (AllocaInst *AI = dyn_cast<AllocaInst>(&I)) {
            const PointerType *T = AI->getType();
            unsigned AS = llvm::isTpcVectorType(T->getElementType()) ? 2 : 1;
            Constant *Init = UndefValue::get(T->getElementType());
            StringRef ainame = AI->getName();
            StringRef cutname = ainame;
            std::string ss = ainame.str();
            size_t posa = ss.find(".");
            if (posa != std::string::npos) {
              ss = ss.substr(0, posa);
              cutname = StringRef(ss);
            }
            // This value will replace alloca uses.
            auto *GV = new GlobalVariable(M, T->getElementType(), false,
                 GlobalValue::PrivateLinkage, Init, ainame , nullptr,
                 GlobalValue::NotThreadLocal, AS);
            // DEBUG:
            //if (check_if_debug(M)) {
            //  for (auto &mBB : F) {
            //    bool found = false;
            //    for (auto &metai : mBB) {
            //      // Metaintring decsribing variable
            //      DbgInfoIntrinsic* ci = dyn_cast<DbgInfoIntrinsic>(&metai);
            //      if (ci) {
            //        // Variable described by metaintrin
            //        DILocalVariable *Variable = ci->getVariable(); //arg 1
            //        DIExpression *Expression = DIB.createExpression(); //must be empty, new will be added
            //        StringRef VarName = Variable->getName();
            //        StringRef argname = StringRef();
            //        Value *arg0 = ci->getArgOperand(0);
            //        if (!arg0->hasName() && isa<MetadataAsValue>(arg0)) {
            //          const MetadataAsValue *V = dyn_cast<MetadataAsValue>(arg0);
            //          if (V) {
            //            auto MD = V->getMetadata();
            //            auto v = cast<ValueAsMetadata>(MD);
            //            auto vv = v->getValue();
            //            if (vv->hasName()) {
            //              argname = vv->getName();
            //            }
            //          }
            //        }
            //        // name of var extracted form metaintrinsic
            //        if (ainame == argname || VarName == cutname) {
            //          if (isa<MDNode>(Variable)) {
            //            //extract data from Local Variable
            //            StringRef vname = Variable->getName();
            //            //auto varg = Variable->getArg();
            //            auto vscope = Variable->getRawScope();
            //            auto vfile = Variable->getRawFile();
            //            auto vline = Variable->getLine();
            //            auto vtype = Variable->getRawType();
            //            //auto vflags = Variable->getFlags();
            //            auto valign = Variable->getAlignInBits();
            //            // create DIGlobalVariableExpression
            //            DIGlobalVariableExpression *GVE = nullptr;
            //            char bflinn[40];
            //            sprintf(bflinn, "%p", (void*)Variable);
            //            StringRef LinkName = StringRef(bflinn);
            //            GVE = DIB.createGlobalVariableExpression(
            //              cast<DIScope>(vscope), vname, LinkName,
            //              cast<DIFile>(vfile), vline, cast<DIType>(vtype),
            //              true,
            //              Expression,
            //              nullptr, valign);
            //            // connect new global var GV and debug info
            //            GV->addDebugInfo(GVE);
            //            GV->addAttribute(Attribute::Builtin); // to mark for resolver
            //            found = true;
            //            break;// metacall found and used
            //          }
            //        }
            //      }
            //    }
            //    if (found) break;
            //  }
            //}
            // Replace references to alloca with reference to global variable.
            // Note, in this case we change address space, as alloca memory
            // reside in space 0, but globals are in space 1 or 2.
            Replacer.replace(AI, GV);
            Changed = true;
          }
        }
      }
    }
    return Changed;
  }
};
}

char Globalizer::ID = 0;
INITIALIZE_PASS(Globalizer, "globalizer", "Alloca Globalizer", false, false)

ModulePass *llvm::createGlobalizer() {
  return new Globalizer();
}

