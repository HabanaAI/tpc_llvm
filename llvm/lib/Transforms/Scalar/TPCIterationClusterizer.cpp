#include "TPCIterationClusterizer.h"

using namespace llvm;

#define DEBUG_TYPE "clusterizer"

IterationClusterizer::IterationClusterizer(Loop *WorkingLoop,
                                           ScalarEvolution *SE,
                                           bool PopulateIfMultiCluster,
                                           bool SkipIfZeroLoadTensors,
                                           bool ShouldDetectPipelinedLoads)
    : WorkingLoop(WorkingLoop), SE(SE),
      PopulateIfMultiCluster(PopulateIfMultiCluster),
      SkipIfZeroLoadTensors(SkipIfZeroLoadTensors),
      ShouldDetectPipelinedLoads(ShouldDetectPipelinedLoads), NumClusters(0),
      IsAccumulationLoop(false) {
  assert(WorkingLoop && "Cannot clusterize : Invalid loop");
  assert(WorkingLoop->getSubLoops().size() == 0 &&
         "Cannot clusterize : Non-innermost loop");
  assert((WorkingLoop->getNumBlocks() < 2) &&
         "Cannot clusterize : Loops with branching");
  HeaderBlock = WorkingLoop->getHeader();
  PreheaderBlock = WorkingLoop->getLoopPreheader();

  WorkingLoop->getInductionDescriptor(*SE, InductionDesc);
}

bool IterationClusterizer::recursivelyMarkClusters(
    Instruction *I, unsigned ClusterIdx, InstInstMapType &AccumPhiMap,
    InstructionSetType &UnclusteredInstructions, UnionFind &ClusterUF,
    std::string Indent) {
  Indent += "\t";
  CLUSTER_INFO_DEBUG(Indent << *I)
  if (I->getParent() != HeaderBlock) {
    CLUSTER_INFO_DEBUG(Indent << "not in BasicBlock, continuing ...")
    return true;
  }

  if (TPCOptUtils::isCoordUpdate(I)) {
    UnclusteredInstructions.erase(I);
    return true;
  }
  if (auto *PHI = dyn_cast<PHINode>(I)) {
    UnclusteredInstructions.erase(I);
    if (ShouldDetectPipelinedLoads) {
      // We handle PHIs of ld_tnsr by marking the incoming from the header as
      // the corresponding ld_tnsr of the current cluster. This should traverse
      // the "PHI rewired" loads post LoopSWP

      if (PHI->getNumIncomingValues() != 2)
        return true;

      if (PHI->getBasicBlockIndex(HeaderBlock) == -1 ||
          PHI->getBasicBlockIndex(PreheaderBlock) == -1)
        return true;

      Instruction *HeaderInc =
          dyn_cast<Instruction>(PHI->getIncomingValueForBlock(HeaderBlock));
      Instruction *PreheaderInc =
          dyn_cast<Instruction>(PHI->getIncomingValueForBlock(PreheaderBlock));

      if (!HeaderInc || !PreheaderInc)
        return true;

      if (!TPCOptUtils::isIntrinsicOfType(HeaderInc, Intrinsic::tpc_ld_tnsr) ||
          !TPCOptUtils::isIntrinsicOfType(PreheaderInc, Intrinsic::tpc_ld_tnsr))
        return true;
      I = HeaderInc;
    } else {
      if (AccumPhiMap.find(I) == AccumPhiMap.end()) {
        // Coord / Income-Pred Phis are supposed to be reached through InstSeq
        CLUSTER_INFO_DEBUG(Indent << "Non-accum phi reached, continuing ...")
        return false;
      }
      CLUSTER_INFO_DEBUG(Indent << "Phi reached, continuing ...")
      return true;
    }
  }

  if (TPCOptUtils::isLoadTensor(I)) {
    CLUSTER_INFO_DEBUG(Indent << "- is a Load")
    auto It = InstClusterMap.find(I);
    if (It != InstClusterMap.end()) {
      CLUSTER_INFO_DEBUG(Indent << "- already visited (Cluster #" << It->second
                                << ")")
      ClusterUF.unionNodes(ClusterIdx, It->second);
      return true;
    }
    InstClusterMap.insert(std::make_pair(I, ClusterIdx));
    HeaderLoadInstsSet.insert(I);
    UnclusteredInstructions.erase(I);
  } else if (AccumPhiMap.find(I) != AccumPhiMap.end()) {
    CLUSTER_INFO_DEBUG(Indent << "- is an Accumulator, continuing ...")
    UnclusteredInstructions.erase(I);
    return true;
  } else { // assume compute
    CLUSTER_INFO_DEBUG(Indent << "- is a Compute")
    auto It = InstClusterMap.find(I);
    if (It != InstClusterMap.end()) {
      CLUSTER_INFO_DEBUG(Indent << "- already visited (Cluster #" << It->second
                                << ")")
      ClusterUF.unionNodes(ClusterIdx, It->second);
      return true;
    }
    InstClusterMap.insert(std::make_pair(I, ClusterIdx));
    HeaderComputeInstsSet.insert(I);
    for (unsigned OpIdx = 0; OpIdx < I->getNumOperands(); OpIdx++) {
      if (auto *OpDef = dyn_cast<Instruction>(I->getOperand(OpIdx))) {
        if (!recursivelyMarkClusters(OpDef, ClusterIdx, AccumPhiMap,
                                     UnclusteredInstructions, ClusterUF,
                                     Indent)) {
          CLUSTER_INFO_DEBUG(Indent
                             << "recursive cluster marking failed, exiting ...")
          return false;
        }
      }
    }
    UnclusteredInstructions.erase(I);
  }

  return true;
}

