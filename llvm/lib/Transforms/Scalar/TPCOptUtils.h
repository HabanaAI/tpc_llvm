//===-------------------------- TPCOptUtils.h --------------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//            2020,2021 - This pass is a property of Habana labs
//
// Author : Vinay V. Vasista
// Email  : vvasista@habana.ai
//
//===-------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_SCALAR_TPC_OPT_UTILS_H
#define LLVM_TRANSFORM_SCALAR_TPC_OPT_UTILS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

#include <sstream>
#include <unordered_set>

namespace llvm {

#define DEBUG_TYPE "tpc-opt-utils"
#define TPC_OPT_UTILS_DEBUG(x)                                                 \
  LLVM_DEBUG(dbgs() << "[TPCOptUtils] " << x << "\n");

class InstSequence;
class LoadStoreSequence;

using UFNodeMapType = SmallVector<unsigned, 16>;

using InstNumMapType = DenseMap<Instruction *, unsigned>;
using InstructionVecType = SmallVector<Instruction *, 16>;
using InstCloneMapType = DenseMap<Instruction *, Instruction *>;
using InstInstMapType = DenseMap<Instruction *, Instruction *>;
using InstructionSetType = SmallPtrSet<Instruction *, 16>;
using InstructionUSetType = std::unordered_set<Instruction *>;
using InstInstVecMapType = DenseMap<Instruction *, InstructionVecType>;
using VecInstVecType = SmallVector<InstructionVecType, 8>;
using UserSetType = SmallPtrSet<User *, 8>;
using InstVecSetType = SmallVector<InstructionSetType, 8>;
using BasicBlockVecType = SmallVector<BasicBlock *, 4>;
using InstSequenceMapType = DenseMap<Instruction *, InstSequence *>;
using InstLoadStoreSeqMapType = DenseMap<Instruction *, LoadStoreSequence *>;
using PhiNodeSetType = SmallPtrSet<PHINode *, 4>;
using IntrinsicIDSet = DenseSet<Intrinsic::ID>;

enum BlockType {
  Preheader = 0,
  Header = 1,
  Exit = 2,
  // ensure that this is the last enum
  NumBlocks
};

class TPCOptUtils {
public:
  // check whether the instruction is an intrinsic of the given type
  static bool isIntrinsicOfType(Instruction *I, Intrinsic::ID IntrinID) {
    if (auto Intrin = dyn_cast<IntrinsicInst>(I))
      return (Intrin->getIntrinsicID() == IntrinID);
    return false;
  }

  // check if the given instruction is of the type of one of those in the given
  // vector of types
  static bool isIntrinsicOfType(Instruction *I, IntrinsicIDSet &IntrinIDs) {
    if (auto Intrin = dyn_cast<IntrinsicInst>(I)) {
      Intrinsic::ID InstIntrinID = Intrin->getIntrinsicID();
      return (IntrinIDs.find(InstIntrinID) != IntrinIDs.end());
    }
    return false;
  }

  /// Returns true if \p I is a TPC coord-update
  static bool isCoordUpdate(const Instruction *I) {
    if (auto Intrin = dyn_cast<IntrinsicInst>(I)) {
      Intrinsic::ID InstIntrinID = Intrin->getIntrinsicID();
      if (InstIntrinID == Intrinsic::tpc_add_mask ||
          InstIntrinID == Intrinsic::tpc_set_indx)
        return true;
    }

    if (const auto *II = dyn_cast<InsertElementInst>(I)) {
      Type *Int5Ty = FixedVectorType::get(Type::getInt32Ty(I->getContext()), 5);
      if (Int5Ty == II->getType())
        return true;
    }

    return false;
  }

  // check if the given instruction is of the type of one of the ld_tnsr
  // variants
  static bool isLoadTensor(Instruction *I) {
    if (auto Intrin = dyn_cast<IntrinsicInst>(I)) {
      Intrinsic::ID InstIntrinID = Intrin->getIntrinsicID();
      return (InstIntrinID == Intrinsic::tpc_ld_tnsr ||
              InstIntrinID == Intrinsic::tpc_ld_tnsr_low);
    }
    return false;
  }

