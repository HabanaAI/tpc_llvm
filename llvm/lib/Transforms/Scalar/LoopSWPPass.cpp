// ============= LoopSWPPass.cpp : TPC IR Software Pipelining ============= //
//
//
//                     The LLVM Compiler Infrastructure:
//
//            2020,2021 - This pass is a property of Habana labs
//
// Author : Vinay V. Vasista
// Email  : vvasista@habana.ai
//
// ======================================================================== //

#include "llvm/Transforms/Scalar/LoopSWPPass.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#include "InstSequence.h"
#include "TPCIterationClusterizer.h"
#include "TPCOptUtils.h"

using namespace llvm;

#define PassName "loop-swp"
#define PassDescription                                                        \
  "Apply Software Pipelining optimization for TPC at LLVM-IR"
#define DEBUG_TYPE PassName

#define LOOP_SWP_DEBUG(x) LLVM_DEBUG(dbgs() << "[LoopSWPPass] " << x << "\n");

static cl::opt<bool> LoopSWPFlag("loop-software-pipelining", cl::init(false),
                                 cl::Hidden);
static cl::opt<unsigned> LoopSWPLoadPrefetchCount("swp-load-prefetch-count",
                                                  cl::init(0), cl::Hidden);
#define BAIL_OUT_TAG "[BAIL-OUT] "
#define DISABLED_TAG "[DISABLED] "
#define SUCCESS_TAG "[SUCCESS] "

class LoopSWP : public LoopPass {
public:
  static char ID;
  LoopSWP() : LoopPass(ID) {
    initializeLoopSWPPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<ScalarEvolutionWrapperPass>();
    getLoopAnalysisUsage(AU);
  }

private:
  /* Data */

  unsigned NumClusters = 0;
  unsigned LoadPrefetchCount = 0;
  bool IsAccumulationLoop = false;
  bool HasLoadIncome = false;
  bool HasInductionInCoordSeq = false;

  // Loop to be pipelined
  Loop *WorkingLoop = nullptr;
  // BasicBlocks of the working loop
  BasicBlock *HeaderBlock, *PreheaderBlock, *ExitBlock;

  // At the end of clusterization, this vector of ClusterInfo struct will be
  // populated with all information related to the loop's iteration clusters.
  // The size of this vector will be equal to the number of clusters found.
  ClusterInfoVecType LoopClusters;

  // link from Preheader/Exit Phi to Header Phi
  InstInstMapType PreToHeaderPhiMap;
  InstInstMapType ExitToHeaderPhiMap;

  // set of all induction phis
  PhiNodeSetType InductionPhiSet;

  // set of Accum Phi instructions
  InstructionSetType AccumPhis[BlockType::NumBlocks];

  // mapping from an Inst to it's clone (across blocks)
  InstCloneMapType InstCloneMap[BlockType::NumBlocks];

  // mappings from each Load/GlobalLoad instruction to Load Phi
  InstInstMapType LoadToPhiMap[BlockType::NumBlocks];
  // mappings from each Accum user to it's Accum Phi
  InstInstMapType AccumToPhiMap[BlockType::NumBlocks];

  // An Alpha is an Instruction that is at the head of the chain of updates
  // it belongs to. For e.g.: In the following BasicBlock, consider the chain
  // of Coord updates:
  //
  // BB :
  //   %ifmCoord = phi <5 x i32> (...)
  //   %ifmCoord.1 = add.mask(1, ifmCoord, width, ...)
  //   %ifmCoord.2 = add.mask(1, ifmCoord.1, width, ...)
  //   %ifmCoord.3 = add.mask(1, ifmCoord.2, width, ...)
  //
  // Here, %ifmCoord is the Alpha for the chain shown.
  // The head of such a chain not necessarily has to be a Phi node, hence the
  // term Alpha is used for convenience.
  //
  // mappings from an update Inst to it's Alpha
  InstInstMapType InstToAlphaMap[BlockType::NumBlocks];

  // mapping from phi to it's last update instruction
  // the key can be a coord update phi or an accum phi
  InstInstMapType PhiLastUpdateMap[BlockType::NumBlocks]; // TODO : deprecate
  // mapping from and Inst to it's update Inst
  InstInstMapType InstUpdateMap[BlockType::NumBlocks];

  // this set contains all the instructions not clustered by traversing
  // use-def chain of store instructions
  InstructionSetType UnclusteredInstructions;

  // this set contains all the instructions related to branch instruction and
  // induction steps
  InstructionSetType LoopStructureInsts;

  // this set contains all the Induction step instructions
  InstructionSetType InductionInsts;

  // set of instructions inserted into the current block being modified
  InstructionSetType InsertedBlockInsts;

  // phi node insertion point for the basic block being modified
  BasicBlock::iterator BBPhiInsertIt;

  // insertion point for the basic block being modified
  BasicBlock::iterator BBInsertIt;

  // mapping from pivot load/store instructions to their InstSequence object ref
  InstSequenceMapType InstSeqMap;
  // mapping from pivot load instructions to their LoadStoreSequence object ref
  InstLoadStoreSeqMapType InstLoadStoreSeqMap;

  InductionDescriptor InductionDesc;
  ScalarEvolution *SE;

  /* Util functions */

  // set/get number of clusters
  void setNumClusters(unsigned N) { NumClusters = N; }
  unsigned getNumClusters() { return NumClusters; }

  // set/get pipelining load count
  void setLoadPrefetchCount();
  unsigned getLoadPrefetchCount() { return LoadPrefetchCount; }

  // get/set whether the loop has accumulator
  void setIsAccumulationLoop(bool IsAccumLoop) {
    IsAccumulationLoop = IsAccumLoop;
  }
  bool getIsAccumulationLoop() { return IsAccumulationLoop; }

  // get/set whether the Loop has Loads with income value
  bool getHasLoadIncome() { return HasLoadIncome; }
  void setHasLoadIncome() { HasLoadIncome = true; }

  // initialize Analysis Info objects
  void initAnalysisInfo();

  // check whether the given Phi node is an Induction Phi
  bool isInductionPHI(PHINode *PhiNode);

  // check whether the given Instruction is an Induction Phi
  bool isInductionPHI(Instruction *I);

  // print the contents of the PhiLastUpdateMap
  void dumpPhiLastUpdateMap(BlockType BT = BlockType::Header);

  // update the Inst -> Phi mapping and the PhiLastUpdateMap
  void setPhiInfo(Instruction *I, Instruction *Phi,
                  InstInstMapType &InstToPhiMap,
                  BlockType BT = BlockType::Header);

  // set flag that the given instruction is an accum instruction
  void setIsAccumInst(Instruction *I, Instruction *AccumPhi,
                      BlockType BT = BlockType::Header);

  // get whether the instruction in block of type accumulation type
  bool getIsAccumInst(Instruction *I, BlockType BT = BlockType::Header);

  // get the constant addend from the given instruction
  Value *getConstAddend(Instruction *I);

  // get the non constant addend from the given instruction
  Value *getNonConstAddend(Instruction *I);

  // print the contents of all loop blocks
  void dumpLoopBlocks(std::string HeaderNote = "");

  // set ptrs to all BasicBlocks related to the loop
  bool setBasicBlocks();

  // check if the given two instruction are of similar in opcode / intrin id
  bool isSimilarType(Instruction *I1, Instruction *I1Override, Instruction *I2);

  // check if the given Usr is the next update of the instruction I
  bool checkInstUpdate(Instruction *I, Instruction *Usr, bool &IsInstUpdate,
                       Instruction *RefPhi = nullptr,
                       BlockType BT = BlockType::Header,
                       std::string Indent = "\t");

  // get the last update instruction in the chain with I as Alpha
  Instruction *getLastUpdate(Instruction *I, BlockType BT);

  // get the N'th update instruction in the chain with I as Alpha
  Instruction *getNthUpdate(Instruction *I, unsigned N, BlockType BT);

  // insert the Instruction I into the given BasicBlock at the insertion point
  bool insertIntoBlock(BlockType BT, Instruction *I, std::string DebugTag = "");

  // create/clear Instruction List Containers
  void resetInstContainers();

  void cleanup();

  // get the preheader clone of the given instruction
  Instruction *getPreheaderClone(Instruction *I);

  // create a mapping from given Load inst to it's next update chain in the
  // Header block
  bool collectLoadUpdates(Instruction *Alpha, Instruction *LastPivot,
                          std::string DebugTag = "\t");

  // create a mapping from given Load inst to it's next update chain in the
  // given Block
  bool collectLoadUpdates(Instruction *I, Instruction *RefPhi, BlockType BT,
                          std::string DebugTag = "\t");

  // create a mapping from given inst to it's next update chain
  bool collectInstUpdates(Instruction *I, Instruction *RefPhi = nullptr,
                          BlockType BT = BlockType::Header,
                          std::string Indent = "\t");

  // create a mappping from alpha family insts to their next updates
  bool collectAlphaUpdates(BlockType BT = BlockType::Header,
                           std::string Indent = "\t");

  // split the coords across loads and stores
  bool splitLoadStoreCoords(InstructionVecType &HeaderStoreList);

  // create pre-load and pre-store sequences for all load/store instructions
  // also mark if any sequence has Induction variable usage
  bool createLoadStoreSequences(InstructionVecType &HeaderStoreList,
                                InstructionVecType &HeaderLoadList);

  // create preheader clone of the given Instruction
  Instruction *createPreheaderCloneBase(Instruction *I,
                                        InstCloneMapType &CloneMap,
                                        bool BlockInsertFlag = true,
                                        std::string DebugTag = "");

  // create preheader clone of the given load instruction
  Instruction *createPreheaderLoadClone(Instruction *LoadInst,
                                        std::string DebugTag = "");

  // create preheader clone for the given instruction
  Instruction *createPreheaderClone(Instruction *I, std::string DebugTag = "");

  // patch the header load coord phi instruction with the last coord update
  // instruction in the preheader block
  void patchHeaderPhiWithPreheaderIncome(Instruction *LoadCoordInst,
                                         Instruction *PreLoadCoordInst,
                                         std::string DebugTag = "");

  // patch all the Header Load coord phis with the last coord update instruction
  // in the preheader block
  void patchHeaderPhisWithPreheaderIncome(std::string DebugTag = "");

  // create preheader clones
  void createPreheaderClones(std::string DebugTag = "");

  // modify the preheader of the loop by moving loads up
  void modifyPreheader();

  // create a new phi node in the header block with incoming values from
  // preheader and header
  Instruction *createHeaderLoadPhi(Instruction *LoadInst, int ClusterIdx,
                                   int LoadIdx, std::string DebugTag = "");

  // modify the loop body by re-arranging computes/stores/loads
  void modifyHeaderBlock();

  // collect all the lcssa phi insts
  void collectExitLCSSAPhis(InstructionSetType &LcssaPhis,
                            std::string DebugTag = "");

  // update the users of exit block's lcssa phis in the blocks outside exit
  void updateLcssaPhiUsers(InstructionSetType &LcssaPhis,
                           std::string DebugTag = "");

  // get the clone of the given instruction for the Block of type BT
  Instruction *getClone(Instruction *I, BlockType BT);

  // replace the definition of the operands of the given instruction with their
  // exit block clones
  void replaceDefsWithClones(Instruction *I, BlockType BT,
                             std::string DebugTag = "");

  // create a clone of the given instruction
  Instruction *createExitCloneBase(Instruction *I, InstCloneMapType &CloneMap,
                                   bool BlockInsertFlag = true,
                                   std::string DebugTag = "");

  // create a clone of the given load instruction, and it's coord if not cloned
  // yet
  Instruction *createExitLoadStoreInstClone(Instruction *I,
                                            std::string DebugTag = "");

  // create a clone of the given accumulator instruction
  Instruction *createExitAccumClone(Instruction *I, std::string DebugTag = "");

  // create a clone of the given instruction
  Instruction *createExitClone(Instruction *I, std::string DebugTag = "");

  // get the Exit block clone of the given instruction
  Instruction *getExitClone(Instruction *I);

  // create the load instructions for exit block
  void createExitLoadInsts(unsigned ClusterIdx, std::string DebugTag = "");

  // create the store/accum instructions for exit block
  void createExitOutInsts(unsigned ClusterIdx, std::string DebugTag = "");

  // create the compute instructions for exit block
  void createExitComputeInsts(unsigned ClusterIdx, std::string DebugTag = "");

  // modify the exit block by adding the final computes/stores
  void modifyExitBlock();

  // update the loop bounds by incrementing the induction value in the
  // preheader block
  void updateLoopBounds();

  // check if the WorkingLoop is a Prologue/Epilogue loop
  bool isPrologueOrEpilogueLoop();

  // check if the coord updates are not uniform
  bool checkUniformCoordUpdates(std::string Indent = "\t");

  // check if the there is at least one compute that is unclustered
  bool hasUnclusteredCompute(std::string Indent = "\t");

  // check the presence of stores and reduction
  bool checkRequirements(std::string Indent = "\t");

  // check if pipelining is enabled for the given loop
  bool isPipeliningEnabled();