bool IterationClusterizer::markClustersForInsts(
    InstNumMapType &InstBBNumMap, InstructionVecType &OutInstList,
    InstInstMapType &AccumPhiMap, InstructionSetType &UnclusteredInstructions,
    std::string Indent) {
  std::string DebugTag = "<markClustersForInsts> ";
  CLUSTER_INFO_DEBUG(DebugTag << "Marking clusters for instructions")
  // make sure the Out Instructions are in original order as in IR BasicBlock
  sort(OutInstList.begin(), OutInstList.end(),
       [&](Instruction *I1, Instruction *I2) -> bool {
         return (InstBBNumMap[I1] < InstBBNumMap[I2]);
       });

  unsigned N = OutInstList.size();
  UnionFind ClusterUF(N);

  for (unsigned i = 0; i < N; i++) {
    CLUSTER_INFO_DEBUG(DebugTag << "\tPreCluster #" << i)
    Instruction *OutInst = OutInstList[i];
    CLUSTER_INFO_DEBUG(DebugTag << "OutInst = " << *OutInst)
    InstClusterMap.insert(std::make_pair(OutInst, i));
    unsigned StartIdx = 0, EndIdx = OutInst->getNumOperands();
    if (!IsAccumulationLoop)
      StartIdx = 1; // st_tnsr's 0'th operand is Coord
    // TODO : update this range once income-pred sequence for store is supported
    // too
    for (unsigned OpIdx = StartIdx; OpIdx < EndIdx; OpIdx++) {
      if (auto *OpDef = dyn_cast<Instruction>(OutInst->getOperand(OpIdx))) {
        if (!recursivelyMarkClusters(OpDef, i, AccumPhiMap,
                                     UnclusteredInstructions, ClusterUF,
                                     DebugTag)) {

          CLUSTER_INFO_DEBUG(DebugTag
                             << "recursive cluster marking failed, exiting ...")
          return false;
        }
      }
    }
    // OutInst should not be retained in the set of UnclusteredInstructions
    UnclusteredInstructions.erase(OutInst);
  }

  CLUSTER_INFO_DEBUG(DebugTag << "Mapping unique clusters ...")
  // retain only the actual roots as cluster ids, and get a mapping from each
  // instruction to it's final cluster idx
  NumClusters = ClusterUF.mapUniqueNodes(ClusterRootMap);

  return true;
}