  // check if the given instruction is of the type of one of the st_tnsr
  // variants
  static bool isStoreTensor(Instruction *I) {
    if (auto Intrin = dyn_cast<IntrinsicInst>(I)) {
      Intrinsic::ID InstIntrinID = Intrin->getIntrinsicID();
      return (InstIntrinID == Intrinsic::tpc_st_tnsr ||
              InstIntrinID == Intrinsic::tpc_st_tnsr_low ||
              InstIntrinID == Intrinsic::tpc_st_tnsr_partial);
    }
    return false;
  }

  // print the control flow graph of the Function the working loop is in
  static void dumpCFG(Loop *L) {
    Function *F = L->getHeader()->getParent();
    std::ostringstream ss;
    ss << "digraph G {\n";
    for (auto &BBObj : *F) {
      BasicBlock *BB = &BBObj;
      for (BasicBlock *Pred : predecessors(BB)) {
        ss << "\t" << Pred->getName().str() << " -> " << BB->getName().str()
           << ";\n";
      }
    }
    ss << "}\n";
    TPC_OPT_UTILS_DEBUG("Function CFG : \n" << ss.str())
  }

  // get the const/non-const addend based on flag from the given instruction
  static Value *getAddend(Instruction *I, bool ConstFlag = true) {
    // instruction types - currently assuming only 'add'
    assert((TPCOptUtils::isIntrinsicOfType(I, Intrinsic::tpc_add_mask) ||
            I->getOpcode() == Instruction::Add) &&
           "Invalid Instruction type");
    // the addend value can be operand-0 or operand-1; in any case it has to be
    // a Constant
    Value *Op0 = I->getOperand(0);
    Value *Op1 = I->getOperand(1);
    assert(!(isa<Constant>(Op0) && isa<Constant>(Op1)) &&
           "Operand-0 and Operand-1 both cannot be Constant Values");
    assert((isa<Constant>(Op0) || isa<Constant>(Op1)) &&
           "Either Operand-0 or Operand-1 must be a Constant Value");
    if (ConstFlag)
      return isa<Constant>(Op0) ? Op0 : Op1;
    else
      return isa<Constant>(Op0) ? Op1 : Op0;
  }

  // This function returns the step value operand for the given Coord update
  // instruction
  static Value *getStepValue(Instruction *I) {
    bool IsAddMask = TPCOptUtils::isIntrinsicOfType(I, Intrinsic::tpc_add_mask);
    bool IsSetIndex =
        TPCOptUtils::isIntrinsicOfType(I, Intrinsic::tpc_set_indx);
    bool IsInsertElement = isa<InsertElementInst>(I);
    if (!(IsAddMask || IsInsertElement || IsSetIndex)) {
      TPC_OPT_UTILS_DEBUG("Unexpected Coord update instruction")
      return nullptr;
    }
    if (IsAddMask)
      return getAddend(I);
    else if (IsInsertElement)
      return I->getOperand(1);
    else // set_index
      return I->getOperand(0);
  }

  static MDNode *getUnrollMetadataForLoop(const Loop *L, StringRef Name) {
    if (MDNode *LoopID = L->getLoopID())
      return GetUnrollMetadata(LoopID, Name);
    return nullptr;
  }

  static std::string getTypeString(Type *T) {
    std::string type_str;
    llvm::raw_string_ostream rso(type_str);
    T->print(rso);
    return rso.str();
  }

  static std::string getBasicBlockStr(BlockType BT) {
    switch (BT) {
    case BlockType::Preheader:
      return "Preheader Block";
    case BlockType::Header:
      return "Header Block";
    case BlockType::Exit:
      return "Exit Block";
    default:
      return "Unknown Block";
    };
  }

  static BasicBlock *getLoopBlock(Loop *L, BlockType BT) {
    switch (BT) {
    case BlockType::Preheader:
      return L->getLoopPreheader();
    case BlockType::Header:
      return L->getHeader();
    case BlockType::Exit:
      return L->getExitBlock();
    default:
      return nullptr;
    };
  }

  static bool isPhiOfType(PHINode &PhiNode, BasicBlock *BB,
                          Intrinsic::ID IntrinID) {
    Instruction *BBIncome =
        dyn_cast<Instruction>(PhiNode.getIncomingValueForBlock(BB));
    if (!BBIncome)
      return false;

    return isIntrinsicOfType(BBIncome, IntrinID);
  }