  // check if pipelining can be applied on the given loop
  bool isPipeliningPossible();

  // check if pipelining on the given loop is profitable
  bool isPipeliningProfitable();

  // apply all basic block changes required to achieve software pipelining
  void applySoftwarePipelining();

  // print the original loop (input to this pass)
  void dumpInputLoop();

  // verify whether the whole loop nest is in LCSSA form after pipelining
  bool verifyLCSSAForm();

  // main function
  bool runOnLoop(Loop *Lp, LPPassManager &LPM) override;
};

char LoopSWP::ID = 0;

INITIALIZE_PASS_BEGIN(LoopSWP, PassName, PassDescription, false, false)
INITIALIZE_PASS_DEPENDENCY(LoopPass)
INITIALIZE_PASS_END(LoopSWP, PassName, PassDescription, false, false)

Pass *llvm::createLoopSWPPass() { return new LoopSWP(); }

Value *LoopSWP::getConstAddend(Instruction *I) {
  return TPCOptUtils::getAddend(I);
}

Value *LoopSWP::getNonConstAddend(Instruction *I) {
  return TPCOptUtils::getAddend(I, false);
}

// TODO : cluster info dep
void LoopSWP::resetInstContainers() {
  NumClusters = 0;
  IsAccumulationLoop = false;

  LoopClusters.clear();

  UnclusteredInstructions.clear();

  PreToHeaderPhiMap.clear();
  ExitToHeaderPhiMap.clear();

  for (unsigned i = 0; i < BlockType::NumBlocks; i++) {
    AccumPhis[i].clear();
    LoadToPhiMap[i].clear();
    AccumToPhiMap[i].clear();
    PhiLastUpdateMap[i].clear();
    InstCloneMap[i].clear();
    LoadToPhiMap[i].clear();
    InstToAlphaMap[i].clear();
    InstUpdateMap[i].clear();
  }

  InstSeqMap.clear();
  InstLoadStoreSeqMap.clear();

  InsertedBlockInsts.clear();
  InductionPhiSet.clear();
  LoopStructureInsts.clear();
  InductionInsts.clear();
}

void LoopSWP::cleanup() {
  for (unsigned i = 0; i < LoopClusters.size(); i++) {
    // free the load inst sequence objects
    for (auto LoadInst : LoopClusters[i]->HeaderLoadInstsVec) {
      auto It = InstLoadStoreSeqMap.find(LoadInst);
      if (It != InstLoadStoreSeqMap.end())
        delete It->second;
    }
    // free the store inst sequence objects
    for (auto StoreInst : LoopClusters[i]->HeaderStoreInstsVec) {
      auto It = InstLoadStoreSeqMap.find(StoreInst);
      if (It != InstLoadStoreSeqMap.end())
        delete It->second;
    }

    // free the cluster-info struct
    delete LoopClusters[i];
  }
  LoopClusters.clear();
}

void LoopSWP::setLoadPrefetchCount() {
  // use user-defined load count
  if (LoopSWPLoadPrefetchCount)
    LoadPrefetchCount = LoopSWPLoadPrefetchCount;
  else // set default
    LoadPrefetchCount = (getNumClusters() ? getNumClusters() : 0);

  // override
  // In case any CoordSeq of Load contains an Induction phi, when the CoordSeq
  // has to be peeled along with it's Pivot load, the Induction Phi should
  // also be cloned to Preheader block. Without decoupling the CoordSeq from
  // the actual Induction Phi, such cloning will be unsafe since it affects the
  // loop range.
  //
  // In such a case, restrict the PrefetchCount to UnrollFactor so that the
  // Induction Phi update remain a multiple of UnrollFactor.
  if (HasInductionInCoordSeq)
    LoadPrefetchCount = getNumClusters();

  LOOP_SWP_DEBUG("LoadPrefetchCount set to : " << LoadPrefetchCount)
}

bool LoopSWP::setBasicBlocks() {
  HeaderBlock = WorkingLoop->getHeader();

  PreheaderBlock = WorkingLoop->getLoopPreheader();
  if (!PreheaderBlock) {
    LOOP_SWP_DEBUG("Invalid Preheader Block, exiting ...")
    return false;
  }

  ExitBlock = WorkingLoop->getExitBlock();
  if (!ExitBlock) {
    LOOP_SWP_DEBUG("Invalid Exit Block, exiting ...")
    return false;
  }

  return true;
}

bool LoopSWP::isSimilarType(Instruction *I1, Instruction *I1Override,
                            Instruction *I2) {
  if (I1Override)
    I1 = I1Override;

  if (isa<PHINode>(I1)) {
    Instruction *RefPhi = I1;
    // if the Phi belongs to PreheaderBlock, we use it's HeaderBlock Phi
    // since we need the incoming instruction to the Header Phi as the
    // reference Inst for type match
    if (RefPhi->getParent() == PreheaderBlock) {
      auto It = PreToHeaderPhiMap.find(RefPhi);
      if (It == PreToHeaderPhiMap.end())
        return false;
      RefPhi = It->second;
    }
    I1 = cast<Instruction>(
        cast<PHINode>(RefPhi)->getIncomingValueForBlock(HeaderBlock));
  }
  if (isa<PHINode>(I2)) {
    Instruction *RefPhi = I2;
    if (RefPhi->getParent() == PreheaderBlock) {
      auto It = PreToHeaderPhiMap.find(RefPhi);
      if (It == PreToHeaderPhiMap.end())
        return false;
      RefPhi = It->second;
    }
    I2 = cast<Instruction>(
        cast<PHINode>(RefPhi)->getIncomingValueForBlock(HeaderBlock));
  }

  bool IsI1Intrin = false, IsI2Intrin = false;
  Intrinsic::ID I1ID, I2ID;
  if (auto Intrin = dyn_cast<IntrinsicInst>(I1)) {
    IsI1Intrin = true;
    I1ID = Intrin->getIntrinsicID();
  }
  if (auto Intrin = dyn_cast<IntrinsicInst>(I2)) {
    IsI2Intrin = true;
    I2ID = Intrin->getIntrinsicID();
  }

  if (IsI1Intrin != IsI2Intrin)
    return false;

  // both are intrinsics
  if (IsI1Intrin) {
    return (I1ID == I2ID);
  } else {
    return I1->getOpcode() == I2->getOpcode();
  }
}

// analyse whether the given user of the instruction is an update instruction
bool LoopSWP::checkInstUpdate(Instruction *I, Instruction *Usr,
                              bool &IsInstUpdate, Instruction *RefPhi,
                              BlockType BT, std::string Indent) {
  IsInstUpdate = false;

  std::string DebugTag = "<checkInstUpdate> " + Indent;
  LOOP_SWP_DEBUG(DebugTag << "[Begin]")
  LOOP_SWP_DEBUG(DebugTag << "I = " << *I)
  LOOP_SWP_DEBUG(DebugTag << "User = " << *Usr)

  // If the Def Inst 'I' is already mapped with it's next update, there is
  // nothing to do
  auto It = InstUpdateMap[BT].find(I);
  if (It != InstUpdateMap[BT].end() && It->second != nullptr) {
    LOOP_SWP_DEBUG(DebugTag << "Update of I is already tracked, exiting ...")
    LOOP_SWP_DEBUG(DebugTag << "[End]")
    return true;
  }

  // If the User Inst is already known as an update for an Inst, this shouldn't
  // have happened
  auto UserIt = InstUpdateMap[BT].find(Usr);
  if (UserIt != InstUpdateMap[BT].end()) {
    LOOP_SWP_DEBUG(DebugTag
                   << "User is already an update of another Inst, exiting ...")
    LOOP_SWP_DEBUG(DebugTag << "[End]")
    return true;
  }

  if (isSimilarType(I, RefPhi, Usr)) {
    // caution : insertelement user has must have I as the first operand to be
    // considered as an update inst
    if (isa<InsertElementInst>(Usr)) {
      if (I != cast<Instruction>(Usr->getOperand(0))) {
        LOOP_SWP_DEBUG(DebugTag
                       << "I is not the first operand of insertelement User; "
                          "hence not an update, exiting ...")
        LOOP_SWP_DEBUG(DebugTag << "[End]")
        return true;
      }
    }

    // at this point, we are certain that the Usr is indeed an update
    // instruction of I
    if (It == InstUpdateMap[BT].end())
      InstUpdateMap[BT].insert(std::make_pair(I, Usr));
    else
      It->second = Usr;

    // append a null tail for the update-chain in the map
    // (this is to make the check for whether an Inst is an update of other
    // simpler)
    InstUpdateMap[BT].insert(std::make_pair(Usr, nullptr));
    IsInstUpdate = true;
  }

  LOOP_SWP_DEBUG(DebugTag << "[End]")
  return true;
}

Instruction *LoopSWP::getLastUpdate(Instruction *I, BlockType BT) {
  if (isInductionPHI(I)) {
    auto LastIt = PhiLastUpdateMap[BT].find(I);
    assert(LastIt != PhiLastUpdateMap[BT].end() &&
           "Bug : last update not found for Induction Phi");
    return LastIt->second;
  }

  auto UpdateMap = InstUpdateMap[BT];
  Instruction *NextUpdate = nullptr;
  auto It = UpdateMap.find(I);
  while (It != UpdateMap.end() && It->second != nullptr) {
    NextUpdate = It->second;
    It = UpdateMap.find(NextUpdate);
  }
  return NextUpdate;
}

Instruction *LoopSWP::getNthUpdate(Instruction *I, unsigned N, BlockType BT) {
  if (N == getNumClusters())
    return getLastUpdate(I, BT);

  auto UpdateMap = InstUpdateMap[BT];
  Instruction *NextUpdate = nullptr;
  auto It = UpdateMap.find(I);
  while (N && It != UpdateMap.end() && It->second != nullptr) {
    N--;
    NextUpdate = It->second;
    It = UpdateMap.find(NextUpdate);
  }
  if (N)
    return nullptr;
  return NextUpdate;
}

bool LoopSWP::insertIntoBlock(BlockType BT, Instruction *I,
                              std::string DebugTag) {
  return TPCOptUtils::insertIntoBlock(WorkingLoop, BT, I, InsertedBlockInsts,
                                      BBInsertIt, BBPhiInsertIt, DebugTag);
}

// TODO: use a custom function and remove dependence on util function in
// the LoopUnroll.h (getUnrollMetadataForLoop())

// loop pragma condition check
static bool isPipelinePragmaDefined(Loop *WorkingLoop) {
  // pipeline pragma
  auto *PipePragmaEnable = TPCOptUtils::getUnrollMetadataForLoop(
      WorkingLoop, "llvm.loop.ir.pipeline");

  LOOP_SWP_DEBUG("llvm.loop.ir.pipeline = " << PipePragmaEnable)
  return PipePragmaEnable;
}

static bool isUnrollPragmaDefined(Loop *WorkingLoop) {
  // unroll pragma
  auto *UnrollPragmaEnable = TPCOptUtils::getUnrollMetadataForLoop(
      WorkingLoop, "llvm.loop.unroll.disable");
  LOOP_SWP_DEBUG("llvm.loop.unroll.disabled = " << UnrollPragmaEnable)
  return UnrollPragmaEnable;

  // unroll count pragma
  auto *UnrollPragmaCount = TPCOptUtils::getUnrollMetadataForLoop(
      WorkingLoop, "llvm.loop.unroll.count");
  LOOP_SWP_DEBUG("llvm.loop.unroll.count = " << UnrollPragmaCount)
  return UnrollPragmaCount;
}

// initialize the data needed for Induction Phi check
void LoopSWP::initAnalysisInfo() {
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  WorkingLoop->getInductionDescriptor(*SE, InductionDesc);
}

bool LoopSWP::isInductionPHI(PHINode *PhiNode) {
  BasicBlock *BB = PhiNode->getParent();
  if (BB == PreheaderBlock)
    PhiNode = cast<PHINode>(PreToHeaderPhiMap[PhiNode]);
  else if (BB == ExitBlock)
    PhiNode = cast<PHINode>(ExitToHeaderPhiMap[PhiNode]);

  auto It = InductionPhiSet.find(PhiNode);
  if (It != InductionPhiSet.end())
    return true;

  bool IsInductionPhi = InductionDescriptor::isInductionPHI(
      PhiNode, WorkingLoop, SE, InductionDesc);
  if (IsInductionPhi)
    InductionPhiSet.insert(PhiNode);

  return IsInductionPhi;
}

bool LoopSWP::isInductionPHI(Instruction *I) {
  if (auto *PhiNode = dyn_cast<PHINode>(I))
    return isInductionPHI(PhiNode);
  return false;
}

void LoopSWP::dumpPhiLastUpdateMap(BlockType BT) {
  LLVM_DEBUG(LOOP_SWP_DEBUG(
      "PhiLastUpdateMap : {") for (auto kvp
                                   : PhiLastUpdateMap[BT]){
      LOOP_SWP_DEBUG("\t" << kvp.first << " : " << kvp.second) LOOP_SWP_DEBUG(
          "\t" << *kvp.first << " : " << *kvp.second)} LOOP_SWP_DEBUG("}"));
  return;
}

