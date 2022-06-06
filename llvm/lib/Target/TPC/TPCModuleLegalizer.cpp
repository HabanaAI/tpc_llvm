//===- TPCModuleLegalizer.cpp --- Initial transformation of TPC module ----===//
//
//===----------------------------------------------------------------------===//
//
// This pass adjust state of TPC module, after IR libraries have loaded.
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

#define DEBUG_TYPE "tpclegal"

static Function *findFunction(Module &M, StringRef FName) {
  for (Function &F : M.functions()) {
    if (!F.isDeclaration() && F.getName() == FName)
      return &F;
  }
  return nullptr;
}

static void printRecursiveCall(const std::map<Function *, std::set<Function *>> &FuncUsers) {
}

namespace {
struct TPCModuleLegalizer : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  Function *Kernel = nullptr;
  std::set<Function *> UsedFunctions;

  TPCModuleLegalizer() : FunctionPass(ID) {}

  bool doInitialization(Module &M) override {
    bool Changed = false;
    // All functions except main function must get `always_inline` attribute and
    // `available_externaly` linkage.
    SmallVector<Function *, 1> NotInlined;
    for (auto &F : M.functions())
      if (!F.isDeclaration()) {
        if (F.hasFnAttribute("tpc-kernel")) {
          // Kernel function.
          if (Kernel)
            report_fatal_error("Functions '" + Kernel->getName() +
                               "' and '" + F.getName() +
                               "' are both declared as kernels", false);
          Kernel = &F;
        } else {
          Function *NoInlineAttr = nullptr;
          if (F.hasFnAttribute(Attribute::NoInline)) {
            F.removeFnAttr(Attribute::NoInline);
            NoInlineAttr = &F;
            Changed = true;
          }
          if (!F.hasFnAttribute(Attribute::AlwaysInline)) {
            F.addFnAttr(Attribute::AlwaysInline);
            NoInlineAttr = &F;
            Changed = true;
          }
          if (F.getLinkage() != GlobalValue::LinkageTypes::AvailableExternallyLinkage) {
            F.setLinkage(GlobalValue::LinkageTypes::AvailableExternallyLinkage);
            Changed = true;
          }
          if (F.hasComdat()) {
            F.setComdat(nullptr);
            Changed = true;
          }
          if (NoInlineAttr)
            NotInlined.push_back(NoInlineAttr);
        }
      }
    if (!Kernel) {
      // Function with attribute `tpc-kernel` was not found in the module.
      // Probably the module was not obtainde from C source. In this case try to
      // guess kernel function using inline attributes.
      if (NotInlined.size() == 1) {
        Kernel = NotInlined.front();
        Kernel->addFnAttr("tpc-kernel");
        if (Kernel->hasFnAttribute(Attribute::AlwaysInline))
          Kernel->removeFnAttr(Attribute::AlwaysInline);
        Kernel->setLinkage(GlobalValue::LinkageTypes::ExternalLinkage);
      } else {
        report_fatal_error("Kernel functions was not found in the module", false);
      }
    }
    LLVM_DEBUG(dbgs() << "Kernel is '" << Kernel->getName() << "'\n");

    // Determine the set of used functions. A function is used if it is
    // reachable from kernel function in a chain of calls.
    std::set<Function *> AllFunctions;
    for (auto &F : M)
      AllFunctions.insert(&F);
    LLVM_DEBUG(
      dbgs() << "All functions:\n";
      for (auto *F : AllFunctions)
        dbgs() << "  " << F->getName() << "\n";
    );

    std::vector<Function *> WorkList;
    WorkList.push_back(Kernel);
    while (!WorkList.empty()) {
      Function *CurrFn = WorkList.back();
      WorkList.pop_back();
      if (UsedFunctions.count(CurrFn) == 0)
        for (Instruction &I : instructions(CurrFn))
          if (auto *CI = dyn_cast<CallInst>(&I))
            if (Function *CalledFn = CI->getCalledFunction())
              if (AllFunctions.count(CalledFn) != 0)
                if (UsedFunctions.count(CalledFn) == 0) {
                  WorkList.push_back(CalledFn);
                }
      UsedFunctions.insert(CurrFn);
    }
    UsedFunctions.insert(Kernel);
    LLVM_DEBUG(
      dbgs() << "Used functions:\n";
      for (auto *F : UsedFunctions)
        dbgs() << "  " << F->getName() << "\n";
    );

    // Functions that are not used should be deleted. To make it correct,
    // build a map of unused intrinsic to its users.
    std::map<Function *, std::set<Function *>> FuncUsers;
    for (Function *F : AllFunctions)
      if (UsedFunctions.count(F) == 0) {
        FuncUsers[F];
        for (Instruction &I : instructions(F))
          if (auto *CI = dyn_cast<CallInst>(&I))
            if (Function *CalledFn = CI->getCalledFunction())
              if (UsedFunctions.count(CalledFn) == 0)
                FuncUsers[CalledFn].insert(F);
      }
    LLVM_DEBUG(
      dbgs() << "Unused functions:\n";
      for (auto &Item : FuncUsers) {
        dbgs() << "  " << Item.first->getName() << "\n";
        for (Function *U : Item.second)
          dbgs() << "    " << U->getName() << "\n";
      }
    );

    while (!FuncUsers.empty()) {
      // Find functions with no users.
      WorkList.clear();
      for (auto &Item : FuncUsers) {
        if (Item.second.empty())
          WorkList.push_back(Item.first);
      }
      if (WorkList.empty()) {
        // We have recursive calls.
        printRecursiveCall(FuncUsers);
        report_fatal_error("Recursive call found", false);
      }
      assert(!WorkList.empty());
      // Remove functions with no users.
      while (!WorkList.empty()) {
        Function *CurrFn = WorkList.back();
        LLVM_DEBUG(dbgs() << "Deleting function: " << CurrFn->getName() << "\n");
        WorkList.pop_back();
        for (auto &Item : FuncUsers)
          Item.second.erase(CurrFn);
        FuncUsers.erase(CurrFn);
        CurrFn->eraseFromParent();
      }
    }

    return Changed;
  }

  bool runOnFunction(Function &F) override {
    bool Changed = false;
    return Changed;
  }

  bool doFinalization(Module &M) override {
    bool Changed = false;
    return Changed;
  }

  //StringRef getPassName() const override {
  //  return "TPC Module Legalizer";
  //}
};
}

char TPCModuleLegalizer::ID = 0;
INITIALIZE_PASS(TPCModuleLegalizer, "tpclegal",
                "TPC Module Legalizer", false, false)

FunctionPass *llvm::createTPCModuleLegalizer() {
  return new TPCModuleLegalizer();
}