  static void insertIntoBlock(BasicBlock *BB, Instruction *I,
                              BasicBlock::iterator InsertIt,
                              BasicBlock::iterator PhiInsertIt,
                              std::string DebugTag) {
    TPC_OPT_UTILS_DEBUG(DebugTag << "----")
    TPC_OPT_UTILS_DEBUG(DebugTag << "Inserting :" << *I)

    auto InsertPt = (isa<PHINode>(I) ? PhiInsertIt : InsertIt);
    TPC_OPT_UTILS_DEBUG(DebugTag << "Into :" << BB->getName())
    TPC_OPT_UTILS_DEBUG(DebugTag << "@before " << *InsertPt)

    BB->getInstList().insert(InsertPt, I);
    TPC_OPT_UTILS_DEBUG(DebugTag << "Inserted : " << *I)
    TPC_OPT_UTILS_DEBUG(DebugTag << "----")
  }

  static bool insertIntoBlock(Loop *L, BlockType BT, Instruction *I,
                              InstructionSetType &InsertedBlockInsts,
                              BasicBlock::iterator InsertIt,
                              BasicBlock::iterator PhiInsertIt,
                              std::string DebugTag) {
    assert(L && "<nullptr> loop not accepted");
    assert(I && "<nullptr> cannot be inserted into BB");

    auto *BB = getLoopBlock(L, BT);
    // if not already inserted
    if (InsertedBlockInsts.find(I) == InsertedBlockInsts.end()) {
      insertIntoBlock(BB, I, InsertIt, PhiInsertIt, DebugTag);
      InsertedBlockInsts.insert(I);
      return true;
    }
    return false;
  }

  // create a PHI node with the given number of predecessors as the number of
  // incoming values, all of them set as the 'Val'
  static PHINode *createPHIWithIncomingPreds(BasicBlock *BB,
                                             unsigned PredsCount, Value *Val) {
    assert(BB && "Invalid BasicBlock");

    PHINode *PhiNode = PHINode::Create(Val->getType(), PredsCount);
    for (BasicBlock *Pred : predecessors(BB))
      PhiNode->addIncoming(Val, Pred);

    return PhiNode;
  }

  // find the last use of 'I' in it's Block
  static Instruction *findLastUse(Instruction *I, InstNumMapType &BBInstOrder) {
    Instruction *LastUse = nullptr;
    BasicBlock *BB = I->getParent();

    if (BBInstOrder.empty()) {
      unsigned InstCount = 0;
      for (Instruction &Inst : *BB)
        BBInstOrder.insert(std::make_pair(&Inst, InstCount++));
    }

    for (auto User : I->users()) {
      if (isa<PHINode>(User))
        continue;
      Instruction *UserInst = cast<Instruction>(User);
      if (UserInst->getParent() != BB)
        continue;

      if (LastUse == nullptr) {
        LastUse = UserInst;
        continue;
      }
      if (BBInstOrder[LastUse] < BBInstOrder[UserInst])
        LastUse = UserInst;
    }

    return LastUse;
  }

  // An unrolled loop could have possibly been strength-reduced by replacing
  // Induction 'add' steps with Induction 'Or' steps (except the final
  // Induction step which remains 'add')
  //
  // In such a case, there may be multiple users of the InductionPHI.
  // This method ensures both are identifed as Induction steps.
  //
  // E.g:
  //
  // %for.body:
  //                                        ; unrolled by 4
  //  %IndPHI = [%step.4, Header], [...]
  //  %step.1 = add %IndPhi, 1
  //  %step.2 = add %step.1, 1
  //  %step.3 = add %step.2, 1
  //  %step.4 = add %step.3, 1
  //  icmp ... %step.4
  //  br ...
  //
  //
  // %for.body:
  //                                        ; unrolled by 4
  //                                        ; Induction strength-reduced
  //  %IndPHI = [%step.4, Header], [...]
  //  %step.1 = or %IndPhi, 1
  //  %step.2 = or %IndPhi, 2
  //  %step.3 = or %IndPhi, 3
  //  %step.4 = add %IndPhi, 4
  //  icmp ... %step.4
  //  br ...
  //
  static bool populateInductionInsts(Loop *L, ScalarEvolution *SE,
                                     InstructionSetType &InductionInsts) {
    PHINode *InductionPhi = L->getInductionVariable(*SE);

    for (auto User : InductionPhi->users()) {
      Instruction *I = cast<Instruction>(User);
      if (I->getOpcode() != Instruction::Add &&
          I->getOpcode() != Instruction::Sub &&
          I->getOpcode() != Instruction::Or)
        continue;
      InductionInsts.insert(I);
    }
    assert(InductionInsts.size() &&
           "The loop InductionPHI must have at least one update");

    // nothing to do
    if (InductionInsts.size() > 1) {
      // add the InductionPHI itself
      InductionInsts.insert(InductionPhi);
      return true;
    }

    // get the only step found till now
    Instruction *CurInd = *InductionInsts.begin();
    while (CurInd) {
      Instruction *NextStep = nullptr;
      for (auto User : CurInd->users()) {
        Instruction *I = cast<Instruction>(User);
        if (I->getOpcode() != Instruction::Add &&
            I->getOpcode() != Instruction::Sub)
          continue;

        auto *Op0 = dyn_cast<Instruction>(I->getOperand(0));
        auto *Op1 = dyn_cast<Instruction>(I->getOperand(1));
        if (!isa<Constant>(Op0) && !isa<Constant>(Op1))
          continue;

        if (CurInd == Op0 || CurInd == Op1) {
          NextStep = I;
          InductionInsts.insert(NextStep);
          // there cannot be multiple Induction updates to the previous update
          break;
        }
      }
      CurInd = NextStep;
    }

    return true;
  }