void LoopSWP::setPhiInfo(Instruction *I, Instruction *Phi,
                         InstInstMapType &InstToPhiMap, BlockType BT) {
  if (InstToPhiMap.find(I) == InstToPhiMap.end())
    InstToPhiMap.insert(std::make_pair(I, Phi));
  else
    InstToPhiMap[I] = Phi;

  if (PhiLastUpdateMap[BT].find(Phi) == PhiLastUpdateMap[BT].end())
    PhiLastUpdateMap[BT].insert(std::make_pair(Phi, I));
  else
    PhiLastUpdateMap[BT][Phi] = I;

  dumpPhiLastUpdateMap(BT);
  return;
}

void LoopSWP::setIsAccumInst(Instruction *I, Instruction *AccumPhi,
                             BlockType BT) {
  setPhiInfo(I, AccumPhi, AccumToPhiMap[BT], BT);
  return;
}

bool LoopSWP::getIsAccumInst(Instruction *I, BlockType BT) {
  return (AccumToPhiMap[BT].find(I) != AccumToPhiMap[BT].end() ||
          AccumPhis[BT].find(I) != AccumPhis[BT].end());
}

bool LoopSWP::collectLoadUpdates(Instruction *I, Instruction *RefPhi,
                                 BlockType BT, std::string DebugTag) {
  /*
   * Populate the Update chain of the Alpha 'I' referring the RefPhi updates
   * and their clones.
   *
   * RefPhi is an Alpha with an identified chain of updates. The caller
   * shall be aware of the relation between 'I' and 'RefPhi'. Typically 'I'
   * is a clone of RefPhi in another block, so a one-one mapping can be
   * established between the chain of RefPhi and that of 'I'. This function
   * uses RefPhi's chain as reference to populate I's chain information, in
   * words, the RefPhi chain's clone.
   *
   * -------------------------------------------------
   * RefPhi -> U1          -> U2          -> ... -> Un
   * I      -> ?           -> ?              ...
   * -------------------------------------------------
   * I      -> U1.getClone -> U2.getClone -> ...
   * -------------------------------------------------
   */

  assert(BT != BlockType::Header &&
         "Illegal call to overloaded collectLoadUpdates");

  LOOP_SWP_DEBUG(DebugTag << "Collecting Load Updates for :")
  LOOP_SWP_DEBUG(DebugTag << "I = " << *I)
  LOOP_SWP_DEBUG(DebugTag << "RefPhi = " << *RefPhi)
  LOOP_SWP_DEBUG(DebugTag << "Block = " << TPCOptUtils::getBasicBlockStr(BT))
  DebugTag += "\t";

  // get the next update for RefPhi (= CurRefInst.update)
  auto RefUpdateIt = InstUpdateMap[BlockType::Header].find(RefPhi);
  auto CurInst = I;
  while (RefUpdateIt != InstUpdateMap[BlockType::Header].end()) {
    LOOP_SWP_DEBUG(DebugTag << "----")
    LOOP_SWP_DEBUG(DebugTag << "CurInst = " << *CurInst)
    // get the clone of the ref update inst
    if (!RefUpdateIt->second) {
      LOOP_SWP_DEBUG(DebugTag << "RefUpdate = nullptr")
      // cloning had stopped here
      InstUpdateMap[BT].insert(std::make_pair(CurInst, nullptr));
      break;
    }
    LOOP_SWP_DEBUG(DebugTag << "RefUpdate = " << *RefUpdateIt->second)

    auto CloneIt = InstCloneMap[BT].find(RefUpdateIt->second);
    if (CloneIt == InstCloneMap[BT].end()) {
      LOOP_SWP_DEBUG(DebugTag << "RefUpdate.getClone = nullptr")
      // cloning had stopped here
      InstUpdateMap[BT].insert(std::make_pair(CurInst, nullptr));
      break;
    } else {
      LOOP_SWP_DEBUG(DebugTag << "RefUpdate.getClone = " << *CloneIt->second)
      // CurInst.update = CurRefInst.update.getClone()
      InstUpdateMap[BT].insert(std::make_pair(CurInst, CloneIt->second));
      // advance
      RefUpdateIt = InstUpdateMap[BlockType::Header].find(RefUpdateIt->second);
      CurInst = CloneIt->second;
    }
  }

  return true;
}

bool LoopSWP::collectLoadUpdates(Instruction *Alpha, Instruction *LastPivot,
                                 std::string DebugTag) {
  LOOP_SWP_DEBUG(DebugTag << "Collecting Load updates for : ")
  LOOP_SWP_DEBUG(DebugTag << "Alpha = " << *Alpha)
  LOOP_SWP_DEBUG(DebugTag << "LastPivot = " << *LastPivot)

  auto It = InstSeqMap.find(Alpha);
  if (It == InstSeqMap.end()) {
    LOOP_SWP_DEBUG(DebugTag
                   << "Alpha does not belong to any InstSeq, exiting ...")
    return false;
  }

  InstSequence *AlphaSeq = It->second;
  Instruction *CurPivot = LastPivot;
  InstSequence *CurSeq = nullptr;

  LOOP_SWP_DEBUG(DebugTag << "AlphaSeq = " << AlphaSeq)
  LOOP_SWP_DEBUG(DebugTag << "AlphaSeq.isCoordSeq = "
                          << (AlphaSeq->isCoordSeq() ? "true" : "false"))
  LOOP_SWP_DEBUG(DebugTag << "CurPivot = " << *CurPivot)
  LOOP_SWP_DEBUG(DebugTag << "CurSeq = " << CurSeq)

  InstructionVecType UpdateStack;
  std::string DebugTagLoop = DebugTag + "\t";
  do {
    LOOP_SWP_DEBUG(DebugTag << "----")

    UpdateStack.push_back(CurPivot);
    auto CurLSIt = InstLoadStoreSeqMap.find(CurPivot);
    if (CurLSIt == InstLoadStoreSeqMap.end()) {
      LOOP_SWP_DEBUG(DebugTagLoop
                     << "LastPivot does not belong to any InstSeq, exiting ...")
      return false;
    }

    auto *CurLSSeq = CurLSIt->second;
    CurSeq = (AlphaSeq->isCoordSeq() ? CurLSSeq->getCoordSequence()
                                     : CurLSSeq->getIncomePredSequence());
    if (!CurSeq)
      continue;

    if (CurSeq->isDerived())
      CurPivot = CurSeq->getDerivedFrom();
    else
      CurPivot = nullptr;

    if (CurPivot)
      LOOP_SWP_DEBUG(DebugTagLoop << "CurPivot = " << *CurPivot)
    else
      LOOP_SWP_DEBUG(DebugTagLoop << "CurPivot = "
                                  << "nullptr")
    LOOP_SWP_DEBUG(DebugTagLoop << "CurSeq = " << CurSeq)
  } while (CurSeq && CurPivot && (CurSeq != AlphaSeq));

  if (CurSeq == AlphaSeq) {
    LOOP_SWP_DEBUG(DebugTag
                   << "CureSeq == AlphaSeq, creating the chain of updates ...")
    CurPivot = Alpha;
    LOOP_SWP_DEBUG(DebugTag << *Alpha)
    for (int i = UpdateStack.size() - 1; i >= 0; i--) {
      Instruction *I = UpdateStack[i];
      LOOP_SWP_DEBUG(DebugTag << "->")
      LOOP_SWP_DEBUG(DebugTag << *I)
      InstUpdateMap[BlockType::Header].insert(std::make_pair(CurPivot, I));
      InstToAlphaMap[BlockType::Header].insert(std::make_pair(I, Alpha));
      CurPivot = I;
    }
    // add an entry (LastInst -> nullptr) if LastInst exists
    if (UpdateStack.size()) {
      LOOP_SWP_DEBUG(DebugTag << "->")
      LOOP_SWP_DEBUG(DebugTag << "nullptr")
      InstUpdateMap[BlockType::Header].insert(
          std::make_pair(UpdateStack[0], nullptr));
    }
  }
  return true;
}

bool LoopSWP::collectInstUpdates(Instruction *I, Instruction *RefPhi,
                                 BlockType BT, std::string Indent) {
  std::string DebugTag = "<collectInstUpdates> " + Indent;
  LOOP_SWP_DEBUG(DebugTag << "Collecting updates for :")
  LOOP_SWP_DEBUG(DebugTag << "I = " << *I)

  if (RefPhi) {
    if (!isa<PHINode>(RefPhi) || RefPhi->getParent() != HeaderBlock) {
      LOOP_SWP_DEBUG(DebugTag << "RefPhi not a Header Phi, exiting ...")
      return false;
    }
  }

  auto It = InstSeqMap.find(I);
  // if I is not related to coord or income-pred sequence, continue
  if (It == InstSeqMap.end()) {
    LOOP_SWP_DEBUG(DebugTag
                   << "Does not belong to any InstSeq, nothing to do ...")
    return true;
  }

  BasicBlock *BB = TPCOptUtils::getLoopBlock(WorkingLoop, BT);

  Instruction *CurInst = I;
  bool UpdateFound = true;
  while (UpdateFound) {
    UpdateFound = false;
    LOOP_SWP_DEBUG(DebugTag << "CurInst = " << *CurInst)
    Indent += "\t";
    DebugTag += "\t";
    for (auto User : CurInst->users()) {
      if (isa<PHINode>(User))
        continue;
      Instruction *UserInst = cast<Instruction>(User);
      LOOP_SWP_DEBUG(DebugTag << "UserInst = " << *UserInst)
      if (UserInst->getParent() != BB) {
        LOOP_SWP_DEBUG(DebugTag << "Does not belong to BB, continuing ...")
        continue;
      }
      if (!checkInstUpdate(CurInst, UserInst, UpdateFound, RefPhi, BT,
                           Indent + "\t")) {
        LOOP_SWP_DEBUG(DebugTag << "Inst update check failed, exiting ...")
        return false;
      }
      if (UpdateFound) {
        LOOP_SWP_DEBUG(DebugTag << "This user Inst is an update.")
        InstToAlphaMap[BT].insert(std::make_pair(CurInst, I));
        CurInst = UserInst;
        break;
      }
    }
  }

  // last update map
  setPhiInfo(CurInst, I, InstToAlphaMap[BT], BT);

  return true;
}

bool LoopSWP::collectAlphaUpdates(BlockType BT, std::string Indent) {
  std::string DebugTag = "<collectAlphaUpdates> " + Indent;

  assert(BT < BlockType::NumBlocks && "Invalid BlockType");

  LOOP_SWP_DEBUG(DebugTag << "Collecting Alpha updates for BB : "
                          << TPCOptUtils::getBasicBlockStr(BT))
  // if any alpha inst is not a phi, collect it's updates too
  for (auto &HeaderPhiNode : HeaderBlock->phis()) {
    LOOP_SWP_DEBUG(DebugTag << "----")
    Instruction *HeaderPhi = cast<Instruction>(&HeaderPhiNode);
    LOOP_SWP_DEBUG(DebugTag << "HeaderPhi = " << *HeaderPhi)

    auto It = InstSeqMap.find(HeaderPhi);
    if (It == InstSeqMap.end()) {
      LOOP_SWP_DEBUG(DebugTag
                     << "Does not belong to any InstSeq, continuing ...")
      continue;
    } else {
      // skip store coord seq for Preheader block
      if (BT == BlockType::Preheader && !It->second->isLoadPivot()) {
        LOOP_SWP_DEBUG(DebugTag << "Store CoordSeq not currently being cloned "
                                   "in Preheader block, continuing")
        continue;
      }
    }

    Instruction *Phi;
    if (BT == BlockType::Header) {
      Phi = HeaderPhi;
    } else {
      auto CloneIt = InstCloneMap[BT].find(HeaderPhi);
      if (CloneIt == InstCloneMap[BT].end()) {
        LOOP_SWP_DEBUG(DebugTag
                       << "HeaderPhi's clone not found, continuing ...")
        continue;
      }
      Phi = CloneIt->second;
    }

    LOOP_SWP_DEBUG(DebugTag << "Phi = " << *Phi)
    if (isInductionPHI(&HeaderPhiNode)) {
      LOOP_SWP_DEBUG(DebugTag << "InductionPhi, continuing ...")
      continue;
    }

    // handle the load Alphas of Header block
    Instruction *Income =
        cast<Instruction>(HeaderPhiNode.getIncomingValueForBlock(HeaderBlock));
    if (TPCOptUtils::isLoadTensor(Income)) {
      if (BT == BlockType::Header) {
        LOOP_SWP_DEBUG(DebugTag << "Header Load Alpha")
        if (!collectLoadUpdates(Phi, Income, DebugTag)) {
          LOOP_SWP_DEBUG(DebugTag
                         << "Load update collection failed, exiting ...")
          return false;
        }
      } else {
        LOOP_SWP_DEBUG(DebugTag << "Load Alpha")
        if (!collectLoadUpdates(Phi, HeaderPhi, BT, DebugTag)) {
          LOOP_SWP_DEBUG(DebugTag
                         << "Load update collection failed, exiting ...")
          return false;
        }
      }
    } else {
      LOOP_SWP_DEBUG(DebugTag << "Non Load Alpha")
      Instruction *RefPhi = (BT == BlockType::Header) ? nullptr : HeaderPhi;
      // collect the chain of updates
      if (!collectInstUpdates(Phi, RefPhi, BT, Indent)) {
        LOOP_SWP_DEBUG(DebugTag << "Inst update collection failed, exiting ...")
        return false;
      }
    }
  }

  LOOP_SWP_DEBUG(DebugTag << "[END]")
  return true;
}