void IterationClusterizer::populateClusterInfo(
    InstNumMapType &InstBBNumMap, InstructionVecType &OutInstList,
    ClusterInfoVecType &LoopClusters) {
  assert(NumClusters == LoopClusters.size() &&
         "Invalid function invocation : LoopClusters size should match the "
         "number of clusters");

  unsigned N = OutInstList.size();
  for (unsigned i = 0; i < N; i++) {
    Instruction *OutInst = OutInstList[i];
    unsigned ClusterIdx = ClusterRootMap[i];
    auto &ClusterOutInstsVec =
        (IsAccumulationLoop ? LoopClusters[ClusterIdx]->HeaderAccumInstsVec
                            : LoopClusters[ClusterIdx]->HeaderStoreInstsVec);
    ClusterOutInstsVec.push_back(OutInst);
  }

  // populate vectors of load insts
  for (auto LoadInst : HeaderLoadInstsSet) {
    unsigned ClusterIdx = ClusterRootMap[InstClusterMap[LoadInst]];
    LoopClusters[ClusterIdx]->HeaderLoadInstsVec.push_back(LoadInst);
  }

  // populate vectors of compute insts
  for (auto ComputeInst : HeaderComputeInstsSet) {
    unsigned ClusterIdx = ClusterRootMap[InstClusterMap[ComputeInst]];
    LoopClusters[ClusterIdx]->HeaderComputeInstsVec.push_back(ComputeInst);
  }

  for (unsigned ClusterIdx = 0; ClusterIdx < NumClusters; ClusterIdx++) {
    sort(LoopClusters[ClusterIdx]->HeaderLoadInstsVec.begin(),
         LoopClusters[ClusterIdx]->HeaderLoadInstsVec.end(),
         [&](Instruction *I1, Instruction *I2) -> bool {
           return InstBBNumMap[I1] < InstBBNumMap[I2];
         });
    sort(LoopClusters[ClusterIdx]->HeaderComputeInstsVec.begin(),
         LoopClusters[ClusterIdx]->HeaderComputeInstsVec.end(),
         [&](Instruction *I1, Instruction *I2) -> bool {
           return InstBBNumMap[I1] < InstBBNumMap[I2];
         });
  }
}

void IterationClusterizer::collectAccumulatorPhis(
    InstructionSetType &AccumPhis, InstInstMapType &AccumToPhiMap,
    std::string DebugTag) {
  CLUSTER_INFO_DEBUG(DebugTag << "Collecting Accumulator Phis")
  Type *T5xi32 =
      FixedVectorType::get(Type::getInt32Ty(HeaderBlock->getContext()), 5);
  for (auto &PhiNode : HeaderBlock->phis()) {
    Instruction *I = cast<Instruction>(&PhiNode);
    CLUSTER_INFO_DEBUG(DebugTag << *I)
    // the phi must have a incoming value from Header block
    auto *HeaderIncome = PhiNode.getIncomingValueForBlock(HeaderBlock);
    if (!HeaderIncome) {
      CLUSTER_INFO_DEBUG(
          DebugTag << "\tNot an AccumPhi - No incoming value from Header block")
      continue;
    }
    // skip coord phis
    if (T5xi32 == HeaderIncome->getType()) {
      CLUSTER_INFO_DEBUG(DebugTag << "\tNot an AccumPhi - is a Coord Phi")
      continue;
    }
    // skip induction phis
    if (InductionDescriptor::isInductionPHI(&PhiNode, WorkingLoop, SE,
                                            InductionDesc)) {
      CLUSTER_INFO_DEBUG(DebugTag << "\tNot an AccumPhi - is an InductionPhi")
      continue;
    }

    // is an accumulator phi
    CLUSTER_INFO_DEBUG(DebugTag << "\t- is an AccumPhi")
    AccumPhis.insert(I);
    AccumToPhiMap.insert(std::make_pair(I, I));
  }
  return;
}

