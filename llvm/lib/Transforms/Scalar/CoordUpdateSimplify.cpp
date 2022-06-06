// ===== CoordUpdateSimplify.cpp : TPC IR Coord update simplification ===== //
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2020 - This pass is a property of Habana labs
//
// This pass transforms the Coord update style originally of the form :
//
// From :
// -------------------------------------------------
// %Header:                         // depth=innermost
//  %Base = phi <5 x i32> ([%x, %Header], [%PreBase, %Preheader])   // Coord
//  Phi %w = phi i32 ([%w.inc, %Header], [%PreW, %Preheader])       // Induction
//
//  %x = insertelement(%Base, %w, #dim, ...)
//  ... uses ... (%x)
//
//  ...
//  %w.inc = add %w, #imm
//
// %Exit:
//  %Base.lcssa = phi <5 x i32> ([%x, %Header], [%PreBase, %Preheader])
//
// -------------------------------------------------
//
// To :
// -------------------------------------------------
// %Preheader:
//  %PreBase = ...
//  %PreW = ...
//  %PreInit = insertelement(%PreBase, %PreW, dim, ...)
//
// %Header:                         // depth=innermost
//  %Base = phi <5 x i32> ([%x, %Header], [%PreInit, %Preheader])
//  %w = phi i32 ([%w.inc, %Header], [%PreW, %Preheader])
//
//  ... uses ... (%Base)
//  %x = add.mask(%Base, #imm, #dim, ...)
//
//  ...
//  %w.inc = add %w, #imm
//
// %Exit:
//  %Base.lcssa = phi <5 x i32> ([%x, %Header], [%PreInit, %Preheader])
//
// -------------------------------------------------
//
// Author : Vinay V. Vasista
// Email  : vvasista@habana.ai
//
// ===== CoordUpdateSimplify.cpp : TPC IR Coord update simplification ===== //

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#include "TPCOptUtils.h"

#include <unordered_set>

using namespace llvm;

#define PassName "coord-simplify"
#define PassDescription                                                        \
  "Simplify costly TPC Coordinate update instructions in IR"
#define DEBUG_TYPE PassName

#define CSP_DEBUG(x) LLVM_DEBUG(dbgs() << "[CoordSimplPass] " << x << "\n");

static cl::opt<bool> CoordUpdateFlag("coord-update-simplify", cl::init(false),
                                     cl::Hidden);
// block size above which profitability is considered
static cl::opt<unsigned> BlockSizeCaution("csp-blocksize-caution",
                                          cl::init(100), cl::Hidden);
// block size beyond which transformation is not applied
static cl::opt<unsigned> BlockSizeMax("csp-blocksize-max", cl::init(200),
                                      cl::Hidden);
// maximum number of loads allowed in the innermost loop's Header block
static cl::opt<unsigned> MaxNumLoads("csp-max-num-loads", cl::init(1),
                                     cl::Hidden);
// maximum number of computes allowed in the innermost loop's Header block
static cl::opt<unsigned> MaxNumComputes("csp-max-num-computes", cl::init(4),
                                        cl::Hidden);
// tolerable fraction of BlockSize for live-range of coord (after
// simplification)
static cl::opt<double> LiveRangeToleranceFactor("csp-live-range-tolerance",
                                                cl::init(0.2), cl::Hidden);

class CoordUpdateSimplify : public LoopPass {
public:
  static char ID;
  CoordUpdateSimplify() : LoopPass(ID) {
    initializeCoordUpdateSimplifyPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<ScalarEvolutionWrapperPass>();
    getLoopAnalysisUsage(AU);
  }

  // entry
  bool runOnLoop(Loop *Lp, LPPassManager &LPM) override;

private:
  Loop *WorkingLoop;
  BasicBlock *HeaderBlock;
  BasicBlock *PreheaderBlock;
  BasicBlock *ExitBlock;
  Value *InductionStep;