void LoopSWP::dumpLoopBlocks(std::string HeaderNote) {
  LLVM_DEBUG(LOOP_SWP_DEBUG("Loop Blocks : " << HeaderNote)
                 LOOP_SWP_DEBUG(*PreheaderBlock << "\n----");
             LOOP_SWP_DEBUG(*HeaderBlock << "\n----");
             LOOP_SWP_DEBUG(*ExitBlock << "\n----"););
}

bool LoopSWP::isPrologueOrEpilogueLoop() {
  std::string DebugTag = "<isPrologueOrEpilogueLoop> ";
  std::string NotPELoopStr = DebugTag + "Not a prologue/epilogue loop : ";

  Loop *ParentLoop = WorkingLoop->getParentLoop();
  // 1. if this is the only loop in this depth, its not a prologue loop
  if (ParentLoop->getSubLoops().size() == 1) {
    LOOP_SWP_DEBUG(NotPELoopStr << "This is the only innermost loop")
    return false;
  }

  // 2. Get the exit condition
  auto *PreheaderPred = PreheaderBlock->getUniquePredecessor();
  if (!PreheaderPred) {
    LOOP_SWP_DEBUG(NotPELoopStr << "Preheader does not have unique predecessor")
    return false;
  }
  auto InstIt = PreheaderPred->end();
  InstIt--;
  Instruction *PredBranchInst = &(*InstIt);
  auto *BI = dyn_cast<BranchInst>(PredBranchInst);
  assert(BI && "Expected : A branch instruction");
  if (BI->isUnconditional()) {
    LOOP_SWP_DEBUG(NotPELoopStr << "Expected conditional branching")
    return false;
  }
  auto *BranchCondInst = dyn_cast<ICmpInst>(BI->getCondition());
  if (!BranchCondInst) {
    LOOP_SWP_DEBUG(NotPELoopStr << "Expected a ICmpInst")
    return false;
  }
  if (!BranchCondInst->isEquality()) {
    LOOP_SWP_DEBUG(NotPELoopStr << "Expected an equality check with 0")
    return false;
  }
  Value *Op[2] = {BranchCondInst->getOperand(0), BranchCondInst->getOperand(1)};
  int ConstOp;
  auto *ConstVal0 = dyn_cast<Constant>(Op[0]);
  auto *ConstVal1 = dyn_cast<Constant>(Op[1]);
  if (ConstVal0 && ConstVal0->isZeroValue()) {
    ConstOp = 0;
  } else if (ConstVal1 && ConstVal1->isZeroValue()) {
    ConstOp = 1;
  } else {
    LOOP_SWP_DEBUG(NotPELoopStr << "Expected an equality check with 0")
    return false;
  }
  // check for 'xtraiter' value
  Instruction *XtraIter = cast<Instruction>(Op[(ConstOp + 1) & 1]);
  if (XtraIter->getOpcode() != Instruction::And &&
      XtraIter->getOpcode() != Instruction::URem) {
    LOOP_SWP_DEBUG(NotPELoopStr << "Expected an And / URem instruction")
    return false;
  }

  // 3. Does the name contain 'prol'/'epil'?
  bool HasProlSubstr =
      (HeaderBlock->getName().find("prol") != std::string::npos);
  LOOP_SWP_DEBUG(DebugTag << "Prologue ('prol') Substring "
                          << (HasProlSubstr ? "present" : "not present"))
  bool HasEpilSubstr =
      (HeaderBlock->getName().find("epil") != std::string::npos);
  LOOP_SWP_DEBUG(DebugTag << "Epilogue ('epil') Substring "
                          << (HasEpilSubstr ? "present" : "not present"))

  // 4. Does the Loop have constant non-zero Max Trip Count?
  auto *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  int MaxTripCount = SE->getSmallConstantMaxTripCount(WorkingLoop);
  LOOP_SWP_DEBUG(DebugTag << "MaxTripCount = " << MaxTripCount)
  bool HasFiniteNonZeroTripCount = (MaxTripCount != 0);

  if (!((HasProlSubstr || HasEpilSubstr) && HasFiniteNonZeroTripCount)) {
    LOOP_SWP_DEBUG(NotPELoopStr
                   << "BB name should have 'prol'/'epil' suffix AND")
    LOOP_SWP_DEBUG(NotPELoopStr
                   << "the loop must have finite non-zero trip count")
    return false;
  }

  return true;
}

// This function checks for uniformity in the coord update instruction chain
// The update chain is considered non-uniform if :
// - no coord update instruction is found in the block
// - any add.mask does not have a constant step value
// - all add.mask instructions in the block do not have the same constant step
//   value type (all ConstantInt or all ConstantDataVector)
// - all add.mask instructions in the block do not have the same constant step
//   value
bool LoopSWP::checkUniformCoordUpdates(std::string Indent) {
  std::string DebugTag = BAIL_OUT_TAG + Indent;
  Type *T5xi32 =
      FixedVectorType::get(Type::getInt32Ty(HeaderBlock->getContext()), 5);
  int MaxIntVal = 0x7fffffff;

  // for each Coord phi node
  for (auto &PhiNode : HeaderBlock->phis()) {
    auto *HeaderIncomingValue = PhiNode.getIncomingValueForBlock(HeaderBlock);
    if (HeaderIncomingValue->getType() != T5xi32)
      continue;

    // collect step vals into a vector
    llvm::SmallVector<Value *, 4> StepVals;
    auto UpdateMap = InstUpdateMap[BlockType::Header];
    Instruction *NextUpdate = cast<Instruction>(&PhiNode);
    LOOP_SWP_DEBUG("<5xi32> PhiNode = " << *NextUpdate)
    auto It = UpdateMap.find(NextUpdate);
    while (It != UpdateMap.end() && It->second != nullptr) {
      NextUpdate = It->second;
      LOOP_SWP_DEBUG("\tNextUpdate = " << *NextUpdate)
      Value *StepVal = TPCOptUtils::getStepValue(NextUpdate);
      if (!StepVal) {
        LOOP_SWP_DEBUG(
            DebugTag
            << "Invalid step val detected for Coord update, exiting ...")
        return false;
      }
      LOOP_SWP_DEBUG("\t\tStepVal = " << *StepVal)
      StepVals.push_back(StepVal);
      It = UpdateMap.find(NextUpdate);
    }
    if (!StepVals.size()) {
      LOOP_SWP_DEBUG(DebugTag << "No Coord update found, exiting ...")
      return false;
    }

    // analyze the step val vector
    auto *ConstStepVal = dyn_cast<ConstantInt>(StepVals[0]); // i32
    auto *ConstVecStepVal =
        dyn_cast<ConstantDataVector>(StepVals[0]); // 5 x i32
    if (TPCOptUtils::isIntrinsicOfType(
            dyn_cast<Instruction>(HeaderIncomingValue),
            Intrinsic::tpc_add_mask)) {
      if (!ConstStepVal && !ConstVecStepVal) {
        LOOP_SWP_DEBUG(DebugTag
                       << "Non-const step val used in add.mask, exiting ...")
        return false;
      }
    }
    int ElementVal = MaxIntVal;
    if (ConstStepVal)
      ElementVal = ConstStepVal->getSExtValue();
    else if (ConstVecStepVal)
      ElementVal = dyn_cast<ConstantInt>(ConstVecStepVal->getSplatValue())
                       ->getSExtValue();
    else
      continue;

    for (unsigned i = 1; i < StepVals.size(); i++) {
      // updates must be either all constants / all non-constants
      auto *CurConstStepVal = dyn_cast<ConstantInt>(StepVals[i]);
      auto *CurConstVecStepVal = dyn_cast<ConstantDataVector>(StepVals[i]);
      if (((ConstStepVal == nullptr) ^ (CurConstStepVal == nullptr)) ||
          ((ConstVecStepVal == nullptr) ^ (CurConstVecStepVal == nullptr))) {
        LOOP_SWP_DEBUG(DebugTag << "Non-uniform Coord update (#" << i
                                << ") step value type, exiting ...")
        return false;
      }

      int CurElementVal = MaxIntVal;
      if (ConstStepVal)
        CurElementVal = CurConstStepVal->getSExtValue();
      else if (ConstVecStepVal)
        CurElementVal =
            dyn_cast<ConstantInt>(CurConstVecStepVal->getSplatValue())
                ->getSExtValue();

      if (ElementVal != CurElementVal) {
        LOOP_SWP_DEBUG(DebugTag << "Non-uniform Coord update (#" << i
                                << ") step value, exiting ...")
        return false;
      }
    }
  }

  return true;
}

// check if there are any compute instructions outside the clusters and
// InstSequences
bool LoopSWP::hasUnclusteredCompute(std::string Indent) {
  std::string DebugTag = BAIL_OUT_TAG + Indent;

  for (auto I : UnclusteredInstructions) {
    // skip instructions that belong to loop structure or any InstSequence
    if (LoopStructureInsts.find(I) != LoopStructureInsts.end() ||
        (InstSeqMap.find(I) != InstSeqMap.end()))
      continue;
    // skip trailing add.mask instructions
    if (TPCOptUtils::isIntrinsicOfType(I, Intrinsic::tpc_add_mask)) {
      unsigned NumUsersWithinBlock = 0;
      bool IsBadInst = false;
      for (auto User : I->users()) {
        Instruction *UserInst = cast<Instruction>(User);
        if (UserInst->getParent() != HeaderBlock)
          continue;
        NumUsersWithinBlock += 1;
        if (!isa<PHINode>(UserInst) || NumUsersWithinBlock > 1) {
          IsBadInst = true;
          break;
        }
      }
      if (!IsBadInst)
        continue;
    }
    LOOP_SWP_DEBUG(DebugTag
                   << "Found instruction not in any cluster or InstSeq :")
    LOOP_SWP_DEBUG(DebugTag << "I = " << *I)
    return true;
  }
  return false;
}

// This function checks for misc. bail out requirements like :
// - the loop is not manually pipelined
// - the loop does not contain both an accumulator and an st_tnsr
bool LoopSWP::checkRequirements(std::string Indent) {
  std::string DebugTag = BAIL_OUT_TAG + Indent;
  Type *T5xi32 =
      FixedVectorType::get(Type::getInt32Ty(HeaderBlock->getContext()), 5);

  bool HasStoreTensor = false;
  for (Instruction &Inst : *HeaderBlock) {
    Instruction *I = &Inst;

    // try and detect pipelining
    if (TPCOptUtils::isLoadTensor(I)) {
      // if this loop is already pipelined, there should be at least one ld_tnsr
      // with zero non-PHI users
      unsigned NonPHIUserCount = 0;
      for (auto User : I->users()) {
        Instruction *UserInst = cast<Instruction>(User);
        NonPHIUserCount +=
            ((UserInst->getParent() == HeaderBlock) && !isa<PHINode>(UserInst));
      }
      if (!NonPHIUserCount) {
        LOOP_SWP_DEBUG(DebugTag << "Load : " << *I)
        LOOP_SWP_DEBUG(
            DebugTag << "Found 'ld_tnsr' with no non-PHI user within the block")
        LOOP_SWP_DEBUG(DebugTag << "- Possibly pipelined already, exiting ...")
        return false;
      }
      continue;
    }

    // skip phis, Coords
    if (isa<PHINode>(I) || I->getType() == T5xi32)
      continue;

    HasStoreTensor |= TPCOptUtils::isStoreTensor(I);

    // if there is a st_tnsr along with an accumulator, skip for now
    unsigned ExternalUserCount = 0;
    for (auto User : I->users()) {
      Instruction *UserInst = cast<Instruction>(User);
      ExternalUserCount += (UserInst->getParent() != HeaderBlock);
    }
    if (ExternalUserCount && HasStoreTensor) {
      LOOP_SWP_DEBUG(
          DebugTag << "Detected presence of both Store and Accum, exiting ...")
      LOOP_SWP_DEBUG("I = " << *I)
      return false;
    }
  }

  return true;
}