bool IterationClusterizer::markAccumulatorInsts(
    InstructionVecType &HeaderAccumList, InstructionSetType &AccumPhis,
    InstInstMapType &AccumToPhiMap, std::string DebugTag) {
  CLUSTER_INFO_DEBUG(DebugTag << "Marking accumulator instructions ...")
  for (auto AccumPhi : AccumPhis) {
    CLUSTER_INFO_DEBUG(DebugTag << "AccumPhi = " << *AccumPhi)
    Instruction *CurAccum = AccumPhi;
    while (CurAccum) {
      Instruction *NextAccum = nullptr;
      CLUSTER_INFO_DEBUG(DebugTag << "CurAccum = " << *CurAccum)
      for (auto User : CurAccum->users()) {
        Instruction *UserInst = cast<Instruction>(User);
        // ignore all Phis, and the Insts of other basic blocks
        if (dyn_cast<PHINode>(UserInst) || UserInst->getParent() != HeaderBlock)
          continue;
        if (NextAccum) { // if already the next accum was found
          CLUSTER_INFO_DEBUG(
              DebugTag
              << "Multiple uses of Accum Phi not handled yet, exiting ...")
          return false;
        }
        NextAccum = UserInst;
      }

      if (NextAccum) {
        CLUSTER_INFO_DEBUG(DebugTag << "NextAccum = " << *NextAccum)
        HeaderAccumList.push_back(NextAccum);
        AccumToPhiMap.insert(std::make_pair(NextAccum, AccumPhi));
      }
      CurAccum = NextAccum;
    }
  }
  return true;
}

bool IterationClusterizer::classifyInstructions(
    InstructionVecType &HeaderStoreList, InstructionVecType &HeaderLoadList,
    InstructionVecType &HeaderAccumList,
    InstructionSetType &UnclusteredInstructions, InstNumMapType &InstBBNumMap,
    InstructionSetType &AccumPhis, InstInstMapType &AccumToPhiMap,
    std::string DebugTag) {
  // start collecting the stores from the last instruction
  CLUSTER_INFO_DEBUG(DebugTag
                     << "Categorizing the HeaderBlock instructions ...")
  unsigned count = 0;
  for (Instruction &Inst : *HeaderBlock) {
    auto *I = &Inst;
    if (TPCOptUtils::isStoreTensor(I)) {
      CLUSTER_INFO_DEBUG(DebugTag << "{ Store }" << *I)
      HeaderStoreList.push_back(I);
    } else if (TPCOptUtils::isLoadTensor(I)) {
      CLUSTER_INFO_DEBUG(DebugTag << "{ Load }" << *I)
      HeaderLoadList.push_back(I);
      UnclusteredInstructions.insert(I);
    } else {
      CLUSTER_INFO_DEBUG(DebugTag << "{ Compute }" << *I)
      UnclusteredInstructions.insert(I);
    }
    InstBBNumMap.insert(std::make_pair(&Inst, count));
    count += 1;
  }

  if (SkipIfZeroLoadTensors && HeaderLoadList.size() == 0) {
    CLUSTER_INFO_DEBUG(
        DebugTag << "No 'LoadTensor' instructions were found. exiting ...")
    return false;
  }

  if (!HeaderStoreList.size()) {
    // probably a reduction loop
    // check for accumulators
    collectAccumulatorPhis(AccumPhis, AccumToPhiMap, DebugTag + "\t");
    if (!AccumPhis.size()) {
      CLUSTER_INFO_DEBUG(
          DebugTag << "Non-accumulator loop not yet supported, exiting ...")
      return false;
    }
    IsAccumulationLoop = true;
    if (!markAccumulatorInsts(HeaderAccumList, AccumPhis, AccumToPhiMap,
                              DebugTag + "\t")) {
      CLUSTER_INFO_DEBUG(
          DebugTag << "Accumulator instruction marking failed, exiting ...")
      return false;
    }
  }
  return true;
}