  InstructionUSetType TargetInsts;
  InstInstMapType InstCloneMap;
  InstInstMapType ExitLcssaPhiMap;

  InstNumMapType BBInstOrder;

  IntrinsicIDSet NonComputeIntrinIDs;

  // this set contains all the instructions related to branch instruction and
  // induction steps
  InstructionSetType LoopStructureInsts;

  // TODO : use enum if a one-one mapping of TPCII enums is created for IR
  unsigned Int32OpType = 2;

  // insertion points
  BasicBlock::iterator BBInsertIt;
  BasicBlock::iterator BBPhiInsertIt;

  InductionDescriptor InductionDesc;
  ScalarEvolution *SE;

  void dumpLoopBlocks(std::string HeaderNote);

  void resetContainers();

  PHINode *getBaseVal(Instruction *I) {
    return dyn_cast<PHINode>(I->getOperand(0));
  }

  PHINode *getInsertVal(Instruction *I) {
    return dyn_cast<PHINode>(I->getOperand(1));
  }

  // get the set of all users of I in it's Parent Block
  InstructionSetType getInductionUses(Instruction *I);

  // initialize the set of IDs of non-compute Intrinsics
  void populateNonComputeIntrinIDs();

  // initialize Analysis Info objects
  void initAnalysisInfo();

  // check if the given PHI node is an Induction Phi
  bool isInductionPHI(PHINode *PhiNode);

  // fetch or prepare Preheader versions of the given PHINode
  Value *getPreheaderVersion(PHINode *I, std::string DebugTag = "\t");

  // add initialization instructions for the Target Coord instructions, in the
  // Preheader block
  void preInitCoords(std::string Indent = "\t");

  // create an add.mask instruction object derived from insertelement 'I'
  Instruction *createAddMaskInst(Instruction *I);

  // return if the instruction has at least one user outside it's parent block
  bool hasUseOutsideBlock(Instruction *I);

  // replace the uses of TargetInst with the new AddMaskInst, and if there are
  // any Lcssa PHI nodes using TargetInst, replace their uses with the
  // correction AddMask
  void patchLcssaPHINodes(Instruction *I, Instruction *AddMaskInst,
                          std::string Indent = "\t");

  // modify Header Coord instructions
  void replaceHeaderUpdates(std::string Indent = "\t");

  // set the step value for add.mask
  void computeAddMaskStep();

  // apply Coord update simplification
  void simplifyCoordUpdates();

  // check if the loop header block exhibits a Unary compute pattern
  bool hasProfitablePattern(std::string Indent = "\t");

  // check if the Coords simplification is profitable
  bool isSimplifyProfitable(std::string Indent = "\t");

  // check if it is safe to simplify the given coord instruction
  bool isSafeToSimplifyInst(Instruction *I, std::string Indent);

  // check if the Coords can be simplified
  bool isSimplifyPossible(std::string Indent = "\t");
};

char CoordUpdateSimplify::ID = 0;