bool LoopSWP::isPipeliningPossible() {
  std::string DebugTag = BAIL_OUT_TAG;
  // The loop should be the innermost loop.
  if (WorkingLoop->getSubLoops().size()) {
    LOOP_SWP_DEBUG(DebugTag << "Cannot apply software pipelining for "
                               "non-innermost loop, exiting ...")
    return false;
  }

  // set the preheader, header and exit basic blocks of the Working Loop
  setBasicBlocks();

  // Identify the prologue/epilogue loop by looking for marker set by unroll
  if (findStringMetadataForLoop(WorkingLoop,
                                "llvm.loop.unroll.remainderloop.marker")) {
    LOOP_SWP_DEBUG(DebugTag
                   << "Possibly prologue loop; won't pipeline, exiting ...")
    return false;
  }

  DebugTag += "[InnerLoop] : ";
  // The loop should have no branching
  if (WorkingLoop->getNumBlocks() > 1) {
    LOOP_SWP_DEBUG(DebugTag
                   << "Branching detected within the loop, exiting ...")
    return false;
  }

  LOOP_SWP_DEBUG("PreheaderBlock = " << PreheaderBlock->getName())
  LOOP_SWP_DEBUG("HeaderBlock = " << HeaderBlock->getName())
  LOOP_SWP_DEBUG("ExitBlock = " << ExitBlock->getName())

  // clear the data
  resetInstContainers();

  if (!TPCOptUtils::populateInductionInsts(WorkingLoop, SE, InductionInsts)) {
    LOOP_SWP_DEBUG(DebugTag
                   << "Could not populate Induction Instructions, exiting ...")
    return false;
  }

  if (!TPCOptUtils::populateLoopStructInsts(WorkingLoop, LoopStructureInsts)) {
    LOOP_SWP_DEBUG(
        DebugTag << "Could not populate loop struct instructions, exiting ...")
    return false;
  }

  // check the presence of stores and reduction
  if (!checkRequirements()) {
    LOOP_SWP_DEBUG(DebugTag
                   << "Stores in reduction loop not handled yet, exiting ...")
    return false;
  }

  // HeaderStoreList and HeaderLoadList are passed as inputs to clusterizer
  // since they are later required for Inst Sequence creation
  InstructionVecType HeaderStoreList;
  InstructionVecType HeaderLoadList;

  // Since SWP bails out in the following cases, instructing clusterizer to
  // bail out / skip if the conditions are met can avoid unnecessary
  // processing.
  //
  // Instructs the clusterizer not to populate the cluster info vector if a
  // single cluster was found.
  bool PopulateIfMultiCluster = true;
  // Instructs the clusterizer to exit if no 'ld_tnsr' instruction was found
  bool SkipIfZeroLoadTensors = true;

  // create clusterizer instance
  IterationClusterizer SWPIterClusterizer(
      WorkingLoop, SE, PopulateIfMultiCluster, SkipIfZeroLoadTensors);
  // classify the loop instructions and collect cluster info
  if (!SWPIterClusterizer.classifyAndClusterize(
          LoopClusters, HeaderStoreList, HeaderLoadList,
          UnclusteredInstructions, AccumPhis[BlockType::Header],
          AccumToPhiMap[BlockType::Header])) {
    LOOP_SWP_DEBUG(DebugTag << "Loop Clusterization failed, exiting ...")
    return false;
  }

  setNumClusters(SWPIterClusterizer.getNumClusters());
  setIsAccumulationLoop(SWPIterClusterizer.getIsAccumulationLoop());

  assert(LoopClusters.size() == getNumClusters() &&
         "Fatal : Inconsistent cluster info");
  for (unsigned i = 0; i < LoopClusters.size(); i++) {
    assert(LoopClusters[i] && "Fatal : cluster info not found");
    assert(LoopClusters[i]->HeaderLoadInstsVec.size() &&
           "Expected 'ld_tnsr' Instruction not found");
  }

  // Create Load/Store Inst sequences
  if (!createLoadStoreSequences(HeaderStoreList, HeaderLoadList)) {
    LOOP_SWP_DEBUG(DebugTag
                   << "Could not create Load/Store Sequence, exiting ...")
    return false;
  }

  // check if there are unclustered compute instructions outside DAG
  if (hasUnclusteredCompute()) {
    LOOP_SWP_DEBUG(DebugTag << "Found One or more unclustered instructions "
                               "that is/are possibly compute, exiting ...")
    return false;
  }

  if (HeaderStoreList.size()) {
    // check if the load coord and store coords collide, if yes split all of
    // them
    if (!splitLoadStoreCoords(HeaderStoreList)) {
      LOOP_SWP_DEBUG(DebugTag
                     << "Splitting Load/Store Coords failed, exiting ...")
      return false;
    }
    dumpLoopBlocks("Loop Blocks (After store split) :");
  }

  if (!collectAlphaUpdates(BlockType::Header)) {
    LOOP_SWP_DEBUG(DebugTag << "Phi update collection failed, exiting ...")
    return false;
  }

  unsigned N = getNumClusters();

  // check if the coord updates are not uniform
  if (!checkUniformCoordUpdates()) {
    LOOP_SWP_DEBUG(DebugTag << "Non-uniform Coord access found, exiting ...")
    return false;
  }

  // ensure all required clones are available
  // TODO

  // ensure all updates are available
  // TODO

  // set the pipelining load count based on user requirement or number of
  // clusters
  setLoadPrefetchCount();

  if (!getLoadPrefetchCount() || getLoadPrefetchCount() > N) {
    LOOP_SWP_DEBUG(DebugTag << "Invalid Load Prefetch count "
                            << LoadPrefetchCount << " (#Clusters = " << N
                            << ")")
    return false;
  }

  return true;
}

// TODO
bool LoopSWP::isPipeliningProfitable() { return true; }

void LoopSWP::applySoftwarePipelining() {
  // edit preheader
  modifyPreheader();
  dumpLoopBlocks("(After Preheader Block Modification)");

  // edit loop body
  modifyHeaderBlock();
  dumpLoopBlocks("(After Header Block Modification)");

  // update loop bounds
  updateLoopBounds();
  dumpLoopBlocks("(After Updating Loop Bounds)");

  // edit cleanup
  modifyExitBlock();
  dumpLoopBlocks("(After Exit Block Modification)");
}

void LoopSWP::dumpInputLoop() {
  LOOP_SWP_DEBUG("\n========\nPipelining for input loop : "
                 << WorkingLoop->getName() << "\n========\n")
  LLVM_DEBUG({
    for (auto *BB : WorkingLoop->getBlocksVector()) {
      LOOP_SWP_DEBUG(BB->getName())
      for (Instruction &I : *BB)
        LOOP_SWP_DEBUG("\t" << I)
      LOOP_SWP_DEBUG("")
    }
  });
  LOOP_SWP_DEBUG("\n========\n")
}

bool LoopSWP::verifyLCSSAForm() {
  auto *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  auto *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

  Loop *OutermostLoop = WorkingLoop;
  while (OutermostLoop->getParentLoop())
    OutermostLoop = OutermostLoop->getParentLoop();

  return OutermostLoop->isRecursivelyLCSSAForm(*DT, *LI);
}

bool LoopSWP::isPipeliningEnabled() {
  // if (!(flag && unroll_pragma) && !(pipeline_pragma))
  //   exit
  LOOP_SWP_DEBUG("SWP Enable Flag = " << LoopSWPFlag)
  return ((LoopSWPFlag && isUnrollPragmaDefined(WorkingLoop)) ||
          isPipelinePragmaDefined(WorkingLoop));
}

bool LoopSWP::runOnLoop(Loop *L, LPPassManager &LPM) {
  WorkingLoop = L;

  if (!isPipeliningEnabled()) {
    std::string DebugTag = DISABLED_TAG;
    if (!WorkingLoop->getSubLoops().size())
      DebugTag += "[InnerLoop] : ";
    LOOP_SWP_DEBUG(DebugTag
                   << "SW Pipelining is not enabled for this loop; not applied")
    return false;
  }

  initAnalysisInfo();

  dumpInputLoop();

  TPCOptUtils::dumpCFG(WorkingLoop);

  if (!isPipeliningPossible()) {
    cleanup();
    return false;
  }

  if (!isPipeliningProfitable()) {
    cleanup();
    return false;
  }

  applySoftwarePipelining();

  assert(verifyLCSSAForm() && "LCSSA Form is broken!");

  dumpLoopBlocks("Loop Blocks (Final) :");

  std::string SuccessTag = SUCCESS_TAG;
  LOOP_SWP_DEBUG(SuccessTag << "Applied SWP on loop.")

  cleanup();

  return false;
}

bool LoopSWP::splitLoadStoreCoords(InstructionVecType &HeaderStoreList) {
  std::string DebugTag = "<splitLoadStoreCoords> ";
  LOOP_SWP_DEBUG(DebugTag << "Splitting Store Coord sequences")

  // this will be just used as a filler argument to clone method
  BBInsertIt = HeaderBlock->begin();

  for (auto I : HeaderStoreList) {
    LOOP_SWP_DEBUG(DebugTag << "-----")
    LOOP_SWP_DEBUG(DebugTag << "Store : " << *I)
    auto StoreSeq = InstLoadStoreSeqMap[I];
    auto *StoreCoordSeq = StoreSeq->getCoordSequence();

    if (StoreCoordSeq->isDerivedFromLoad()) {
      LOOP_SWP_DEBUG(DebugTag << "Splitting store coords ...")
      // In case this sequence derives from a LoadSequence through an Induction
      // Instruction
      Instruction *RefInst = StoreCoordSeq->derivesOnlyInduction();
      if (!RefInst)
        RefInst = StoreCoordSeq->getRefInst();
      LOOP_SWP_DEBUG(
          DebugTag
          << "Reference Instruction of the Sequence which this derives from : ")
      LOOP_SWP_DEBUG(DebugTag << *RefInst)

      InstSequence *DerivedFrom = InstSeqMap[RefInst];
      if (!StoreCoordSeq->clone(DerivedFrom, InstCloneMap[BlockType::Header],
                                InsertedBlockInsts, BBInsertIt, false,
                                BlockType::Header)) {
        LOOP_SWP_DEBUG(DebugTag << "Store Coord seq object failed, exiting ...")
        return false;
      }
      LOOP_SWP_DEBUG(DebugTag << "After Split :")
      StoreCoordSeq->debugDump();
    }
  }

  // Fix phis
  LOOP_SWP_DEBUG(DebugTag << "Fixing InstSeq Phis ... [Begin]")
  for (auto &PhiNode : HeaderBlock->phis()) {
    Instruction *Phi = cast<Instruction>(&PhiNode);
    LOOP_SWP_DEBUG(DebugTag << "----")
    LOOP_SWP_DEBUG(DebugTag << "Phi = " << *Phi)
    // not interested in phis outside Coord / Income-Pred sequences
    if (InstSeqMap.find(Phi) == InstSeqMap.end())
      continue;
    // pick only the load coord phis
    // if a clone is found for the phi, this is a load coord phi
    if (InstCloneMap[BlockType::Header].find(Phi) ==
        InstCloneMap[BlockType::Header].end())
      continue;
    Instruction *PhiIncome =
        cast<Instruction>(PhiNode.getIncomingValueForBlock(HeaderBlock));

    // get Store clone for Load Phi
    auto PhiIt = InstCloneMap[BlockType::Header].find(Phi);
    if (PhiIt == InstCloneMap[BlockType::Header].end()) {
      LOOP_SWP_DEBUG(DebugTag
                     << "Improper split with store coord phi, exiting ...")
      return false;
    }
    auto StorePhiNode = cast<PHINode>(PhiIt->second);

    auto It = InstSeqMap.find(PhiIncome);
    if (It == InstSeqMap.end()) {
      // if the incoming value from Header to this Phi does not belong to any
      // InstSeq object, the only known reasons are that it has to be a trailing
      // add.mask instruction or an induction phi update
      bool IsInductionPHI = isInductionPHI(&PhiNode);
      bool IsAddMask =
          TPCOptUtils::isIntrinsicOfType(PhiIncome, Intrinsic::tpc_add_mask);
      if (!(IsInductionPHI || IsAddMask)) {
        LOOP_SWP_DEBUG(DebugTag
                       << "Unexpected trailing phi incoming value, exiting ...")
        return false;
      }
      // clone this add.mask / induction update to achieve the final split step
      Instruction *InstClone = PhiIncome->clone();
      Instruction *PrevUpdate = cast<Instruction>(getNonConstAddend(PhiIncome));
      auto It = InstCloneMap[BlockType::Header].find(PrevUpdate);
      if (It == InstCloneMap[BlockType::Header].end()) {
        LOOP_SWP_DEBUG(
            DebugTag
            << "Improper split with penultimate trailing update, exiting ...")
        return false;
      }
      // replace uses of prev-update with it's clone (a split for store insts)
      InstClone->replaceUsesOfWith(PrevUpdate, It->second);
      // update the incoming value for the store phi (clone phi) with the new
      // inst clone
      StorePhiNode->removeIncomingValue(HeaderBlock, false);
      StorePhiNode->addIncoming(InstClone, HeaderBlock);
      // insert into block
      BBInsertIt = PhiIncome->getIterator();
      if (!insertIntoBlock(BlockType::Header, InstClone, DebugTag)) {
        INSTSEQ_DEBUG(DebugTag
                      << "Final add.mask split insertion failed, exiting ...")
        return false;
      }

      InstCloneMap[BlockType::Header].insert(
          std::make_pair(PhiIncome, InstClone));
    } else {
      // An initial check has ensured only Coord seq objects are considered

      // The last split that happened as a result of cloning a load CoordSeq
      // might be affecting the Header income for the Phi's clone
      // This income must have a clone
      auto CloneIt = InstCloneMap[BlockType::Header].find(PhiIncome);
      if (CloneIt == InstCloneMap[BlockType::Header].end()) {
        LOOP_SWP_DEBUG(DebugTag << "Improper split from load coord")
        return false;
      }
      // update the incoming value for the store phi (clone phi) with the new
      // inst clone
      StorePhiNode->removeIncomingValue(HeaderBlock, false);
      StorePhiNode->addIncoming(CloneIt->second, HeaderBlock);
    }
  }
  LOOP_SWP_DEBUG(DebugTag << "Fixing InstSeq Phis ... [End]")

  return true;
}