bool IterationClusterizer::clusterInstructions(
    InstructionVecType &HeaderStoreList, InstructionVecType &HeaderLoadList,
    InstructionVecType &HeaderAccumList,
    InstructionSetType &UnclusteredInstructions, InstNumMapType &InstBBNumMap,
    InstructionSetType &AccumPhis, InstInstMapType &AccumToPhiMap,
    ClusterInfoVecType &LoopClusters, std::string DebugTag) {

  if (ShouldDetectPipelinedLoads && !PreheaderBlock) {
    CLUSTER_INFO_DEBUG(DebugTag << "Invalid Preheader block, exiting ...")
    return false;
  }

  auto &OutInstList =
      (getIsAccumulationLoop() ? HeaderAccumList : HeaderStoreList);
  if (!markClustersForInsts(InstBBNumMap, OutInstList, AccumToPhiMap,
                            UnclusteredInstructions)) {
    CLUSTER_INFO_DEBUG(
        DebugTag << "Marking Clusters for Instruction failed, exiting ...")
    return false;
  }
  CLUSTER_INFO_DEBUG(DebugTag << "Number of Clusters Found = "
                              << getNumClusters())

  if (getNumClusters() < 1) {
    CLUSTER_INFO_DEBUG(DebugTag << "No cluster was identifed, exiting ...")
    return false;
  }

  if (PopulateIfMultiCluster && getNumClusters() == 1) {
    CLUSTER_INFO_DEBUG(DebugTag << "Single cluster found, exiting ...")
    return false;
  }

  for (unsigned C = 0; C < getNumClusters(); C++)
    LoopClusters.push_back(new ClusterInfo());
  populateClusterInfo(InstBBNumMap, OutInstList, LoopClusters);

  return true;
}

bool IterationClusterizer::classifyAndClusterize(
    ClusterInfoVecType &LoopClusters, InstructionVecType &HeaderStoreList,
    InstructionVecType &HeaderLoadList,
    InstructionSetType &UnclusteredInstructions, InstructionSetType &AccumPhis,
    InstInstMapType &AccumToPhiMap) {
  std::string DebugTag = "<classifyAndClusterize> ";

  // HeaderAccumList is a temporary container to hold the list of all
  // accumulator instructions of the loop
  InstructionVecType HeaderAccumList;

  // InstBBNumMap is a mapping from Instruction to it's original relative
  // position (unsigned) within the block, which will be first populated by the
  // classifier, and then consumed by clusterizer to sort the Instructions
  // within the cluster-info object in the original order.
  InstNumMapType InstBBNumMap;

  if (!classifyInstructions(HeaderStoreList, HeaderLoadList, HeaderAccumList,
                            UnclusteredInstructions, InstBBNumMap, AccumPhis,
                            AccumToPhiMap, DebugTag + "\t")) {
    CLUSTER_INFO_DEBUG(DebugTag
                       << "Could not classify Instructions, exiting ...")
    return false;
  }

  if (!clusterInstructions(HeaderStoreList, HeaderLoadList, HeaderAccumList,
                           UnclusteredInstructions, InstBBNumMap, AccumPhis,
                           AccumToPhiMap, LoopClusters, DebugTag + "\t")) {
    CLUSTER_INFO_DEBUG(DebugTag
                       << "Could not clusterize Instructions, exiting ...")
    return false;
  }

  // print cluster debug info
  LLVM_DEBUG(IterationClusterizer::dumpClusterInfo(LoopClusters,
                                                   UnclusteredInstructions););

  return true;
}

bool IterationClusterizer::classifyAndClusterize(
    ClusterInfoVecType &LoopClusters) {
  // HeaderStoreList and HeaderLoadList are temporary containers to hold the
  // list of all store and load instructions of the loop, resp.
  InstructionVecType HeaderStoreList;
  InstructionVecType HeaderLoadList;
  // UnclusteredInstructions is a set of Instruction used to hold all loop
  // instructions that do not belong to any cluster
  // These could be any loop structure instructions like branching, induction,
  // coord updates etc.
  InstructionSetType UnclusteredInstructions;
  // This set is used to collect all the accumulator phis in the loop
  InstructionSetType AccumPhis;
  // This is a map from an accumulator instructions to it's origin accumulator
  // Phi
  InstInstMapType AccumToPhiMap;

  return classifyAndClusterize(LoopClusters, HeaderStoreList, HeaderLoadList,
                               UnclusteredInstructions, AccumPhis,
                               AccumToPhiMap);
}

#undef DEBUG_TYPE
