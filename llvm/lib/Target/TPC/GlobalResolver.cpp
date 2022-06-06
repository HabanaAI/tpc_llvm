//===- GlobalResolver.cpp - makes Global Variables Locals ---- ------------===//
//
//===----------------------------------------------------------------------===//
//
// This pass transforms global variables into constant addresses.
//
//===----------------------------------------------------------------------===//
#include <iostream>
#include "TPCTargetMachine.h"
#include "TPCTools.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Pass.h"
#include "llvm/IR/Attributes.h"
#include "llvm/Target/TPCKernelInfo.h"
using namespace llvm;


static bool mustBeResolved(const GlobalVariable &V) {
  if (V.getName().startswith("llvm."))
    return false;
#ifdef LLVM_TPC_COMPILER
  else if (V.getSection().equals(".source"))
    return false;
  else if (V.getSection().equals(KernelInfoSectionName))
    return false;
  else if (V.getSection().equals(".tpc_compiler"))
    return false;
#endif
  else
    return true;
}


namespace {
struct GlobalResolver : public ModulePass {
  static char ID; // Pass identification, replacement for typeid

  GlobalResolver() : ModulePass(ID) {}

  bool runOnModule(Module &M) override {
    const int ScalarAddrSpace = 1;
    const int VectorAddrSpace = 2;

    if (skipModule(M))
      return false;

    auto &DL = M.getDataLayout();
    LLVMContext &C = M.getContext();
    bool Changed = false;

    // Find main function.
    Function *MainFunc = nullptr;
    for (Function &F : M) {
      if (!F.isDeclaration()) {
        assert(MainFunc == 0 && "More than one entry points");
        MainFunc = &F;
      }
    }
    assert(MainFunc && "No functions in module");

    // Scan globals.
    IRBuilder<> InitBuilder(&MainFunc->getEntryBlock().front());
    SmallVector<GlobalVariable *, 32> RemovedVariables;
    unsigned ScalarAddress = 0;

    unsigned VectorAddress = 0;
    ValueReplacer Replacer;
    for (GlobalVariable &GV : M.globals()) {
      if (mustBeResolved(GV)) {
        if (!GV.use_empty()) {
          PointerType *Ty = GV.getType();
          bool IsVectorAddrSpace = isTpcVectorType(Ty->getElementType());
          uint64_t Sz = DL.getTypeAllocSize(Ty->getElementType());
          uint64_t Addr;
          if (!IsVectorAddrSpace) {
            Addr = ScalarAddress;
            if (Sz > 0 && Sz < 4) {
              Sz = 4;
            } 
            ScalarAddress += Sz;
            Ty = PointerType::get(Ty->getElementType(), ScalarAddrSpace);
          } else {
            Addr = VectorAddress;
            VectorAddress += Sz;
            Ty = PointerType::get(Ty->getElementType(), VectorAddrSpace);
          }
          ConstantInt *AddrVal = ConstantInt::get(Type::getInt32Ty(C), Addr);
          auto PtrRef = ConstantExpr::getIntToPtr(AddrVal, Ty);
          // Replace reference to the global variable with constant expression,
          // that represents address of the variable in respective address
          // space.
          Replacer.replace(&GV, PtrRef);

          if (!IsVectorAddrSpace) {
            GV.setSection(".sldata");
            GV.setLinkage(GlobalValue::LinkageTypes::ExternalLinkage);
          } else {
            GV.setSection(".vldata");
            GV.setLinkage(GlobalValue::LinkageTypes::ExternalLinkage);
          }

          if (GV.hasInitializer()) {
            Constant *Init = GV.getInitializer();
            if (!isa<UndefValue>(Init))
              InitBuilder.CreateStore(Init, PtrRef, isVolatileVariable(GV));
            GV.setInitializer(nullptr);
          }
          // adding address as DIExpression
          SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;
          GV.getAllMetadata(MDs);
          for (auto Attachment : MDs) {
            MDNode * scn = Attachment.second;
            int nop = scn->getNumOperands();
            DIGlobalVariable* dgv = dyn_cast_or_null<DIGlobalVariable>(scn->getOperand(0));
            if (dgv && nop == 2) {
              DIExpression* die = cast<DIExpression>(scn->getOperand(1));
              DIExpression* novodie;
              if (GV.hasAttribute(Attribute::Builtin)) { //Globalized local, need another expr
                SmallVector<uint64_t, 3> Ops = { dwarf::DW_OP_TPC_glob_adress, Addr,
                                                 GV.getType()->getAddressSpace()};
                novodie = DIExpression::append(die, Ops);
              }
              else {
                SmallVector<uint64_t, 2> Ops = { dwarf::DW_OP_constu, Addr};
                novodie = DIExpression::append(die, Ops);
              }
              scn->replaceOperandWith(1, novodie);
            }
          }

        } else {
          RemovedVariables.push_back(&GV);
        }

        //  RemovedVariables.push_back(&GV);
      }
    }

    // Remove resolved globals.
    for (GlobalVariable *V : RemovedVariables) {
      V->eraseFromParent();
      Changed = true;
    }

    // Store sizes of local memories in the module.
    //
    // The metadata representing the memory size may already be present. It
    // happens when compiling IR file obtained from C file. If the metadata
    // exists, delete it, as GlobalResolver already calculated actual size.
    NamedMDNode *SizeMD = M.getNamedMetadata("llvm.tpc.scalar_data");
    if (SizeMD)
      M.eraseNamedMetadata(SizeMD);
    SizeMD = M.getOrInsertNamedMetadata("llvm.tpc.scalar_data");
    assert(SizeMD->getNumOperands() == 0 && "Invalid metadata format");
    Constant *Sz = ConstantInt::get(Type::getInt32Ty(C), ScalarAddress);
    MDNode *N = MDNode::get(C, ConstantAsMetadata::get(Sz));
    SizeMD->addOperand(N);

    SizeMD = M.getNamedMetadata("llvm.tpc.vector_data");
    if (SizeMD)
      M.eraseNamedMetadata(SizeMD);
    SizeMD = M.getOrInsertNamedMetadata("llvm.tpc.vector_data");
    assert(SizeMD->getNumOperands() == 0 && "Invalid metadata format");
    Sz = ConstantInt::get(Type::getInt32Ty(C), VectorAddress);
    N = MDNode::get(C, ConstantAsMetadata::get(Sz));
    SizeMD->addOperand(N);

    return Changed;
  }

  StringRef getPassName() const override {
    return "Global Variable Resolver";
  }
};
}

char GlobalResolver::ID = 0;
INITIALIZE_PASS(GlobalResolver, "glbresolver",
                "Global Variables Resolver", false, false)

  ModulePass *llvm::createGlobalResolver() {
  return new GlobalResolver();
}