bool LoopSWP::createLoadStoreSequences(InstructionVecType &HeaderStoreList,
                                       InstructionVecType &HeaderLoadList) {
  std::string DebugTag = "<createLoadStoreSequences> ";
  LOOP_SWP_DEBUG(DebugTag << "Initializing Pre-Load Sequences ...")
  for (auto I : HeaderLoadList) {
    LOOP_SWP_DEBUG(DebugTag << "Load : " << *I)

    auto LoadSeq = new LoadStoreSequence(I, WorkingLoop, BlockType::Header,
                                         InstSeqMap, InductionInsts);
    InstLoadStoreSeqMap.insert(std::make_pair(I, LoadSeq));

    if (!LoadSeq->populateSequence(InstUpdateMap[BlockType::Header])) {
      LOOP_SWP_DEBUG(DebugTag
                     << "LoadSequence construction failed, exiting ...")
      return false;
    }
    if (LoadSeq->getIncomePredSequence())
      setHasLoadIncome();
    LoadSeq->debugDump();
  }

  LOOP_SWP_DEBUG(DebugTag << "Initializing Pre-Store Sequences ...")
  for (auto I : HeaderStoreList) {
    LOOP_SWP_DEBUG(DebugTag << "Store : " << *I)

    auto StoreSeq = new LoadStoreSequence(I, WorkingLoop, BlockType::Header,
                                          InstSeqMap, InductionInsts);
    InstLoadStoreSeqMap.insert(std::make_pair(I, StoreSeq));

    if (!StoreSeq->populateSequence(InstUpdateMap[BlockType::Header])) {
      LOOP_SWP_DEBUG(DebugTag
                     << "StoreSequence construction failed, exiting ...")
      return false;
    }
    StoreSeq->debugDump();
  }

  // check if Induction Phi is part of any InstSeq
  for (auto &PhiNode : HeaderBlock->phis()) {
    if (isInductionPHI(&PhiNode)) {
      Instruction *Phi = cast<Instruction>(&PhiNode);
      auto It = InstSeqMap.find(Phi);
      if (It != InstSeqMap.end()) {
        HasInductionInCoordSeq = true;
        break;
      }
    }
  }

  return true;
}

Instruction *LoopSWP::getClone(Instruction *I, BlockType BT) {
  assert((BT == BlockType::Preheader || BT == BlockType::Exit) &&
         "Invalid BlockType");

  switch (BT) {
  case BlockType::Preheader:
    return getPreheaderClone(I);
  case BlockType::Exit:
    return getExitClone(I);
  default:
    return nullptr;
  };
}

void LoopSWP::replaceDefsWithClones(Instruction *I, BlockType BT,
                                    std::string DebugTag) {
  if (isa<PHINode>(I))
    return;
  LOOP_SWP_DEBUG(DebugTag << "Replacing with clones of operands in :")
  LOOP_SWP_DEBUG(DebugTag << *I)
  for (unsigned OpIdx = 0; OpIdx < I->getNumOperands(); OpIdx++) {
    if (auto *OpDef = dyn_cast<Instruction>(I->getOperand(OpIdx))) {
      if (OpDef->getParent() != HeaderBlock)
        continue;
      LOOP_SWP_DEBUG(DebugTag << "Op : " << *OpDef)
      Instruction *OpClone = getClone(OpDef, BT);
      LOOP_SWP_DEBUG(DebugTag << "OpClone = " << *OpClone)
      I->setOperand(OpIdx, OpClone);
    }
  }
  LOOP_SWP_DEBUG(DebugTag << *I)
  LOOP_SWP_DEBUG(DebugTag << "Operands replaced.")

  return;
}

Instruction *LoopSWP::createPreheaderCloneBase(Instruction *I,
                                               InstCloneMapType &CloneMap,
                                               bool BlockInsertFlag,
                                               std::string DebugTag) {
  assert(!isa<PHINode>(I) && "Phi cloning must not be done here");
  Instruction *PreheaderClone = I->clone();
  CloneMap.insert(std::make_pair(I, PreheaderClone));
  if (BlockInsertFlag) {
    replaceDefsWithClones(PreheaderClone, BlockType::Preheader,
                          DebugTag + "\t");
    insertIntoBlock(BlockType::Preheader, PreheaderClone, DebugTag);
  }

  return PreheaderClone;
}

Instruction *LoopSWP::createPreheaderLoadClone(Instruction *LoadInst,
                                               std::string DebugTag) {
  auto *LoadInstClone = createPreheaderCloneBase(
      LoadInst, InstCloneMap[BlockType::Preheader], false, DebugTag);

  // find the Seq object for LoadInst
  auto It = InstLoadStoreSeqMap.find(LoadInst);
  assert(It != InstLoadStoreSeqMap.end() &&
         "LoadSequence for clone source not found");

  auto LoadSeq =
      new LoadStoreSequence(LoadInstClone, WorkingLoop, BlockType::Header,
                            InstSeqMap, InductionInsts);
  bool IsCloned = LoadSeq->clone(It->second, InstCloneMap[BlockType::Preheader],
                                 InsertedBlockInsts, BBInsertIt, true,
                                 BlockType::Preheader);
  (void)IsCloned;
  assert(IsCloned && "LoadSequence construction failed, exiting ...");

  // now insert the load clone into Preheader
  insertIntoBlock(BlockType::Preheader, LoadInstClone, DebugTag);

  InstLoadStoreSeqMap.insert(std::make_pair(LoadInstClone, LoadSeq));
  LoadSeq->debugDump();

  return LoadInstClone;
}

Instruction *LoopSWP::createPreheaderClone(Instruction *I,
                                           std::string DebugTag) {
  LOOP_SWP_DEBUG(DebugTag << "----")
  LOOP_SWP_DEBUG(DebugTag << "Cloning Instruction : " << *I)
  Instruction *Clone = nullptr;
  if (TPCOptUtils::isLoadTensor(I))
    Clone = createPreheaderLoadClone(I, DebugTag);
  // if computes/stores are to be peeled out of Header, handle them here

  if (Clone)
    LOOP_SWP_DEBUG(DebugTag << "Cloned Instruction = " << *Clone)
  else
    LOOP_SWP_DEBUG(DebugTag << "Cloned Instruction = <nullptr>")
  LOOP_SWP_DEBUG(DebugTag << "----")

  return Clone;
}

Instruction *LoopSWP::getPreheaderClone(Instruction *I) {
  auto It = InstCloneMap[BlockType::Preheader].find(I);
  return ((It != InstCloneMap[BlockType::Preheader].end()) ? It->second
                                                           : nullptr);
}

void LoopSWP::createPreheaderClones(std::string DebugTag) {
  auto InsertIt = PreheaderBlock->end();
  InsertIt--;

  LOOP_SWP_DEBUG(
      DebugTag
      << "Cloning loads and corresponding load sequences for Preheader Block")
  // clone the load sequences, loads and insert them to preheader
  for (unsigned i = 0; i < getLoadPrefetchCount(); i++) {
    LOOP_SWP_DEBUG(DebugTag << "#" << i)
    for (auto LoadInst : LoopClusters[i]->HeaderLoadInstsVec) {
      Instruction *LoadClone = createPreheaderClone(LoadInst, DebugTag);
      (void)LoadClone;
      assert(LoadClone && "Preheader Load cloning failed");
    }
  }

  LOOP_SWP_DEBUG(
      DebugTag
      << "Cloning trailing add.mask / induction update instructions ...")
  // populate reverse map PhiClone -> Phi
  for (auto &PhiNode : HeaderBlock->phis()) {
    Instruction *HeaderPhi = cast<Instruction>(&PhiNode);
    LOOP_SWP_DEBUG(DebugTag << "----")
    LOOP_SWP_DEBUG(DebugTag << "HeaderPhi = " << *HeaderPhi)
    // check if this phi has a clone in Preheader
    auto It = InstCloneMap[BlockType::Preheader].find(HeaderPhi);
    if (It == InstCloneMap[BlockType::Preheader].end())
      continue;
    Instruction *PreheaderPhi = It->second;
    // add an entry in the reverse map (ClonePhi -> Phi)
    PreToHeaderPhiMap.insert(std::make_pair(PreheaderPhi, HeaderPhi));
    LOOP_SWP_DEBUG(DebugTag << "PreheaderPhi = " << *PreheaderPhi)
    // This will be useful while establishing similarity b/w a Preheader Phi
    // and it's update inst (by refering to HeaderPhi type)

    // if this Phi is of AddMask type, get the N'th (N = PrefetchCount) update
    // add_mask instruction and clone it
    bool IsInductionPHI = isInductionPHI(&PhiNode);
    bool IsAddMask =
        TPCOptUtils::isPhiOfType(PhiNode, HeaderBlock, Intrinsic::tpc_add_mask);
    if (!(IsInductionPHI || IsAddMask)) {
      LOOP_SWP_DEBUG(DebugTag << "Non AddMask/Induction Phi, continuing ...")
      continue;
    }
    Instruction *NthUpdate = nullptr;
    if (IsAddMask) {
      NthUpdate =
          getNthUpdate(HeaderPhi, getLoadPrefetchCount(), BlockType::Header);
    } else { // Induction Phi
      assert(getLoadPrefetchCount() == getNumClusters() &&
             "If CoordSeq has Induction Phis, all cluster's loads are to be "
             "prefetched");
      NthUpdate =
          cast<Instruction>(PhiNode.getIncomingValueForBlock(HeaderBlock));
    }
    assert(NthUpdate &&
           "Trailing add_mask / induction update instruction not found!");
    Instruction *NthUpdateClone = createPreheaderCloneBase(
        NthUpdate, InstCloneMap[BlockType::Preheader], true, DebugTag);

    setPhiInfo(NthUpdateClone, PreheaderPhi,
               InstToAlphaMap[BlockType::Preheader], BlockType::Preheader);
  }

  LOOP_SWP_DEBUG(DebugTag << "Collecting phi updates for Preheader block")
  if (!collectAlphaUpdates(BlockType::Preheader)) {
    LOOP_SWP_DEBUG(DebugTag << "Phi update collection failed, exiting ...")
    assert(false && "Phi update collection failed ...");
  }

  return;
}

void LoopSWP::patchHeaderPhisWithPreheaderIncome(std::string DebugTag) {
  LOOP_SWP_DEBUG(
      DebugTag << "Patching the header phis with preheader income ...")
  for (auto &PhiNode : HeaderBlock->phis()) {
    Instruction *HeaderPhi = cast<Instruction>(&PhiNode);
    auto It = InstCloneMap[BlockType::Preheader].find(HeaderPhi);
    if (It == InstCloneMap[BlockType::Preheader].end())
      continue;

    LOOP_SWP_DEBUG(DebugTag << "----")
    LOOP_SWP_DEBUG(DebugTag << "Phi = " << *HeaderPhi)
    LOOP_SWP_DEBUG(DebugTag << "PhiClone = " << *It->second)
    Instruction *LastUpdate = getLastUpdate(It->second, BlockType::Preheader);
    assert(LastUpdate && "Instruction update not found!");

    BasicBlockVecType PhiBlocks(PhiNode.block_begin(), PhiNode.block_end());
    for (BasicBlock *BB : PhiBlocks)
      if (BB != HeaderBlock)
        PhiNode.removeIncomingValue(BB, false);

    // add the new incoming value from Preheader block
    PhiNode.addIncoming(LastUpdate, PreheaderBlock);
    // Header incoming value should be reset after the new load clones are
    // added at the end of Header block

    LOOP_SWP_DEBUG(DebugTag << "Phi' = " << *HeaderPhi)
  }
}

// TODO: strategy needs to be changed based on the unroll factor, the number of
// loads, stores, compute etc.
void LoopSWP::modifyPreheader() {
  std::string DebugTag = "<modifyPreheader> ";

  LOOP_SWP_DEBUG(DebugTag << "==== Modifying PreheaderBlock ====")

  BBPhiInsertIt = PreheaderBlock->begin();
  BBInsertIt = PreheaderBlock->end();
  BBInsertIt--;

  // create coord clones for each of the loads in the Header block
  createPreheaderClones(DebugTag);

  patchHeaderPhisWithPreheaderIncome(DebugTag);

  LOOP_SWP_DEBUG(DebugTag << "==== Preheader modification done ====")

  return;
}

