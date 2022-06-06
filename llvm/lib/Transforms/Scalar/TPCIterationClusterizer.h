// === TPCIterationClusterizer.h : Clusteriztion for TPC Innermost Loops === //
//
//
//                     The LLVM Compiler Infrastructure:
//
//                2021 - This pass is a property of Habana labs
//
// Author : Vinay V. Vasista
// Email  : vvasista@habana.ai
//
// ========================================================================= //


#ifndef LLVM_TRANSFORM_SCALAR_TPC_ITER_CLUSTERIZER_H
#define LLVM_TRANSFORM_SCALAR_TPC_ITER_CLUSTERIZER_H

#include "TPCOptUtils.h"
#include "llvm/Analysis/IVDescriptors.h"

namespace llvm {

#define DEBUG_TYPE "clusterizer"

#define CLUSTER_INFO_DEBUG(x)                                                  \
  LLVM_DEBUG(dbgs() << "[Clusterizer] " << x << "\n");

struct ClusterInfo {
  InstructionVecType HeaderLoadInstsVec;
  InstructionVecType HeaderStoreInstsVec;
  InstructionVecType HeaderAccumInstsVec;
  InstructionVecType HeaderComputeInstsVec;
};

using ClusterInfoVecType = SmallVector<ClusterInfo *, 4>;

class IterationClusterizer {
public:
  IterationClusterizer(Loop *WorkingLoop, ScalarEvolution *SE,
                       bool PopulateIfMultiCluster = false,
                       bool SkipIfZeroLoadTensors = false,
                       bool ShouldDetectPipelinedLoads = false);

  unsigned getNumClusters() { return NumClusters; }

  bool getIsAccumulationLoop() { return IsAccumulationLoop; }

  // This function classifies the HeaderBlock instructions as load / store /
  // compute / accum instructions, and creates a vector of clusters
  // representing the atomic iterations of the loop block
  bool classifyAndClusterize(ClusterInfoVecType &LoopClusters,
                             InstructionVecType &HeaderStoreList,
                             InstructionVecType &HeaderLoadList,
                             InstructionSetType &UnclusteredInstructions,
                             InstructionSetType &AccumPhis,
                             InstInstMapType &AccumToPhiMap);
  // Same as above, with fewer parameters
  bool classifyAndClusterize(ClusterInfoVecType &LoopClusters);

  // follow use-def chain of store instruction to mark all instructions with
  // the final cluster idx they belong to
  bool markClustersForInsts(InstNumMapType &InstBBNumMap,
                            InstructionVecType &OutInstList,
                            InstInstMapType &AccumPhiMap,
                            InstructionSetType &UnclusteredInstructions,
                            std::string Indent = "");

  // populate the ClusterInfo vector using the information in the
  // Inst->ClusterIdx mapping
  void populateClusterInfo(InstNumMapType &InstBBNumMap,
                           InstructionVecType &OutInstList,
                           ClusterInfoVecType &LoopClusters);

  // classify Instructions as Load, Store, Compute and Accumulator Instructions
  bool classifyInstructions(InstructionVecType &HeaderStoreList,
                            InstructionVecType &HeaderLoadList,
                            InstructionVecType &HeaderAccumList,
                            InstructionSetType &UnclusteredInstructions,
                            InstNumMapType &InstBBNumMap,
                            InstructionSetType &AccumPhis,
                            InstInstMapType &AccumToPhiMap,
                            std::string DebugTag = "");

  // collect cluster information, create Inst -> ClusterIdx mapping and
  // populate ClusterInfo vector
  bool clusterInstructions(
      InstructionVecType &HeaderStoreList, InstructionVecType &HeaderLoadList,
      InstructionVecType &HeaderAccumList,
      InstructionSetType &UnclusteredInstructions, InstNumMapType &InstBBNumMap,
      InstructionSetType &AccumPhis, InstInstMapType &AccumToPhiMap,
      ClusterInfoVecType &LoopClusters, std::string DebugTag = "");

  // print cluster debug info
  static void dumpClusterInfo(ClusterInfoVecType &LoopClusters,
                              InstructionSetType &UnclusteredInstructions) {
    LLVM_DEBUG(
        CLUSTER_INFO_DEBUG("\n====\nInstructions not in any cluster :\n===="); {
          for (auto It : UnclusteredInstructions)
            CLUSTER_INFO_DEBUG(*It)
        }

        CLUSTER_INFO_DEBUG("\n====\nClusters :\n====");
        {
          for (unsigned i = 0; i < LoopClusters.size(); i++) {
            CLUSTER_INFO_DEBUG("#" << i)
            CLUSTER_INFO_DEBUG("---- Accumulators ----")
            for (auto OutInst : LoopClusters[i]->HeaderAccumInstsVec)
              CLUSTER_INFO_DEBUG(*OutInst)
            CLUSTER_INFO_DEBUG("---- Stores ----")
            for (auto OutInst : LoopClusters[i]->HeaderStoreInstsVec)
              CLUSTER_INFO_DEBUG(*OutInst)
            CLUSTER_INFO_DEBUG("---- Loads ----")
            for (auto LoadInst : LoopClusters[i]->HeaderLoadInstsVec)
              CLUSTER_INFO_DEBUG(*LoadInst)
            CLUSTER_INFO_DEBUG("---- Computes ----")
            for (auto ComputeInst : LoopClusters[i]->HeaderComputeInstsVec)
              CLUSTER_INFO_DEBUG(*ComputeInst)
          }
        } CLUSTER_INFO_DEBUG("===="););

    return;
  }

private:
  // collect all Accumulator Phis
  void collectAccumulatorPhis(InstructionSetType &AccumPhis,
                              InstInstMapType &AccumToPhiMap,
                              std::string DebugTag = "\t");

  // Scan the loop instructions that are users of Accumulator Phis and mark all
  // the Accumulator Insts
  bool markAccumulatorInsts(InstructionVecType &HeaderAccumList,
                            InstructionSetType &AccumPhis,
                            InstInstMapType &AccumToPhiMap,
                            std::string DebugTag = "");

  // recursively follow the use-def chain of given Instruction
  // terminate on coords phis and add mask instructions
  // if the def Instruction is newly discovered, include it to the cluster
  // else, merge the cluster of the def with that of the given Instruction
  bool recursivelyMarkClusters(Instruction *I, unsigned ClusterIdx,
                               InstInstMapType &AccumPhiMap,
                               InstructionSetType &UnclusteredInstructions,
                               UnionFind &ClusterUF, std::string Indent = "");

  Loop *WorkingLoop;
  BasicBlock *HeaderBlock;
  BasicBlock *PreheaderBlock;
  InductionDescriptor InductionDesc;
  ScalarEvolution *SE;

  bool PopulateIfMultiCluster;
  bool SkipIfZeroLoadTensors;
  bool ShouldDetectPipelinedLoads;
  unsigned NumClusters;
  bool IsAccumulationLoop;

  InstructionSetType HeaderLoadInstsSet;
  InstructionSetType HeaderComputeInstsSet;
  InstNumMapType InstClusterMap;
  UFNodeMapType ClusterRootMap;
};

#undef DEBUG_TYPE

} // end namespace llvm

#endif