  // populate the instructions related to branching and induction update
  static bool populateLoopStructInsts(Loop *L,
                                      InstructionSetType &LoopStructureInsts) {
    assert(L->getSubLoops().size() == 0 && "Not an innermost loop");
    BasicBlock *HeaderBlock = L->getHeader();

    auto It = HeaderBlock->end();
    It--;
    Instruction *LastInst = &(*It);
    if (!isa<BranchInst>(LastInst))
      return false;
    InstructionSetType SetA, SetB;
    InstructionSetType *NextSet = &SetA, *WorkingSet = &SetB;
    WorkingSet->insert(LastInst);
    LoopStructureInsts.insert(LastInst);
    while (WorkingSet->size()) {
      for (auto Inst : *WorkingSet) {
        for (unsigned OpIdx = 0; OpIdx < Inst->getNumOperands(); OpIdx++) {
          if (Instruction *OpDef =
                  dyn_cast<Instruction>(Inst->getOperand(OpIdx))) {
            if (OpDef->getParent() != HeaderBlock)
              continue;
            if (!isa<PHINode>(OpDef))
              NextSet->insert(OpDef);
            LoopStructureInsts.insert(OpDef);
          }
        }
      }
      WorkingSet->clear();

      auto *TempSet = NextSet;
      NextSet = WorkingSet;
      WorkingSet = TempSet;
    }
    return true;
  }

  // This function accepts a Set of Intrinsic IDs considered as non-compute by
  // the programmer, and returns false for input Intructions that are Intrinsics
  // with these IDs. It will be the responsibility of the programmer using this
  // functionality to populate the Non-Compute set. With this design, we allow
  // contextual classification of Instructions into compute and non-computes.
  //
  // For e.g.:
  // Both arithmetic and load instructions can use predicate instructions
  //
  // - In one context, such as determining the profitability of
  //   coord-simplify-pass by analyzing it's effect on live-ranges of
  //   Instructions, categorizing such predicates as compute is necessary
  //
  // - In another context, the pipelining pass may want to use this function to
  //   verify if it's clusterizer missed including any compute that is not
  //   between a load and a store. So here a predicate used by load need not be
  //   treated as compute (as it belongs to a pre-load sequence).
  //
  // Apart from the non-compute Set check, the function also accepts a Set of
  // Instructions, to avoid false positives, that are related to the
  // loop-structure such as branching, induction update instructions etc.
  // All instructions with type 5xi32, and PhiNodes, InsertElement, users of
  // Induction Phi (because most-likely a compute will only use Induction
  // variables indirectly through 5xi32) etc., will be treated as non-compute.
  //
  // Lambda signature :
  // isInductionPHI([] (PHINode *) -> bool {})
  template <typename Lambda>
  static bool isComputeInst(Instruction *I,
                            DenseSet<Intrinsic::ID> &NonComputeIntrinIDs,
                            InstructionSetType &LoopStructureInsts,
                            Lambda isInductionPHILambda) {
    BasicBlock *BB = I->getParent();
    Type *T5xi32 = FixedVectorType::get(Type::getInt32Ty(BB->getContext()), 5);
    if (cast<Value>(I)->getType() == T5xi32)
      return false;

    if (LoopStructureInsts.find(I) != LoopStructureInsts.end() ||
        TPCOptUtils::isIntrinsicOfType(I, NonComputeIntrinIDs))
      return false;

    if (isa<PHINode>(I) || isa<BranchInst>(I) || isa<InsertElementInst>(I) ||
        isa<ICmpInst>(I))
      return false;

    // if I uses induction phi, it most likely cannot be a compute
    // (induction update instructions will be caught here)
    for (auto User : I->users()) {
      if (auto *Phi = dyn_cast<PHINode>(User)) {
        if (isInductionPHILambda(Phi))
          return false;
      }
    }

    return true;
  }