INITIALIZE_PASS_BEGIN(CoordUpdateSimplify, PassName, PassDescription, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(LoopPass)
INITIALIZE_PASS_END(CoordUpdateSimplify, PassName, PassDescription, false,
                    false)

Pass *llvm::createCoordUpdateSimplifyPass() {
  return new CoordUpdateSimplify();
}

void CoordUpdateSimplify::dumpLoopBlocks(std::string HeaderNote) {
  LLVM_DEBUG(CSP_DEBUG("Loop Blocks : " << HeaderNote);
             CSP_DEBUG(*PreheaderBlock << "\n----");
             CSP_DEBUG(*HeaderBlock << "\n----");
             CSP_DEBUG(*ExitBlock << "\n----"););
}

void CoordUpdateSimplify::resetContainers() {
  TargetInsts.clear();
  LoopStructureInsts.clear();
  NonComputeIntrinIDs.clear();
}

// Since the Intrinsics structure do not contain information on whether they
// are of compute/non-compute type, this set (NonComputeIntrinIDs) is used in
// a categorization local to this pass, and does not intend to provide an
// exhaustive list of non-computes.
//
// For now this Set of non-compute Intrinsics serves the purpose of being
// conservative while bailing-out on cases with compute Inst count higher than
// a tolerable limit.
void CoordUpdateSimplify::populateNonComputeIntrinIDs() {
  NonComputeIntrinIDs.insert(Intrinsic::tpc_ld_tnsr);
  NonComputeIntrinIDs.insert(Intrinsic::tpc_ld_g);
  NonComputeIntrinIDs.insert(Intrinsic::tpc_st_tnsr);
  NonComputeIntrinIDs.insert(Intrinsic::tpc_st_g);
  NonComputeIntrinIDs.insert(Intrinsic::tpc_add_mask);
  NonComputeIntrinIDs.insert(Intrinsic::tpc_set_indx);
}

// get the users of the given Instruction within the Instruction's parent
// block, which are not PHI nodes and not of InsertElementInst type
InstructionSetType CoordUpdateSimplify::getInductionUses(Instruction *I) {
  InstructionSetType BlockUsers;
  for (auto User : I->users()) {
    Instruction *UserInst = cast<Instruction>(User);
    if (UserInst->getParent() != I->getParent())
      continue;
    if (isa<PHINode>(UserInst))
      continue;
    if (isa<InsertElementInst>(UserInst))
      continue;
    BlockUsers.insert(UserInst);
  }

  return BlockUsers;
}

// initialize the data needed for Induction Phi check
void CoordUpdateSimplify::initAnalysisInfo() {
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  WorkingLoop->getInductionDescriptor(*SE, InductionDesc);
}

bool CoordUpdateSimplify::isInductionPHI(PHINode *PhiNode) {
  return InductionDescriptor::isInductionPHI(PhiNode, WorkingLoop, SE,
                                             InductionDesc);
}

bool CoordUpdateSimplify::hasProfitablePattern(std::string Indent) {
  // populate useful info to detect profitable pattern
  populateNonComputeIntrinIDs();
  TPCOptUtils::populateLoopStructInsts(WorkingLoop, LoopStructureInsts);

  // NOTE : since this pass is scheduled before the unroll pass, we assume that
  // it is alright if this function conservatively reports false for manually
  // unrolled loops, with NumLoads > 1
  // Also, this check is intended to be temporary and open for modifications
  unsigned NumLoads = 0, NumComputes = 0;
  for (Instruction &Inst : *HeaderBlock) {
    Instruction *I = &Inst;
    if (TPCOptUtils::isComputeInst(
            I, NonComputeIntrinIDs, LoopStructureInsts,
            [&](PHINode *PhiNode) -> bool { return isInductionPHI(PhiNode); }))
      NumComputes += 1;
    else if (TPCOptUtils::isIntrinsicOfType(I, Intrinsic::tpc_ld_tnsr))
      NumLoads += 1;
  }
  CSP_DEBUG(Indent << "NumLoads = " << NumLoads)
  CSP_DEBUG(Indent << "NumComputes = " << NumComputes)

  // MaxNumLoads and MaxNumComputes are configurable tolerance limit for the
  // number of loads and computes resp. in the innermost loop's Header Block.
  // Default values are set as per the observations made on elemntwise
  // unary-like patterns and contain the effect of this pass to loops with
  // similar patterns.
  return (NumLoads == MaxNumLoads && NumComputes <= MaxNumComputes);
}

bool CoordUpdateSimplify::isSimplifyProfitable(std::string Indent) {
  // NOTE : for profitability (based on the observation of performance
  // improvements/regressions), the coord update simplify in it's current form
  // is limited to elementwise unary kernels and those with similar pattern.
  // TODO : this check is temporary and is supposed to evolve with further
  // observations
  if (!hasProfitablePattern(Indent + "\t")) {
    CSP_DEBUG(
        Indent
        << "Computation pattern not favourable for performance, exiting ...")
    return false;
  }

  // check if we need not worry about potential register spills
  unsigned BlockSize = HeaderBlock->getInstList().size();
  if (BlockSize <= BlockSizeCaution)
    return true;

  // see example below
  double SafeAndProfitableLiveRange =
      static_cast<double>(BlockSize) * LiveRangeToleranceFactor;

  auto It = TargetInsts.begin();
  while (It != TargetInsts.end()) {
    Instruction *I = *It;
    Value *LastUse = TPCOptUtils::findLastUse(I, BBInstOrder);
    Instruction *TargetLastUseInst = LastUse ? cast<Instruction>(LastUse) : I;

    Instruction *BasePHIInst = cast<Instruction>(getBaseVal(I));
    Value *BaseLastUse = TPCOptUtils::findLastUse(BasePHIInst, BBInstOrder);
    assert(BaseLastUse && "InsertElementInst is at least one user");
    Instruction *BaseLastUseInst = cast<Instruction>(BaseLastUse);

    // After :
    // distane between the BasePHI and Target's last use
    // BaseNewLastUse = max(
    //    [[ BasePHI ---> ~InsertElement~ ---> TargetLastUse ---> Add.Mask ]],
    //    [[ BasePHI ---> ...                         ---> BaseLastUseInst ]])
    // (after simplify, 'I' will be deleted and add.mask, inserted after I's
    // last use, will be using the BasePHI)
    Instruction *BaseNewLastUse =
        (BBInstOrder[TargetLastUseInst] < BBInstOrder[BaseLastUseInst])
            ? BaseLastUseInst
            : TargetLastUseInst;
    unsigned BaseLiveRangeAfter =
        BBInstOrder[BaseNewLastUse] - BBInstOrder[BasePHIInst];
    // TODO : modify this calculation if add.mask insertion point is changed

    // check live range safety
    // for e.g.:
    // BlockSize = 110, LiveRangeToleranceFactor = 0.2 =>
    // SafeAndProfitableLiveRange = 22
    // any BaseLiveRangeAfter greater than SafeAndProfitableLiveRange will avoid
    // the coord simplification
    if (static_cast<double>(BaseLiveRangeAfter) > SafeAndProfitableLiveRange) {
      // but, do one last check : has the live range changed at all?
      // Before :
      // BaseLiveRange =
      //    [[ BasePHI ---> InsertElement ---> BaseLastUse ]]
      unsigned BaseLiveRange =
          BBInstOrder[BaseLastUseInst] - BBInstOrder[BasePHIInst];
      // TargetLiveRange =
      //    BasePHI ---> [[ InsertElement ---> InsertElementLastUse ]]
      unsigned TargetLiveRange =
          BBInstOrder[TargetLastUseInst] - BBInstOrder[I];
      // longer of the two
      unsigned LiveRangeBefore =
          (BaseLiveRange > TargetLiveRange) ? BaseLiveRange : TargetLiveRange;
      if (LiveRangeBefore < BaseLiveRangeAfter) {
        CSP_DEBUG(Indent << "Not profitable to Simplify (skipping ...) :")
        CSP_DEBUG(Indent << *I)
        It = TargetInsts.erase(It);
        continue;
      }
    }
    It++;
  }

  return TargetInsts.size();
}

bool CoordUpdateSimplify::isSafeToSimplifyInst(Instruction *I,
                                               std::string Indent) {
  // check-1 : get the base
  auto *BasePHI = getBaseVal(I);
  if (!BasePHI) {
    CSP_DEBUG(Indent << "Base Value not a PHINode, continuing ...")
    return false;
  }

  // check-2.0 : BasePHI has no user other than the insertelement
  if (cast<Value>(BasePHI)->getNumUses() != 1) {
    CSP_DEBUG(Indent
              << "BasePHI has a user other than the TargetInst, continuing ...")
    return false;
  }

  // check-2.1 : 'I' should be the incoming value of the BasePHI
  Instruction *BaseIncoming =
      cast<Instruction>(BasePHI->getIncomingValueForBlock(HeaderBlock));
  if (BaseIncoming != I) {
    CSP_DEBUG(Indent << "Candidate should be HeaderBlock-Incoming Value of "
                        "it's Base Val, continuing ...")
    return false;
  }

  // check-3 : get the insert value
  auto *InsertValPHI = getInsertVal(I);
  if (!InsertValPHI) {
    CSP_DEBUG(Indent << "Insertion Value not a PHINode, continuing ...")
    return false;
  }
  // check-4 : check whether the InsertVal is Induction Phi
  if (!isInductionPHI(InsertValPHI)) {
    CSP_DEBUG(Indent << "Insertion Value not an Induction PHI, continuing ...")
    return false;
  }
  // check-5 : check whether there is just one update of the induction Phi
  InstructionSetType InductionUses = getInductionUses(InsertValPHI);
  if (InductionUses.size() > 1) {
    CSP_DEBUG(
        Indent
        << "Induction PHI does not have exactly one update, continuing ...")
    return false;
  }
  auto *InsertValIncoming =
      cast<Instruction>(InsertValPHI->getIncomingValueForBlock(HeaderBlock));
  auto *InductionUpdate = *InductionUses.begin();
  // check-6 : the only induction update should be the incoming value of the
  // Induction Phi for the Header block
  if (InductionUpdate != InsertValIncoming) {
    CSP_DEBUG(Indent << "Induction update should be HeaderBlock-Incoming "
                        "Value of the Induction PHI, continuing ...")
    return false;
  }

  // check-7 : ensure the Induction step is achieved through Add
  if (InductionUpdate->getOpcode() != Instruction::Add) {
    CSP_DEBUG(
        Indent << "Induction update should be add instruction, continuing ...")
    return false;
  }

  // check-8 : ensure the Induction step is a const value
  if (!(isa<ConstantInt>(InductionUpdate->getOperand(0)) ||
        isa<ConstantInt>(InductionUpdate->getOperand(1)))) {
    CSP_DEBUG(Indent << "Induction step must be a constant, continuing ...")
    return false;
  }

  // check-9 : ensure BasePHI has non-Constant Preheader incoming value
  if (isa<Constant>(BasePHI->getIncomingValueForBlock(PreheaderBlock))) {
    CSP_DEBUG(Indent << "BasePHI's Preheader incoming value cannot be "
                        "Constant, continuing ...")
    return false;
  }

  // check-10 : ensure the only user of TargetInst outside the HeaderBlock is
  // one LCSSA PHI node
  bool HasExitLcssaUser = false;
  for (auto User : I->users()) {
    Instruction *UserInst = cast<Instruction>(User);
    BasicBlock *ParentBB = UserInst->getParent();
    if (ParentBB == HeaderBlock)
      continue;
    if (UserInst->getParent() == ExitBlock && !HasExitLcssaUser &&
        isa<PHINode>(UserInst)) {
      ExitLcssaPhiMap.insert(std::make_pair(I, UserInst));
      HasExitLcssaUser = true;
    } else {
      CSP_DEBUG(
          Indent << "The loop is posiibly not in LCSSA form, continuing ...")
      return false;
    }
  }

  return true;
}

bool CoordUpdateSimplify::isSimplifyPossible(std::string Indent) {
  // check if the loop is the innermost
  if (WorkingLoop->getSubLoops().size()) {
    CSP_DEBUG(Indent << "Not an innerloop, exiting ...")
    return false;
  }

  // set basic blocks
  HeaderBlock = WorkingLoop->getHeader();
  PreheaderBlock = WorkingLoop->getLoopPreheader();
  if (!PreheaderBlock) {
    CSP_DEBUG(Indent << "Preheader block not found for loop, exiting ...");
    return false;
  }
  ExitBlock = WorkingLoop->getExitBlock();
  if (!ExitBlock) {
    CSP_DEBUG(Indent << "Exit block not found for loop, exiting ...");
    return false;
  }

  // The loop should have no branching
  if (WorkingLoop->getNumBlocks() > 1) {
    CSP_DEBUG(Indent << "Branching detected within the loop, exiting ...")
    return false;
  }

  // conservatively avoid applying transformation if the block size is beyond
  // the maximum value allowed
  if (HeaderBlock->getInstList().size() > BlockSizeMax) {
    CSP_DEBUG(
        Indent
        << "HeaderBlock is too big to safely simplify coords, exiting ...")
    return false;
  }

  // collect all insert-element instructions, which can be translated to
  // add.mask intrinsics
  for (Instruction &Inst : *HeaderBlock) {
    Instruction *I = &Inst;
    if (!isa<InsertElementInst>(I))
      continue;

    CSP_DEBUG(Indent << "Candidate insertelement Inst :")
    CSP_DEBUG(Indent << *I)

    if (isSafeToSimplifyInst(I, Indent + "\t"))
      TargetInsts.insert(I);
  }

  LLVM_DEBUG(CSP_DEBUG(Indent << "Target Coord update instructions : {");
             for (auto I
                  : TargetInsts) {
               CSP_DEBUG(Indent << *I);
             } CSP_DEBUG(Indent << "}"));

  return TargetInsts.size();
}

Value *CoordUpdateSimplify::getPreheaderVersion(PHINode *PhiNode,
                                                std::string DebugTag) {
  Instruction *I = cast<Instruction>(PhiNode);
  auto It = InstCloneMap.find(I);
  if (It != InstCloneMap.end())
    return It->second;

  Value *PreIncomingValue = PhiNode->getIncomingValueForBlock(PreheaderBlock);
  Instruction *PreIncomingInst = dyn_cast<Instruction>(PreIncomingValue);
  if (!PreIncomingInst)
    return PreIncomingValue;

  // this could be a pass-through Phi node, so creation maybe required
  if (PreIncomingInst->getParent() != PreheaderBlock) {
    unsigned PredsCount = llvm::pred_size(PreheaderBlock);
    if (PredsCount > 1) {
      // need to create a new Phi node
      auto *PrePHI = TPCOptUtils::createPHIWithIncomingPreds(
          PreheaderBlock, PredsCount, PreIncomingValue);
      TPCOptUtils::insertIntoBlock(PreheaderBlock, cast<Instruction>(PrePHI),
                                   BBInsertIt, BBPhiInsertIt, DebugTag + "\t");
      PreIncomingValue = cast<Value>(PrePHI);
      PreIncomingInst = cast<Instruction>(PrePHI);
    }
  }
  InstCloneMap.insert(std::make_pair(I, PreIncomingInst));

  return PreIncomingValue;
}

void CoordUpdateSimplify::preInitCoords(std::string Indent) {
  std::string DebugTag = "<preInitCoords> " + Indent;
  CSP_DEBUG(DebugTag << "[Begin]")

  BBPhiInsertIt = PreheaderBlock->begin();
  BBInsertIt = PreheaderBlock->end();
  BBInsertIt--;

  CSP_DEBUG(DebugTag << "Visiting each TargetInst ...")
  for (auto I : TargetInsts) {
    CSP_DEBUG(DebugTag << "----")
    CSP_DEBUG(DebugTag << "\t" << *I)

    auto *BasePHI = getBaseVal(I);
    auto *InsertValPHI = getInsertVal(I);

    // prepare Pre-BasePhi
    Value *PreBasePHI = getPreheaderVersion(BasePHI, DebugTag);

    // prepare Pre-InductionPhi
    Value *PreInsertValPHI = getPreheaderVersion(InsertValPHI, DebugTag);

    // clone insertelement with operands:
    // 0 - PreBasePHI
    // 1 - PreInsertValPHI
    // with this we achieve initialization of Coord before entering the Header
    Instruction *IClone = I->clone();
    IClone->replaceUsesOfWith(BasePHI, PreBasePHI);
    IClone->replaceUsesOfWith(InsertValPHI, PreInsertValPHI);
    TPCOptUtils::insertIntoBlock(PreheaderBlock, IClone, BBInsertIt,
                                 BBPhiInsertIt, DebugTag + "\t");

    // fix the Header Phis
    // BasePHI
    BasePHI->removeIncomingValue(PreheaderBlock, false);
    BasePHI->addIncoming(IClone, PreheaderBlock);

    if (isa<Constant>(PreInsertValPHI))
      continue;
    // InductionPHI
    InsertValPHI->removeIncomingValue(PreheaderBlock, false);
    InsertValPHI->addIncoming(PreInsertValPHI, PreheaderBlock);
  }

  CSP_DEBUG(DebugTag << "[End]")

  return;
}

Instruction *CoordUpdateSimplify::createAddMaskInst(Instruction *I) {
  // 'I' should be an insertelement instruction
  assert(isa<InsertElementInst>(I) &&
         "Invalid Target instruction - not an InsertElementInst");

  auto *BasePHI = getBaseVal(I);

  // get the element index from insertelement
  auto *Dim = cast<ConstantInt>(I->getOperand(2));

  IRBuilder<> Builder(HeaderBlock);
  LLVMContext &C = HeaderBlock->getContext();
  Type *Int32Ty = Type::getInt32Ty(C);
  Type *Int5x32Ty = FixedVectorType::get(Int32Ty, 5);

  Constant *IRFLocation =
      ConstantInt::get(Int32Ty, (1LL << Dim->getZExtValue()));
  Constant *ZeroVal = ConstantInt::get(Int32Ty, 0);

  Function *AddMaskFunc = Intrinsic::getDeclaration(
      HeaderBlock->getModule(), Intrinsic::tpc_add_mask, {Int5x32Ty, Int32Ty});
  Instruction *AddMask = Builder.CreateCall(
      AddMaskFunc, {BasePHI, InductionStep, IRFLocation,
                    ConstantInt::get(Type::getInt8Ty(C), Int32OpType), ZeroVal,
                    BasePHI, ConstantInt::get(Type::getInt1Ty(C), 1),
                    ConstantInt::get(Type::getInt1Ty(C), 0)});

  return AddMask;
}

void CoordUpdateSimplify::patchLcssaPHINodes(Instruction *I,
                                             Instruction *AddMaskInst,
                                             std::string Indent) {
  auto It = ExitLcssaPhiMap.find(I);
  if (It == ExitLcssaPhiMap.end()) {
    CSP_DEBUG(Indent << "No lcssa phi node to fix, exiting ...")
    return;
  }

  Instruction *LcssaPHIInst = It->second;
  LcssaPHIInst->replaceUsesOfWith(I, AddMaskInst);
  // now add correction instruction
  int64_t InductionStepInt = cast<ConstantInt>(InductionStep)->getSExtValue();
  auto *NewStep =
      ConstantInt::get(InductionStep->getType(), -InductionStepInt, true);
  Instruction *LcssaCorrection = AddMaskInst->clone();
  LcssaCorrection->replaceUsesOfWith(getBaseVal(I), LcssaPHIInst);
  LcssaCorrection->setOperand(1, NewStep);

  BBInsertIt = ExitBlock->end();
  BBInsertIt--;
  TPCOptUtils::insertIntoBlock(ExitBlock, LcssaCorrection, BBInsertIt,
                               BBPhiInsertIt, Indent + "\t");
  // replace the uses of LCSSA PHI with the correction
  for (auto User : LcssaPHIInst->users()) {
    Instruction *UserInst = cast<Instruction>(User);
    if (UserInst == LcssaCorrection)
      continue;
    UserInst->replaceUsesOfWith(LcssaPHIInst, LcssaCorrection);
  }

  return;
}

void CoordUpdateSimplify::replaceHeaderUpdates(std::string Indent) {
  std::string DebugTag = "<replaceHeaderUpdates> " + Indent;

  CSP_DEBUG(DebugTag << "[Begin]")

  CSP_DEBUG(DebugTag << "Visiting each TargetInst ...")
  for (auto I : TargetInsts) {
    CSP_DEBUG(DebugTag << "----")
    CSP_DEBUG(DebugTag << "\t" << *I)

    Instruction *AddMaskInst = createAddMaskInst(I);

    // identify the last use of the target
    Instruction *LastUse = TPCOptUtils::findLastUse(I, BBInstOrder);
    // when there is no use of target within the block, move the add.mask
    // right next to the target (before it's deletion)
    Instruction *MoveAfter = LastUse ? LastUse : I;
    // NOTE : the case where there is no user of insertelement within this
    // block should have been handled in loop strength reduction
    // TODO : avoid add.mask simplification in this case, directly insert the
    // live-out value of induction var in the Exit block (and simplify loop)

    // lcssa phi should now use the new add.mask value
    patchLcssaPHINodes(I, AddMaskInst);

    // fix the BasePHI with add.mask incoming value from HeaderBlock
    auto *BasePHI = getBaseVal(I);
    BasePHI->removeIncomingValue(HeaderBlock);
    BasePHI->addIncoming(AddMaskInst, HeaderBlock);

    // all other uses within HeaderBlock will now use BasePHI
    I->replaceAllUsesWith(BasePHI);
    // position the add.mask instruction right after the last use of
    // insertelement
    AddMaskInst->moveAfter(MoveAfter);

    assert(I->getNumUses() == 0 && "Incorrect user replacement!");
    // insertelement instruction is no longer required
    I->eraseFromParent();
  }
  CSP_DEBUG(DebugTag << "[End]")

  return;
}

void CoordUpdateSimplify::computeAddMaskStep() {
  Instruction *I = *(TargetInsts.begin());
  auto *InsertValPHI = getInsertVal(I);
  Instruction *InsertValIncoming =
      cast<Instruction>(InsertValPHI->getIncomingValueForBlock(HeaderBlock));
  // TODO: use TPCOptUtils::getAddend
  // the check-7 ensures that incoming inst is an 'Add'
  Value *InductionOp0 = InsertValIncoming->getOperand(0);
  Value *InductionOp1 = InsertValIncoming->getOperand(1);
  InductionStep = isa<ConstantInt>(InductionOp0) ? InductionOp0 : InductionOp1;
  return;
}

void CoordUpdateSimplify::simplifyCoordUpdates() {
  // init step value
  computeAddMaskStep();

  // initialize the TargetVal outside the Header
  preInitCoords();
  dumpLoopBlocks("(After preInitCoords())");

  replaceHeaderUpdates();
  dumpLoopBlocks("(After replaceHeaderUpdates())");

  return;
}

bool CoordUpdateSimplify::runOnLoop(Loop *L, LPPassManager &LPM) {
  if (!CoordUpdateFlag) {
    CSP_DEBUG("Coordinate Simplify Pass disabled for this loop; not applied")
    return false;
  }

  WorkingLoop = L;
  initAnalysisInfo();

  if (!isSimplifyPossible()) {
    CSP_DEBUG("Coordinate Simplification is not possible; not applied")
    resetContainers();
    return false;
  }

  if (!isSimplifyProfitable()) {
    CSP_DEBUG("Coordinate Simplification is not profitable; not applied")
    resetContainers();
    return false;
  }

  simplifyCoordUpdates();
  resetContainers();

  return true;
}