Instruction *LoopSWP::createHeaderLoadPhi(Instruction *LoadInst, int ClusterIdx,
                                          int LoadIdx, std::string DebugTag) {
  LOOP_SWP_DEBUG(DebugTag << "Creating Header Load Phi ...")

  auto PhiInsertIt = HeaderBlock->begin();
  PhiInsertIt++;
  int N = getNumClusters();
  int X = getLoadPrefetchCount();

  // get the next cluster's avatar of the current LoadInst
  Instruction *LoadInstNext =
      LoopClusters[ClusterIdx + (N - X)]->HeaderLoadInstsVec[LoadIdx];
  LOOP_SWP_DEBUG(DebugTag << "Header Income = " << *LoadInstNext)

  Instruction *PreLoadInst = getPreheaderClone(LoadInst);
  LOOP_SWP_DEBUG(DebugTag << "Preheader Income = " << *PreLoadInst)

  PHINode *LoadPhiNode = nullptr;

  if (getHasLoadIncome() && (ClusterIdx == X - 1)) {
    // If a load income/pred is present, it means the original Header Block
    // had a Phi node for Loads
    // This condition decides which of the peeled load should be connected with
    // the existing Phi node (without creating a new one).
    LoadPhiNode = cast<PHINode>(InstToAlphaMap[BlockType::Header][LoadInst]);
    // clear the current incoming values
    LoadPhiNode->removeIncomingValue(HeaderBlock, false);
    LoadPhiNode->removeIncomingValue(PreheaderBlock, false);
    // mark as inserted
    InsertedBlockInsts.insert(cast<Instruction>(LoadPhiNode));
  } else {
    // create a new phi node
    LoadPhiNode = PHINode::Create(cast<Value>(LoadInst)->getType(),
                                  2); // 2 incoming values
  }
  LoadPhiNode->addIncoming(LoadInstNext, HeaderBlock);
  LoadPhiNode->addIncoming(PreLoadInst, PreheaderBlock);

  Instruction *LoadPhiInst = cast<Instruction>(LoadPhiNode);
  insertIntoBlock(BlockType::Header, LoadPhiInst, DebugTag);
  setPhiInfo(LoadInstNext, LoadPhiInst, InstToAlphaMap[BlockType::Header],
             BlockType::Header);

  LOOP_SWP_DEBUG(DebugTag << "New Header Load Phi = " << *LoadPhiInst);

  return LoadPhiInst;
}

void LoopSWP::modifyHeaderBlock() {
  std::string DebugTag = "<modifyHeaderBlock> ";
  LOOP_SWP_DEBUG(DebugTag << "==== Modifying HeaderBlock ====")

  BBPhiInsertIt = HeaderBlock->begin();
  BBInsertIt = HeaderBlock->getFirstInsertionPt();

  int N = getNumClusters();
  int X = getLoadPrefetchCount();

  // clear the record of instructions inserted into the preheader block
  InsertedBlockInsts.clear();

  LOOP_SWP_DEBUG(
      DebugTag << "\nPatching the load instructions for Clusters 0 .. " << X - 1
               << "\n")
  for (int i = 0; i < X; i++) {
    LOOP_SWP_DEBUG(DebugTag << "---- Cluster #" << i << " ----")
    for (unsigned l = 0; l < LoopClusters[i]->HeaderLoadInstsVec.size(); l++) {
      Instruction *LoadInst = LoopClusters[i]->HeaderLoadInstsVec[l];
      LOOP_SWP_DEBUG(DebugTag << "----")
      LOOP_SWP_DEBUG(DebugTag << "LoadInst : " << *LoadInst)

      Instruction *LoadPhi = createHeaderLoadPhi(LoadInst, i, l, DebugTag);
      TPCOptUtils::replaceUsesOfWith(
          LoadInst, LoadPhi,
          [&](Instruction *UserInst) -> bool {
            auto It = InsertedBlockInsts.find(UserInst);
            return (!TPCOptUtils::isIntrinsicOfType(UserInst,
                                                    Intrinsic::tpc_ld_g) &&
                    It == InsertedBlockInsts.end());
          },
          DebugTag + "\t");

      InstCloneMap[BlockType::Header].insert(std::make_pair(LoadInst, LoadPhi));
    }
  }

  LOOP_SWP_DEBUG(DebugTag << "\nPatching the load instructions for Clusters "
                          << X << " .. " << N - 1 << "\n")
  for (int i = X; i < N; i++) {
    // patch the last load in Header
    for (unsigned l = 0; l < LoopClusters[i]->HeaderLoadInstsVec.size(); l++) {
      Instruction *LoadInst = LoopClusters[i]->HeaderLoadInstsVec[l];
      LOOP_SWP_DEBUG(DebugTag << "----")
      LOOP_SWP_DEBUG(DebugTag << "LoadInst : " << *LoadInst)
      BasicBlock *BB = LoadInst->getParent();

      Instruction *LoadInstReplace = LoopClusters[i - X]->HeaderLoadInstsVec[l];
      LOOP_SWP_DEBUG(DebugTag << "Replacement Instruction = "
                              << *LoadInstReplace)
      TPCOptUtils::replaceUsesOfWith(
          LoadInst, LoadInstReplace,
          [&](Instruction *UserInst) -> bool {
            if (BB != UserInst->getParent())
              return false;
            auto It = InsertedBlockInsts.find(UserInst);
            return (It == InsertedBlockInsts.end());
          },
          DebugTag + "\t");
      InstCloneMap[BlockType::Header].insert(
          std::make_pair(LoadInst, LoadInstReplace));
    }
  }

  return;
}

void LoopSWP::collectExitLCSSAPhis(InstructionSetType &LcssaPhis,
                                   std::string DebugTag) {
  LOOP_SWP_DEBUG(DebugTag << "Collecting Exit LCSSA Phis ...")

  for (auto &PhiNode : ExitBlock->phis()) {
    Instruction *I = cast<Instruction>(&PhiNode);
    LOOP_SWP_DEBUG(DebugTag << "\t" << *I)

    // get the incoming value from Header
    Instruction *HeaderIncome =
        cast<Instruction>(PhiNode.getIncomingValueForBlock(HeaderBlock));
    LOOP_SWP_DEBUG(DebugTag << "Reference HeaderIncome = ")
    LOOP_SWP_DEBUG(DebugTag << *HeaderIncome)
    if (isa<PHINode>(HeaderIncome)) {
      // with LoadPrefetchCount = NumClusters, the ExitBlock Phi's header income
      // value can be a Phi node in the HeaderBlock
      PHINode *HeaderIncomePhiNode = cast<PHINode>(HeaderIncome);
      HeaderIncome = cast<Instruction>(
          HeaderIncomePhiNode->getIncomingValueForBlock(HeaderBlock));
      LOOP_SWP_DEBUG(DebugTag << "Reference HeaderIncome' = ")
      LOOP_SWP_DEBUG(DebugTag << *HeaderIncome)
    }

    Instruction *HeaderPhi = nullptr;
    // Accum type Phi
    if (getIsAccumInst(HeaderIncome)) {
      LOOP_SWP_DEBUG(DebugTag << "\tAccumPhi")
      // get the Header accum phi
      HeaderPhi = AccumToPhiMap[BlockType::Header][HeaderIncome];
      setIsAccumInst(I, I, BlockType::Exit);
    } else {
      LOOP_SWP_DEBUG(DebugTag << "\tNon-AccumPhi")
      HeaderPhi = InstToAlphaMap[BlockType::Header][HeaderIncome];
      setPhiInfo(I, I, InstToAlphaMap[BlockType::Exit], BlockType::Exit);
    }

    assert(HeaderPhi && "Header Phi Cannot be nullptr");
    LcssaPhis.insert(I);
    InstCloneMap[BlockType::Exit].insert(std::make_pair(HeaderPhi, I));

    // if the LCSSA Phi node doesn't already have an income value from
    // preheader, add corresponding income value in the HeaderPhi
    bool HasPreheaderIncome = false;
    BasicBlockVecType PhiBlocks(PhiNode.block_begin(), PhiNode.block_end());
    for (BasicBlock *BB : PhiBlocks) {
      if (BB == PreheaderBlock) {
        HasPreheaderIncome = true;
        break;
      }
    }
    if (!HasPreheaderIncome) {
      LOOP_SWP_DEBUG(
          DebugTag
          << "LCSSA node does not have an incoming value from PreheaderBlock")
      Value *PreheaderIncome =
          cast<PHINode>(HeaderPhi)->getIncomingValueForBlock(PreheaderBlock);
      LOOP_SWP_DEBUG(DebugTag << "Inserting value : " << *PreheaderIncome)
      PhiNode.addIncoming(PreheaderIncome, PreheaderBlock);
    }
  }

  return;
}

void LoopSWP::updateLcssaPhiUsers(InstructionSetType &LcssaPhis,
                                  std::string DebugTag) {
  LOOP_SWP_DEBUG(DebugTag << "Updating Exit LCSSA Phi Users")

  for (auto PhiInst : LcssaPhis) {
    LOOP_SWP_DEBUG(DebugTag << "----\n" << *PhiInst)
    LOOP_SWP_DEBUG(DebugTag
                   << (getIsAccumInst(PhiInst, BlockType::Exit) ? "\tAccumPhi"
                                                                : "\tCoordPhi"))
    // TODO: use getter
    auto It = PhiLastUpdateMap[BlockType::Exit].find(PhiInst);
    assert(It != PhiLastUpdateMap[BlockType::Exit].end() &&
           "Last update instruction not found");
    Instruction *Last = It->second;
    LOOP_SWP_DEBUG(DebugTag << "Last Update = " << *Last)
    if (Last == PhiInst) // there is no update coord inst
      continue;

    // replace uses outside the exit block
    UserSetType UserSet;
    for (User *U : PhiInst->users())
      UserSet.insert(U);
    LOOP_SWP_DEBUG(DebugTag << "Users : {")
    for (const auto &UserObj : UserSet) {
      Instruction *UserInst = cast<Instruction>(UserObj);
      LOOP_SWP_DEBUG(DebugTag << "\t--\n" << *UserInst)
      LOOP_SWP_DEBUG(DebugTag << "\tParent = "
                              << UserInst->getParent()->getName())

      // don't change any other use in the exit block, except those that were
      // not inserted in this pass
      if (UserInst->getParent() == ExitBlock &&
          InsertedBlockInsts.find(UserInst) != InsertedBlockInsts.end())
        continue;
      LOOP_SWP_DEBUG(DebugTag << "Replacing use ...")
      UserInst->replaceUsesOfWith(PhiInst, Last);
    }
    LOOP_SWP_DEBUG(DebugTag << "}")
  }
  return;
}

Instruction *LoopSWP::createExitCloneBase(Instruction *I,
                                          InstCloneMapType &CloneMap,
                                          bool BlockInsertFlag,
                                          std::string DebugTag) {
  Instruction *ExitClone = I->clone();
  CloneMap.insert(std::make_pair(I, ExitClone));
  if (BlockInsertFlag) {
    // re-adjust PHI Insertion point to the latest first non-PHI position
    if (isa<PHINode>(I))
      BBPhiInsertIt = ExitBlock->getFirstInsertionPt();
    else
      replaceDefsWithClones(ExitClone, BlockType::Exit, DebugTag + "\t");
    insertIntoBlock(BlockType::Exit, ExitClone, DebugTag);
  }

  return ExitClone;
}

Instruction *LoopSWP::createExitLoadStoreInstClone(Instruction *I,
                                                   std::string DebugTag) {
  auto *InstClone = createExitCloneBase(I, InstCloneMap[BlockType::Preheader],
                                        false, DebugTag);

  // find the Seq object for I
  auto It = InstLoadStoreSeqMap.find(I);
  assert(It != InstLoadStoreSeqMap.end() &&
         "Load/Store Sequence for clone source not found");

  auto LoadStoreSeq = new LoadStoreSequence(
      InstClone, WorkingLoop, BlockType::Header, InstSeqMap, InductionInsts);
  bool IsCloned = LoadStoreSeq->clone(It->second, InstCloneMap[BlockType::Exit],
                                      InsertedBlockInsts, BBInsertIt, true,
                                      BlockType::Exit);
  (void)IsCloned;
  assert(IsCloned && "Load/Store Sequence construction failed, exiting ...");

  // TODO: not necessary after income-pred seq is supported for stores
  if (TPCOptUtils::isStoreTensor(I))
    replaceDefsWithClones(InstClone, BlockType::Exit, DebugTag + "\t");

  // now insert the load clone into Preheader
  insertIntoBlock(BlockType::Exit, InstClone, DebugTag);

  InstLoadStoreSeqMap.insert(std::make_pair(InstClone, LoadStoreSeq));
  LoadStoreSeq->debugDump();

  return InstClone;
}