  // f(Instruction *)
  template <typename Lambda>
  static Instruction *followCoordDefChain(Instruction *FirstUpdateInst,
                                          Intrinsic::ID IntrinID, Lambda f) {
    while (!isa<PHINode>(FirstUpdateInst)) {
      FirstUpdateInst = dyn_cast<Instruction>(FirstUpdateInst->getOperand(0));
      f(FirstUpdateInst);
    }
    return FirstUpdateInst;
  }

  // f(Instruction *)
  template <typename Lambda>
  static Instruction *followCoordUserChain(Instruction *LastUpdateInst,
                                           Intrinsic::ID IntrinID, Lambda f) {
    bool Change = true;
    while (Change) {
      Change = false;
      for (auto User : LastUpdateInst->users()) {
        Instruction *UserInst = dyn_cast<Instruction>(User);
        if (isIntrinsicOfType(UserInst, IntrinID)) {
          LastUpdateInst = UserInst;
          f(LastUpdateInst);
          Change = true;
          break;
        }
      }
    }

    return LastUpdateInst;
  }

  // f(Instruction *User)
  template <typename Lambda>
  static void replaceUsesOfWith(
      Instruction *Of, Instruction *With,
      Lambda f = [](Instruction *) -> bool { return true; },
      std::string DebugTag = "\t") {
    UserSetType UserSet;
    for (User *U : Of->users())
      UserSet.insert(U);
    for (const auto &UserObj : UserSet) {
      Instruction *UserInst = cast<Instruction>(UserObj);
      TPC_OPT_UTILS_DEBUG(DebugTag << "----\n" << *UserInst)
      if (f(UserInst)) {
        UserInst->replaceUsesOfWith(Of, With);
      }
      TPC_OPT_UTILS_DEBUG(DebugTag << "----\n" << *UserInst)
    }
    return;
  }
};

class UnionFind {
public:
  explicit UnionFind(unsigned N) : N(N) {
    Parent.resize(N);
    for (unsigned i = 0; i < N; i++)
      Parent[i] = i;
  }

  unsigned findRoot(unsigned A) {
    assert((A < N) && "Invalid UnionFind Index");
    while (Parent[A] != Parent[Parent[A]])
      Parent[A] = Parent[Parent[A]];
    return Parent[A];
  }

  bool unionNodes(unsigned A, unsigned B) {
    assert((A < N) && "Invalid UnionFind Index");
    assert((B < N) && "Invalid UnionFind Index");

    unsigned RA = findRoot(A);
    unsigned RB = findRoot(B);
    // no merge required if A and B belongs to the same root
    if (RA == RB)
      return false;

    // simple tie break for new champion root
    if (RA < RB) // make RA the root
      Parent[RB] = RA;
    else // make RB the root
      Parent[RA] = RB;

    return true;
  }

  unsigned mapUniqueNodes(UFNodeMapType &FinalMap) {
    FinalMap.resize(N);
    unsigned Counter = 0;
    // re-assign number for root nodes
    for (unsigned i = 0; i < N; i++) {
      if (findRoot(i) == i) {
        FinalMap[i] = Counter;
        Counter += 1;
      }
    }
    // use re-assigned number from root nodes
    for (unsigned i = 0; i < N; i++) {
      unsigned Root = findRoot(i);
      if (Root != i) {
        unsigned MapVal = FinalMap[Root];
        FinalMap[i] = MapVal;
      }
    }
    return Counter;
  }

private:
  unsigned N;
  UFNodeMapType Parent;
};

#undef DEBUG_TYPE

} // end namespace llvm

#endif
