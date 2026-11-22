//===- TPCLocalMemoryForwarding.cpp - Keep VLM vectors in registers ------===//
//
// Rules for the vector local memory (VLM) accesses of a function, which run
// before instruction selection at -O1 and up, as in Habana's compiler:
//
// 1. An LD_L_V that always executes does not read its income, so the income
//    becomes undef and needs no register.
// 2. Within a block, an LD_L_V takes the vector that an earlier ST_L_V or
//    LD_L_V of the same address left there, when nothing that may overlap it
//    was written to VLM in between. Only constant addresses can be told
//    apart.
// 3. VLM does not outlive the kernel. When nothing in a kernel reads VLM any
//    more, its ST_L_V stores are deleted.
//
//===----------------------------------------------------------------------===//

#include "TPC.h"
#include "TPCTargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "tpc-local-memory-forwarding"

static cl::opt<bool> EnableForwarding("enable-tpc-local-memory-forwarding",
                                      cl::Hidden, cl::init(true));

STATISTIC(NumIncomes, "Incomes of unconditional VLM loads dropped");
STATISTIC(NumForwarded, "VLM loads replaced by a known vector");
STATISTIC(NumStores, "Dead VLM stores deleted");

namespace {

/// Bytes of the widest VLM access, a vector.
constexpr int64_t VectorBytes = 256;

/// Whether an access with this predicate and polarity always executes.
bool isUnconditional(const Value *Predicate, const Value *Polarity) {
  const auto *P = dyn_cast<ConstantInt>(Predicate);
  const auto *Pol = dyn_cast<ConstantInt>(Polarity);
  return P && Pol && P->getType()->isIntegerTy(1) && P->isOne() != Pol->isOne();
}

std::optional<int64_t> getConstant(const Value *V) {
  if (const auto *C = dyn_cast<ConstantInt>(V))
    return C->getSExtValue();
  return std::nullopt;
}

bool isVLMLoad(Intrinsic::ID ID) {
  return ID == Intrinsic::tpc_ld_l_v || ID == Intrinsic::tpc_ld_l_v_low ||
         ID == Intrinsic::tpc_ld_l_v_high;
}

bool isVLMStore(Intrinsic::ID ID) {
  switch (ID) {
  case Intrinsic::tpc_st_l_v:
  case Intrinsic::tpc_st_l_v_low:
  case Intrinsic::tpc_st_l_v_high:
  case Intrinsic::tpc_st_l_v_ofs:
  case Intrinsic::tpc_st_l_v_low_ofs:
  case Intrinsic::tpc_st_l_v_high_ofs:
    return true;
  default:
    return false;
  }
}

/// Whether a pointer of this address space can only reach SLM or global
/// memory.
bool isNotVLM(unsigned AddrSpace) {
  return AddrSpace == LOCAL_SCALAR || AddrSpace == GLOBAL;
}

/// TPC intrinsics that access memory but never VLM.
bool isNonVLMAccess(Intrinsic::ID ID) {
  switch (ID) {
  case Intrinsic::tpc_ld_l:
  case Intrinsic::tpc_st_l:
  case Intrinsic::tpc_ld_g:
  case Intrinsic::tpc_ld_g_inc:
  case Intrinsic::tpc_ld_g_int5:
  case Intrinsic::tpc_ld_g_int5_inc:
  case Intrinsic::tpc_ld_g_partial_inc:
  case Intrinsic::tpc_st_g:
  case Intrinsic::tpc_st_g_inc:
  case Intrinsic::tpc_st_tnsr:
  case Intrinsic::tpc_st_tnsr_direct:
  case Intrinsic::tpc_st_tnsr_partial:
  case Intrinsic::tpc_st_tnsr_partial_direct:
  case Intrinsic::tpc_st_tnsr_rmw:
  case Intrinsic::tpc_st_tnsr_rmw_direct:
  case Intrinsic::tpc_st_tnsr_partial_rmw:
  case Intrinsic::tpc_st_tnsr_partial_rmw_direct:
  case Intrinsic::tpc_st_tnsr_low:
  case Intrinsic::tpc_st_tnsr_low_direct:
  case Intrinsic::tpc_st_tnsr_low_rmw:
  case Intrinsic::tpc_st_tnsr_low_rmw_direct:
  case Intrinsic::tpc_st_tnsr_high:
  case Intrinsic::tpc_st_tnsr_high_direct:
  case Intrinsic::tpc_st_tnsr_high_rmw:
  case Intrinsic::tpc_st_tnsr_high_rmw_direct:
  case Intrinsic::tpc_st_tnsr_sqz:
  case Intrinsic::tpc_st_tnsr_sqz_rmw:
  case Intrinsic::tpc_st_tnsr_s:
  case Intrinsic::tpc_st_tnsr_s_rmw:
    return true;
  default:
    return false;
  }
}

/// Whether I may write VLM other than as a VLM store intrinsic.
bool mayWriteVLM(const Instruction &I) {
  if (!I.mayWriteToMemory())
    return false;
  if (const auto *SI = dyn_cast<StoreInst>(&I))
    return !isNotVLM(SI->getPointerAddressSpace());
  if (const auto *II = dyn_cast<IntrinsicInst>(&I))
    return !II->isAssumeLikeIntrinsic() &&
           !isNonVLMAccess(II->getIntrinsicID());
  return true;
}

/// Whether I may read VLM.
bool mayReadVLM(const Instruction &I) {
  if (!I.mayReadFromMemory())
    return false;
  if (const auto *LI = dyn_cast<LoadInst>(&I))
    return !isNotVLM(LI->getPointerAddressSpace());
  if (const auto *II = dyn_cast<IntrinsicInst>(&I)) {
    // A store intrinsic that only writes memory says so; one that reads
    // (an RMW tensor store) reads global memory.
    return !II->isAssumeLikeIntrinsic() &&
           !isNonVLMAccess(II->getIntrinsicID()) &&
           !isVLMStore(II->getIntrinsicID());
  }
  return true;
}

/// Whether VLM vectors at addresses A and B may overlap: unless both are
/// constants 256 bytes or more apart. Equal values are the same address.
bool mayOverlap(const Value *A, const Value *B) {
  const std::optional<int64_t> CA = getConstant(A), CB = getConstant(B);
  return !CA || !CB || (*CA - *CB < VectorBytes && *CB - *CA < VectorBytes);
}

/// Rules 1 and 2 in BB.
bool forwardInBlock(BasicBlock &BB) {
  bool Changed = false;
  // The vectors known to be in VLM, by address.
  SmallVector<std::pair<Value *, Value *>, 8> Known;
  const auto Forget = [&Known](const Value *Addr) {
    llvm::erase_if(Known, [Addr](const std::pair<Value *, Value *> &K) {
      return mayOverlap(K.first, Addr);
    });
  };

  for (Instruction &I : make_early_inc_range(BB)) {
    auto *II = dyn_cast<IntrinsicInst>(&I);
    const Intrinsic::ID ID = II ? II->getIntrinsicID() : Intrinsic::not_intrinsic;

    if (isVLMLoad(ID)) {
      // (address, switches, income, predicate, polarity)
      if (!isUnconditional(II->getArgOperand(3), II->getArgOperand(4)))
        continue;
      if (!isa<UndefValue>(II->getArgOperand(2))) {
        II->setArgOperand(2, UndefValue::get(II->getType()));
        ++NumIncomes;
        Changed = true;
      }
      Value *Addr = II->getArgOperand(0);
      if (ID != Intrinsic::tpc_ld_l_v || getConstant(II->getArgOperand(1)) != 0)
        continue;
      const auto It = llvm::find_if(Known, [&](const auto &K) {
        return K.first == Addr && K.second->getType() == II->getType();
      });
      if (It != Known.end()) {
        II->replaceAllUsesWith(It->second);
        II->eraseFromParent();
        ++NumForwarded;
        Changed = true;
        continue;
      }
      Known.push_back({Addr, II});
      continue;
    }

    if (isVLMStore(ID)) {
      // (address, value, switches, predicate, polarity); the _ofs forms
      // have a base and an offset.
      if (ID == Intrinsic::tpc_st_l_v_ofs ||
          ID == Intrinsic::tpc_st_l_v_low_ofs ||
          ID == Intrinsic::tpc_st_l_v_high_ofs) {
        Known.clear();
        continue;
      }
      Value *Addr = II->getArgOperand(0);
      Forget(Addr);
      if (ID == Intrinsic::tpc_st_l_v &&
          getConstant(II->getArgOperand(2)) == 0 &&
          isUnconditional(II->getArgOperand(3), II->getArgOperand(4)))
        Known.push_back({Addr, II->getArgOperand(1)});
      continue;
    }

    if (mayWriteVLM(I))
      Known.clear();
  }
  return Changed;
}

/// Rule 3 in kernel F.
bool deleteDeadStores(Function &F) {
  if (F.getFnAttribute("tpc-kernel").getValueAsString() != "true")
    return false;
  SmallVector<Instruction *, 16> Stores;
  for (Instruction &I : instructions(F)) {
    if (const auto *II = dyn_cast<IntrinsicInst>(&I);
        II && isVLMStore(II->getIntrinsicID())) {
      Stores.push_back(&I);
      continue;
    }
    if (mayReadVLM(I))
      return false;
  }
  for (Instruction *I : Stores)
    I->eraseFromParent();
  NumStores += Stores.size();
  return !Stores.empty();
}

class TPCLocalMemoryForwarding : public FunctionPass {
public:
  static char ID;
  TPCLocalMemoryForwarding() : FunctionPass(ID) {}
  StringRef getPassName() const override {
    return "TPC local memory forwarding";
  }
  bool runOnFunction(Function &F) override {
    if (!EnableForwarding || skipFunction(F))
      return false;
    bool Changed = false;
    for (BasicBlock &BB : F)
      Changed |= forwardInBlock(BB);
    Changed |= deleteDeadStores(F);
    return Changed;
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
  }
};

} // namespace

char TPCLocalMemoryForwarding::ID = 0;

INITIALIZE_PASS(TPCLocalMemoryForwarding, DEBUG_TYPE,
                "TPC local memory forwarding", false, false)

FunctionPass *llvm::createTPCLocalMemoryForwarding() {
  return new TPCLocalMemoryForwarding();
}