Instruction *LoopSWP::createExitAccumClone(Instruction *I,
                                           std::string DebugTag) {
  Instruction *ExitAccumInst =
      createExitCloneBase(I, InstCloneMap[BlockType::Exit], true, DebugTag);
  // mark as accum inst
  Instruction *HeaderAccumPhi = AccumToPhiMap[BlockType::Header][I];
  Instruction *ExitAccumPhi = getExitClone(HeaderAccumPhi);
  setIsAccumInst(ExitAccumInst, ExitAccumPhi, BlockType::Exit);

  return ExitAccumInst;
}

Instruction *LoopSWP::createExitClone(Instruction *I, std::string DebugTag) {
  LOOP_SWP_DEBUG(DebugTag << "----")
  LOOP_SWP_DEBUG(DebugTag << "Cloning Instruction : " << *I)
  Instruction *Clone = nullptr;
  if (isa<PHINode>(I))
    Clone =
        createExitCloneBase(I, InstCloneMap[BlockType::Exit], true, DebugTag);
  else if (TPCOptUtils::isLoadTensor(I) || TPCOptUtils::isStoreTensor(I))
    Clone = createExitLoadStoreInstClone(I, DebugTag);
  else if (getIsAccumInst(I))
    Clone = createExitAccumClone(I, DebugTag);
  else
    Clone =
        createExitCloneBase(I, InstCloneMap[BlockType::Exit], true, DebugTag);

  if (Clone)
    LOOP_SWP_DEBUG(DebugTag << "Cloned Instruction = " << *Clone)
  else
    LOOP_SWP_DEBUG(DebugTag << "Cloned Instruction = <nullptr>")
  LOOP_SWP_DEBUG(DebugTag << "----")

  return Clone;
}

Instruction *LoopSWP::getExitClone(Instruction *I) {
  InstCloneMapType ExitCloneMap;
  ExitCloneMap = InstCloneMap[BlockType::Exit];

  auto It = ExitCloneMap.find(I);
  return ((It != ExitCloneMap.end()) ? It->second : nullptr);
}

void LoopSWP::createExitLoadInsts(unsigned ClusterIdx, std::string DebugTag) {
  LOOP_SWP_DEBUG(DebugTag << "Create Load Clones")
  for (unsigned l = 0; l < LoopClusters[ClusterIdx]->HeaderLoadInstsVec.size();
       l++) {
    LOOP_SWP_DEBUG(DebugTag << "--")
    Instruction *LoadInst = LoopClusters[ClusterIdx]->HeaderLoadInstsVec[l];
    LOOP_SWP_DEBUG(DebugTag << "LoadInst = " << *LoadInst)
    Instruction *HeaderClone = InstCloneMap[BlockType::Header][LoadInst];
    LOOP_SWP_DEBUG(DebugTag << "HeaderClone = " << *HeaderClone)

    // TODO : required?
    if (getExitClone(HeaderClone))
      continue;

    createExitClone(HeaderClone, DebugTag);
  }
  return;
}

void LoopSWP::createExitOutInsts(unsigned ClusterIdx, std::string DebugTag) {
  LOOP_SWP_DEBUG(DebugTag << (getIsAccumulationLoop() ? "Create Accum Clones"
                                                      : "Create Store Clones"))

  if (getIsAccumulationLoop()) {
    for (unsigned s = 0;
         s < LoopClusters[ClusterIdx]->HeaderAccumInstsVec.size(); s++) {
      LOOP_SWP_DEBUG(DebugTag << "--")
      Instruction *OutInst = LoopClusters[ClusterIdx]->HeaderAccumInstsVec[s];
      LOOP_SWP_DEBUG(DebugTag << "OutInst = " << *OutInst)

      // The accum insts could be reaching as lcssa phi nodes
      if (getExitClone(OutInst))
        continue;

      createExitClone(OutInst, DebugTag);
    }
  } else {
    for (unsigned s = 0;
         s < LoopClusters[ClusterIdx]->HeaderStoreInstsVec.size(); s++) {
      LOOP_SWP_DEBUG(DebugTag << "--")
      Instruction *OutInst = LoopClusters[ClusterIdx]->HeaderStoreInstsVec[s];
      LOOP_SWP_DEBUG(DebugTag << "OutInst = " << *OutInst)

      // The accum insts could be reaching as lcssa phi nodes
      if (getExitClone(OutInst))
        continue;

      createExitClone(OutInst, DebugTag);
    }
  }
  return;
}

void LoopSWP::createExitComputeInsts(unsigned ClusterIdx,
                                     std::string DebugTag) {
  LOOP_SWP_DEBUG(DebugTag << "Create compute Clones")
  for (unsigned c = 0;
       c < LoopClusters[ClusterIdx]->HeaderComputeInstsVec.size(); c++) {
    LOOP_SWP_DEBUG(DebugTag << "--")
    Instruction *ComputeInst =
        LoopClusters[ClusterIdx]->HeaderComputeInstsVec[c];
    LOOP_SWP_DEBUG(DebugTag << "ComputeInst = " << *ComputeInst)

    createExitClone(ComputeInst, DebugTag);
  }
  return;
}

void LoopSWP::modifyExitBlock() {
  std::string DebugTag = "<modifyExitBlock> ";

  LOOP_SWP_DEBUG(DebugTag << "==== Modifying ExitBlock ====")

  BBPhiInsertIt = ExitBlock->begin();
  BBInsertIt = ExitBlock->getFirstInsertionPt();

  int N = getNumClusters();

  // if the exit block already has lcssa phi nodes, connect them with the
  // header phi nodes
  //
  // set of all lcssa coords found in ExitBlock
  InstructionSetType LcssaPhis;
  collectExitLCSSAPhis(LcssaPhis, DebugTag);

  // clear the record of instructions inserted into the exit block
  InsertedBlockInsts.clear();
  InsertedBlockInsts.insert(LcssaPhis.begin(), LcssaPhis.end());

  for (int i = 0; i < N; i++) {
    LOOP_SWP_DEBUG(DebugTag << "---- Cluster #" << i << " ----")
    createExitLoadInsts(i, DebugTag);

    createExitComputeInsts(i, DebugTag);

    createExitOutInsts(i, DebugTag);
  }

  unsigned X = getLoadPrefetchCount();
  LOOP_SWP_DEBUG(DebugTag << "Cloning trailing add.mask instructions ...")
  // populate reverse map PhiClone -> Phi
  for (auto &PhiNode : HeaderBlock->phis()) {
    Instruction *HeaderPhi = cast<Instruction>(&PhiNode);
    LOOP_SWP_DEBUG(DebugTag << "----")
    LOOP_SWP_DEBUG(DebugTag << "HeaderPhi = " << *HeaderPhi)
    // check if this phi has a clone in Preheader
    auto It = InstCloneMap[BlockType::Exit].find(HeaderPhi);
    if (It == InstCloneMap[BlockType::Exit].end())
      continue;
    // add an entry in the reverse map (ClonePhi -> Phi)
    ExitToHeaderPhiMap.insert(std::make_pair(It->second, HeaderPhi));
    LOOP_SWP_DEBUG(DebugTag << "ExitPhi = " << *It->second)
    // This will be useful while establishing similarity b/w a Exit Phi
    // and it's update inst (by refering to HeaderPhi type)

    // if this Phi is of AddMask type, get the N'th (N = PrefetchCount) update
    // add_mask instruction and clone it
    if (!TPCOptUtils::isPhiOfType(PhiNode, HeaderBlock,
                                  Intrinsic::tpc_add_mask)) {
      LOOP_SWP_DEBUG(DebugTag << "\tnot an add.mask phi, continuing ...")
      continue;
    }

    unsigned TrailingPos = 0;
    auto SeqIt = InstSeqMap.find(HeaderPhi);
    assert(SeqIt != InstSeqMap.end() &&
           "AddMask Phi not found in any InstSeq!");
    if (SeqIt->second->isLoadPivot()) {
      TrailingPos = N - X;
      if (!TrailingPos)
        continue;
    } else {
      TrailingPos = N;
    }

    Instruction *NthUpdate =
        getNthUpdate(HeaderPhi, TrailingPos, BlockType::Header);
    assert(NthUpdate && "Trailing add_mask instruction not found!");
    createExitCloneBase(NthUpdate, InstCloneMap[BlockType::Exit], true,
                        DebugTag);
  }

  LOOP_SWP_DEBUG(DebugTag << "Collecting phi updates for Exit block")
  if (!collectAlphaUpdates(BlockType::Exit)) {
    LOOP_SWP_DEBUG(DebugTag << "Phi update collection failed, exiting ...")
    assert(false && "Phi update collection failed ...");
  }

  // replace all uses of the coord phis with their last update
  // instruction (this must be done before cloning for ExitBlock loads)
  updateLcssaPhiUsers(LcssaPhis, DebugTag);

  return;
}

void LoopSWP::updateLoopBounds() {
  std::string DebugTag = "<updateLoopBounds> ";

  BBPhiInsertIt = PreheaderBlock->begin();
  BBInsertIt = PreheaderBlock->end();
  BBInsertIt--;

  // TODO: use llvm methods to get induction var

  // find the branching instruction
  Instruction *InductionPhi = nullptr;
  Value *InductionSize = nullptr;
  // get the branch instruction
  BranchInst *BI = nullptr;
  auto It = HeaderBlock->end();
  while (It != HeaderBlock->begin()) {
    It--;
    Instruction *I = &(*It);
    if (isa<BranchInst>(I)) {
      BI = cast<BranchInst>(I);
      break;
    }
  }

  // NOTE: assumes that the CondLHS is a result of single statement
  Instruction *Condition = cast<Instruction>(BI->getCondition());
  Instruction *CondLHS = cast<Instruction>(Condition->getOperand(0));
  InductionPhi = cast<Instruction>(CondLHS->getOperand(0));
  InductionSize = CondLHS->getOperand(1);

  assert(isa<Constant>(InductionSize));
  assert(InductionPhi && "Could not find the Induction variable");
  LOOP_SWP_DEBUG(DebugTag << "InductionPhi = " << *InductionPhi)
  LOOP_SWP_DEBUG(DebugTag << "InductionSize = " << *InductionSize)

  // increment on PreheaderIncome
  PHINode *InductionPhiNode = cast<PHINode>(InductionPhi);
  Instruction *NewInductionUpdate;
  auto CloneIt = InstCloneMap[BlockType::Preheader].find(CondLHS);
  // not cloned yet
  if (CloneIt == InstCloneMap[BlockType::Preheader].end()) {
    // get the incoming value for InductionPhi from Preheader block
    Value *IncomeVal =
        InductionPhiNode->getIncomingValueForBlock(PreheaderBlock);
    Value *NewIncomeVal = IncomeVal;

    if (auto *I = dyn_cast<Instruction>(IncomeVal)) {
      BasicBlock *InValParent = InValParent = I->getParent();
      if (InValParent != PreheaderBlock) {
        PHINode *NewInductionPhiNode =
            PHINode::Create(InductionPhi->getType(), 1);
        // TODO: assuming unnecessary PHI nodes will be eliminated later
        // for each predecessor of PreheaderBlock, add the same incoming value
        for (BasicBlock *Pred : predecessors(PreheaderBlock))
          NewInductionPhiNode->addIncoming(IncomeVal, Pred);
        NewIncomeVal = cast<Value>(NewInductionPhiNode);
        auto PreInsertIt = PreheaderBlock->begin();
        PreheaderBlock->getInstList().insert(PreInsertIt,
                                             cast<Instruction>(NewIncomeVal));
      }
    }
    NewInductionUpdate = CondLHS->clone();
    NewInductionUpdate->setOperand(0, NewIncomeVal);
    // insert into preheader block
    insertIntoBlock(BlockType::Preheader, NewInductionUpdate, DebugTag);
  } else {
    NewInductionUpdate = CloneIt->second;
  }

  // add branching condition clone
  Instruction *ConditionClone = Condition->clone();
  ConditionClone->setOperand(0, NewInductionUpdate);
  insertIntoBlock(BlockType::Preheader, ConditionClone, DebugTag);

  // add branching inst clone
  Instruction *NewPreBI =
      BranchInst::Create(ExitBlock, HeaderBlock, ConditionClone);
  insertIntoBlock(BlockType::Preheader, NewPreBI, DebugTag);

  auto BIt = PreheaderBlock->end();
  BIt--;
  BranchInst *StalePreBI = cast<BranchInst>(&(*BIt));
  LOOP_SWP_DEBUG(
      DebugTag << "Erasing the previous Preheader branch instruction :")
  LOOP_SWP_DEBUG(DebugTag << *StalePreBI)
  StalePreBI->eraseFromParent();

  // update the InductionPhi
  // (false: dont delete if empty)
  InductionPhiNode->removeIncomingValue(PreheaderBlock, false);
  InductionPhiNode->addIncoming(cast<Value>(NewInductionUpdate),
                                PreheaderBlock);

  return;
}

#undef DEBUG_TYPE
