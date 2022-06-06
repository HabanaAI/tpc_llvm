//===- IterationInterleave.cpp - Interleave Iterations --------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2020 - This pass is a property of Habana labs
//
//
//===----------------------------------------------------------------------===//
//
// This file implements a transformation on unrolled loops to generate
// interleaved instructions across iterations within the unrolled body.
//
// Interleaving strategies
// =======================
// Isomorphic
// ----------
//  for (...; i += 4) {                      for (...; i += 4) {
//    a(i); b(i); c(i)                        a(i); a(i + 1); a(i + 2); a(i + 3)
//    a(i + 1); b(i + 1); c(i + 1)            b(i); b(i + 1); b(i + 2); b(i + 3)
//    a(i + 2); b(i + 2); c(i + 2)      =>    c(i); c(i + 1); c(i + 2); c(i + 3)
//    a(i + 3); b(i + 3); c(i + 3)          }
// }
//
// This strategy groups isomorphic instructions across iterations to hide the
// latency. IF or interleave-factor is an integral metric like vector-factor of
// the loop-vectorizer that denotes the number of iterations an isomorphic
// ordering correspond to. The cost-model tries to a set an IF that maximizes
// ILP without incurring high register pressure.
//
// SWP
// ---
// This strategy leverages software-pipelined loads which can be ordered for
// better "VLIW packetizing".  Since these loads are used by phi for the next
// iteration, we have more freedom in scheduling it compared to the
// non-software-pipelined loads.
//
//
//
// TODO
// ====
// - Develop a cost-model that decides the set of strategies to execute instead
//   of executing them serially.
//
// - Develop a cost-model for the SWP strategies.
//
// - Eliminate (or minimize) dependence on std::distance() on
//   BasicBlock::iterator. BasicBlock tracks it's instructions using
//   simple_ilist whose iterator doesn't support random-access. So
//   std::distance() will incur a linear time complexity.
//
//   Analysis/OrderedBasicBlock can solve the std::distance() problem if the
//   BasicBlock is read-only. Unfortunately this pass does a non-trivial
//   reordering and uses std::distance() (for querying dominance relations,
//   live-ranges etc.) during the reordering.
//
// - Teach LLVM about TPC memory dependences (i.e. teach
//   DependenceInfo::depends() and/or MemorySSA etc.) so that we can bail out if
//   the pass violates the memory access pattern on the same address. This
//   problem exist for any transformation that reorders ld_XXX and/or st_XXX in
//   TPC (like TPC/TPCoptimization, Scalar/LoopSWP etc.)
//===----------------------------------------------------------------------===//

#include "TPCIterationClusterizer.h"
#include "TPCOptUtils.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Scalar.h"
#include <deque>
#include <iterator>
#include <list>
#include <memory>

using namespace llvm;

#define DEBUG_TYPE "iter-ilv"

STATISTIC(NumInstsInterleaved, "Number of instructions interleaved");
STATISTIC(NumLoopsInterleaved, "Number of loops interleaved");

// Options to override the cost-model's IF. Interleaving can be disabled for a
// container by setting it's IF = 0.
static cl::opt<int> CoordUpdateIFOpt("iter-ilv-coord-update-IF", cl::init(-1),
                                     cl::Hidden);
static cl::opt<int> LoadIFOpt("iter-ilv-load-IF", cl::init(-1), cl::Hidden);
static cl::opt<int> ComputeIFOpt("iter-ilv-compute-IF", cl::init(-1),
                                 cl::Hidden);
static cl::opt<int> StoreIFOpt("iter-ilv-store-IF", cl::init(-1), cl::Hidden);

static cl::opt<bool>
    ElimFalseCoordUpdateDepsOpt("iter-ilv-elim-false-coord-update-deps",
                                cl::desc("Eliminate false coord-update deps"),
                                cl::Hidden, cl::init(false));

static cl::opt<bool> TrimCoordUpdateLiveRangeOpt(
    "iter-ilv-trim-coord-update-live-ranges",
    cl::desc("Move coord-updates closer to their first use"), cl::Hidden,
    cl::init(false));

// TODO: Move to Instruction
/// Returns true if \p I is an intrinsic of ID \p IntrinsicID
static bool isIntrinsicOfID(const Instruction *I, Intrinsic::ID IntrinsicID) {
  if (const auto *Intrinsic = dyn_cast<IntrinsicInst>(I))
    return (Intrinsic->getIntrinsicID() == IntrinsicID);
  return false;
}

// TODO: Move to TPCUtils
/// Returns true if \p I is a TPC coord-update
static bool isCoordUpdate(const Instruction *I) {
  if (isIntrinsicOfID(I, Intrinsic::tpc_add_mask))
    return true;

  if (isIntrinsicOfID(I, Intrinsic::tpc_set_indx))
    return true;

  const auto *II = dyn_cast<InsertElementInst>(I);
  if (II) {
    Type *Int5Ty = FixedVectorType::get(Type::getInt32Ty(I->getContext()), 5);
    if (Int5Ty == II->getType()) {
      return true;
    }
  }

  return false;
}

// TODO: Move to BasicBlock {
/// Returns true if the instruction \p A comes before \p B in the BasicBlock
/// assuming that both \p A and \p B belong to the same BasicBlock
static bool instComesBeforeInBB(const Instruction *A, const Instruction *B) {
  assert(A->getParent() == B->getParent());
  const BasicBlock::const_iterator BBBegin = A->getParent()->begin();
  BasicBlock::const_iterator AIter = A->getIterator();
  BasicBlock::const_iterator BIter = B->getIterator();
  unsigned APos = std::distance(BBBegin, AIter);
  unsigned BPos = std::distance(BBBegin, BIter);
  return APos <= BPos ? true : false;
}

/// Prepends only the broken SSA dependence DAG sub-graph rooted at \p Inst
/// before \p RefInst pruned at BB boundaries in post-order (i.e. we're
/// recursively fixing the operands within the BB). This is expected to be
/// called if you moved \p Inst up in it's BB without fixing it's SSA
/// dependence(s)
static void fixNonPHIDAGBelow(Instruction *Inst, Instruction *RefInst) {
  assert(!isa<PHINode>(Inst));
  assert(Inst->getParent() == RefInst->getParent());

  for (Value *Opnd : Inst->operands()) {
    Instruction *OpndDef = dyn_cast<Instruction>(Opnd);
    if (!OpndDef)
      continue;

    if (OpndDef->getParent() != RefInst->getParent())
      continue;

    if (isa<PHINode>(OpndDef))
      continue;

    // If `OpndDef` already dominates `RefInst`
    if (instComesBeforeInBB(OpndDef, RefInst))
      continue;

    fixNonPHIDAGBelow(OpndDef, RefInst);
    OpndDef->moveBefore(RefInst);
  }
}

/// Appends only the broken SSA dependence DAG reachable "above" \p Inst (i.e.
/// the users) after \p RefInst pruned at BB boundaries in post-order (i.e.
/// we're recursively fixing the users within the BB). This is expected to be
/// called if you moved \p Inst down in it's BB without fixing it's SSA
/// dependence(s)
static void fixNonPHIDAGAbove(Instruction *Inst, Instruction *RefInst) {
  assert(!isa<PHINode>(Inst));
  assert(Inst->getParent() == RefInst->getParent());

  for (Value *Usr : Inst->users()) {
    Instruction *UsrDef = dyn_cast<Instruction>(Usr);
    if (!UsrDef)
      continue;

    if (UsrDef->getParent() != RefInst->getParent())
      continue;

    if (isa<PHINode>(UsrDef))
      continue;

    // If `UsrDef` already post-dominates `RefInst`
    if (!instComesBeforeInBB(UsrDef, RefInst))
      continue;

    fixNonPHIDAGAbove(UsrDef, RefInst);
    UsrDef->moveAfter(RefInst);
  }
}

/// Moves non-PHI \p Inst after \p RefInst assuming they're both in the same BB
/// and fixes their SSA dependence(s) post movement.
static void moveNonPHIInBBAfter(Instruction *Inst, Instruction *RefInst) {
  assert(!isa<PHINode>(Inst));
  assert(Inst->getParent() == RefInst->getParent());

  if (Inst == RefInst)
    return;

  // If we need to move `Inst` up
  if (instComesBeforeInBB(RefInst, Inst)) {
    Inst->moveAfter(RefInst);
    fixNonPHIDAGBelow(Inst, /* RefInst */ Inst);

  } else {
    Inst->moveAfter(RefInst);
    fixNonPHIDAGAbove(Inst, /* RefInst */ Inst);
  }
}

/// Returns the first non-PHI user of \p Inst in it's BB if any. Returns nullptr
/// if \p Inst has no user in it's BB.
static Instruction *getFirstNonPHIUserInBB(const Instruction *Inst) {
  const Instruction *FirstUser = nullptr;

  for (const User *Usr : Inst->users()) {
    const Instruction *UsrInst = dyn_cast<Instruction>(Usr);
    if (!UsrInst) {
      continue;

    } else if (Inst->getParent() != UsrInst->getParent()) {
      continue;

    } else if (isa<PHINode>(UsrInst)) {
      continue;

    } else if (!FirstUser || instComesBeforeInBB(UsrInst, FirstUser)) {
      FirstUser = UsrInst;
    }
  }

  return const_cast<Instruction *>(FirstUser);
}

// } Move to BasicBlock

// TODO: The following code is copied from LoopSWP primarily authored by
// @vvasista and modified for iter-ilv specific requirements.
// {
namespace {

class ClusterInfo {
public:
  ScalarEvolution *SE;
  Loop *WorkingLoop;

  ClusterInfo(Loop *L, ScalarEvolution *SE) : SE(SE), WorkingLoop(L) {}

  unsigned NumClusters = 0;
  bool IsAccumulationLoop = false;

  VecInstVecType HeaderCoordUpdateInstsVec;
  VecInstVecType HeaderComputeInstsVec;
  VecInstVecType HeaderLoadInstsVec;
  VecInstVecType HeaderStoreInstsVec;
  VecInstVecType HeaderAccumInstsVec;

  // set/get number of clusters
  void setNumClusters(unsigned N) { NumClusters = N; }
  unsigned getNumClusters() { return NumClusters; }

  // get/set whether the loop has accumulator
  void setIsAccumulationLoop(bool IsAccumLoop) {
    IsAccumulationLoop = IsAccumLoop;
  }
  bool getIsAccumulationLoop() { return IsAccumulationLoop; }

  ClusterInfoVecType LoopClusters;
  bool clusterInstructions() {
    IterationClusterizer Clusterizer(WorkingLoop, SE,
                                     /* PopulateIfMultiCluster */ false,
                                     /* SkipIfZeroLoadTensors */ false,
                                     /* ShouldDetectPipelinedLoads */ true);
    if (!Clusterizer.classifyAndClusterize(LoopClusters))
      return false;

    setNumClusters(Clusterizer.getNumClusters());
    setIsAccumulationLoop(Clusterizer.getIsAccumulationLoop());

    for (const llvm::ClusterInfo *LoopCluster : LoopClusters) {
      HeaderLoadInstsVec.push_back(LoopCluster->HeaderLoadInstsVec);
      HeaderComputeInstsVec.push_back(LoopCluster->HeaderComputeInstsVec);
      HeaderStoreInstsVec.push_back(LoopCluster->HeaderStoreInstsVec);
      HeaderAccumInstsVec.push_back(LoopCluster->HeaderAccumInstsVec);
    }

    return true;
  }
};

// } code copied from LoopSWP primarily authored by @vvasista

class IterationInterleaver {
  ScalarEvolution *SE;
  const TargetTransformInfo *TTI;
  std::unique_ptr<ClusterInfo> CI;

  /// TheLoopXXX refers to the entities of the working loop
  Loop *TheLoop;
  BasicBlock *TheLoopHeader;

  /// List of coord-updates, loads and stores in `TheLoop`
  SmallVector<Instruction *, 4> CoordUpdates, Loads, Stores;

  /// unroll-factor of `TheLoop`
  unsigned UF = 1;

  /// The following data-structures are more or less 2 different perspectives
  /// to clusters. Given a loop that's unrolled UF times, it's clusters can be
  /// seen as:
  /// cluster[0] {
  ///   foo inst-container
  ///   bar inst-container
  ///   ...
  /// }
  ///
  /// cluster[1] {
  ///   ...
  /// }
  ///
  /// ...
  /// cluster[UF - 1] {
  ///   ...
  /// }
  ///
  /// The `ContainersOfInterest` help in interleaving while the
  /// `ClusterSetsOfInterest` help in reasoning about inter-cluster dependencies
  /// but they both refer to the same underlying data.

  struct Container {
    const VecInstVecType &InstLists;
    enum class Type { CoordUpdate, Load, Compute, Store, Unknown };
    Type Ty;

    Type getType() const { return Ty; }
    bool isCoordUpdateType() const { return (Ty == Type::CoordUpdate); }
    bool isLoadType() const { return (Ty == Type::Load); }
    bool isComputeType() const { return (Ty == Type::Compute); }
    bool isStoreType() const { return (Ty == Type::Store); }

    static Type getTypeFor(const Instruction *Inst) {
      if (isCoordUpdate(Inst)) {
        return Type::CoordUpdate;

      } else if (isIntrinsicOfID(Inst, Intrinsic::tpc_ld_tnsr)) {
        return Type::Load;

      } else if (isIntrinsicOfID(Inst, Intrinsic::tpc_st_tnsr)) {
        return Type::Store;
      }

      if (const auto *Intrinsic = dyn_cast<IntrinsicInst>(Inst)) {
        if (Intrinsic->getName().startswith("llvm.tpc"))
          return Type::Compute;
      }

      return Type::Unknown;
    }

    static const char *getTypeAsString(Type Ty) {
      switch (Ty) {
      case Type::CoordUpdate:
        return "coord-update";

      case Type::Load:
        return "load";

      case Type::Compute:
        return "compute";

      case Type::Store:
        return "store";

      case Type::Unknown:
        return "unknown";
      }
    }

    const char *getTypeAsString() const { return getTypeAsString(getType()); }

    unsigned getNumIteration() const { return InstLists.size(); }

    unsigned getNumInstPerIteration() const { return InstLists[0].size(); }

    unsigned getNumInst() const {
      return getNumInstPerIteration() * getNumIteration();
    }

#ifndef NDEBUG

    void print(raw_ostream &OS) const {
      OS << getTypeAsString() << "-container";
    }

    LLVM_DUMP_METHOD void dump() const { print(dbgs()); }

    friend raw_ostream &operator<<(raw_ostream &OS, const Container &Cont) {
      Cont.print(OS);
      return OS;
    }

#endif

    explicit Container(const VecInstVecType &InstLists, Type Ty)
        : InstLists(InstLists), Ty(Ty) {}
  };

  /// Containers that'll be subject to interleaving
  std::list<Container> ContainersOfInterest;

  /// Returns the container of type \p Ty from \p ContainersOfInterest if
  /// present (else returns nullptr)
  Container *getContainerOfInterest(Container::Type Ty) const {
    for (const Container &Cont : ContainersOfInterest) {
      if (Cont.getType() == Ty)
        return const_cast<Container *>(&Cont);
    }

    return nullptr;
  }

  /// Adds a container corresponding to \p InstLists of type \p Ty to \p
  /// ContainersOfInterest if it's valid.
  void addContainerOfInterest(VecInstVecType &InstLists, Container::Type Ty) {
    if (!InstLists.size()) {
      LLVM_DEBUG(dbgs() << "iter-ilv: Not adding "
                        << Container::getTypeAsString(Ty)
                        << "-container since it's empty\n");
      return;
    }

    if (InstLists.size() != UF) {
      LLVM_DEBUG(
          dbgs()
          << "iter-ilv: Not adding " << Container::getTypeAsString(Ty)
          << "-container since it doesn't represent UF number of clusters\n");
      return;
    }

    unsigned RefSize = InstLists[0].size();
    for (const InstructionVecType &InstList : InstLists) {
      if (RefSize != InstList.size()) {
        LLVM_DEBUG(
            dbgs()
            << "iter-ilv: Not adding " << Container::getTypeAsString(Ty)
            << "-container since it has uneven number of instructions\n");
        return;
      }
    }

    if (!RefSize) {
      LLVM_DEBUG(
          dbgs() << "iter-ilv: Not adding " << Container::getTypeAsString(Ty)
                 << "-container since there are no instructions under it\n");
    }

    ContainersOfInterest.emplace_back(InstLists, Ty);
  }

  /// Removes the container of type \p Ty from \p ContainersOfInterest if
  /// present
  void removeContainerOfInterest(Container::Type Ty) {
    for (auto I = ContainersOfInterest.begin(), E = ContainersOfInterest.end();
         I != E; ++I) {
      Container Cont = *I;
      if (Cont.getType() == Ty) {
        ContainersOfInterest.erase(I);
        return;
      }
    }
  }

  /// Removes the container \p Cont from \p ContainersOfInterest if present
  void removeContainerOfInterest(const Container &Cont) {
    removeContainerOfInterest(Cont.getType());
  }

  /// Cluster sets that'll be subject to interleaving
  SmallVector<SmallSet<Instruction *, 4>, 4> ClusterSetsOfInterest;

  class IsomorphicCostModel {
    const TargetTransformInfo *TTI;
    unsigned UF;
    const std::list<Container> &ContainersOfInterest;
    std::map<Container::Type, int> ContainerIFMap;

    // Note:
    // - The client is expected to override any parameter from this
    //   cost-model.  i.e., for example, the CostModel doesn't care about
    //   external parameters that could override IF like a user enforced IF via
    //   cl::opt.
    //
    // - This cost-model makes "local" decision like deciding the IF for each
    //   containers, i.e. it doesn't make "global" decisions like "don't
    //   interleave at all". The "global" decisions are expected to be done by
    //   the profitability checks outside (like
    //   IterationInterleaver::isReasonable())

    /// Returns the preferred IF for the load-container \p Cont.
    unsigned calculateLoadIF(const Container &Cont) const {
      assert(Cont.isLoadType());
      unsigned PreferredIF = UF;

      return PreferredIF;
    }

    /// Returns the preferred IF for the coord-update-container \p Cont.
    unsigned calculateCoordUpdateIF(const Container &Cont) const {
      assert(Cont.isCoordUpdateType());
      unsigned PreferredIF = 0;

      return PreferredIF;
    }

    /// Return true if two-addr pass would create copies for \p I
    bool generatesTwoAddrCopy(const Instruction *I) const {
      const IntrinsicInst *Intrinsic = dyn_cast<IntrinsicInst>(I);
      if (!Intrinsic)
        return false;

      // FIXME: This list is not complete. Is there some other way to fetch this
      // info in opt?
      switch (Intrinsic->getIntrinsicID()) {
      case Intrinsic::tpc_mac:
      case Intrinsic::tpc_mac_x2:
      case Intrinsic::tpc_mac_x2_f32:
        return true;
      }

      return false;
    }

    /// Returns the preferred IF for the compute-container \p Cont.
    unsigned calculateComputeIF(const Container &Cont) const {
      assert(Cont.isComputeType());
      assert(Cont.InstLists.size() > 0);

      // IFVotes[<n>] for IF = <n> represents the preference (in terms of number
      // of votes (see below)) of <n> as the IF.
      SmallVector<unsigned, 8> IFVotes;
      IFVotes.resize(UF + 1);
      for (unsigned IF = 0; IF <= UF; IF++) {
        IFVotes[IF] = 0;
      }

      // We iterate the compute instructions and use heuristic to decide the
      // vote for a particular IF. Some instructions are allowed to cheat in the
      // voting if the heuristic deems it reasonable. This method is holistic
      // since every instruction can cast a vote on the compute-container's IF
      // but it's still far from perfect due to the reliance on heuristics.
      const InstructionVecType &InstList = Cont.InstLists[0];
      for (const Instruction *Inst : InstList) {
        // Be pessimistic if there are intrinsics that will be expanded to
        // multiple instructions. (TODO: These must be handled in TTI's
        // getInstructionCost() in Target/TPC/TPCTargetTransformInfo)
        if (const auto *Intrinsic = dyn_cast<IntrinsicInst>(Inst)) {
          switch (Intrinsic->getIntrinsicID()) {
          case Intrinsic::tpc_fpext_swch:
          case Intrinsic::tpc_fptrunc_swch:
          case Intrinsic::tpc_fptosi_swch:
          case Intrinsic::tpc_fptoui_swch:
          case Intrinsic::tpc_sitofp_swch:
          case Intrinsic::tpc_uitofp_swch:
            IFVotes[0] += 5;
            continue;
          }
        }

        int InstLatency =
            *TTI->getInstructionCost(Inst, TargetTransformInfo::TCK_Latency).getValue();
        if (InstLatency == 0)
          continue;

        if (Inst->getNumUses() != 1)
          continue;

        const Instruction *FirstUser = getFirstNonPHIUserInBB(Inst);
        if (!FirstUser)
          continue;

        unsigned LiveRangeDist =
            std::distance(Inst->getIterator(), FirstUser->getIterator());

        // Calculate `IF` such that:
        // - `UF` >= `IF`
        // - `InstLatency` is directly proportional to `IF`
        // - `LiveRangeDist` is inversely proportional to `IF`
        unsigned IF = InstLatency / LiveRangeDist;
        if (IF < UF && LiveRangeDist == 1) {
          IF = UF;
        }

        IF = IF > UF ? UF : IF;

        IFVotes[IF]++;

        // If two-addr pass would generate copies for `Inst` and `Inst` prefers
        // IF = 0 (i.e. no interleaving), then be pessimistic and give a high
        // preference for IF = 0.
        if (IF == 0 && generatesTwoAddrCopy(Inst)) {
          IFVotes[IF] += 5;
        }
      }

      unsigned PreferredIF = 0, MaxVotes = 0;
      LLVM_DEBUG(
          dbgs() << "iter-ilv::isomorphicCM: Compute-container's IF-votes:\n");
      for (unsigned IF = 0; IF <= UF; IF++) {
        if (!IFVotes[IF])
          continue;

        LLVM_DEBUG(dbgs() << "  For IF: " << IF << " Votes: " << IFVotes[IF]
                          << "\n");
        if (IFVotes[IF] > MaxVotes) {
          MaxVotes = IFVotes[IF];
          PreferredIF = IF;
        }
      }

      return PreferredIF;
    }

    /// Returns the preferred IF for the store-container \p Cont.
    unsigned calculateStoreIF(const Container &Cont) const {
      assert(Cont.isStoreType());
      unsigned PreferredIF = 0;

      return PreferredIF;
    }

  public:
    /// Returns the preferred IF for the container \p Cont.
    unsigned getIF(const Container &Cont, bool Recalculate = false) {
      auto It = ContainerIFMap.find(Cont.getType());
      assert(It != ContainerIFMap.end());
      int IF = (*It).second;

      if (IF == -1 || Recalculate) {
        // (Re)calculate IF of `Cont` since it's unset.
        switch (Cont.getType()) {
        case Container::Type::CoordUpdate:
          IF = calculateCoordUpdateIF(Cont);
          break;

        case Container::Type::Load:
          IF = calculateLoadIF(Cont);
          break;

        case Container::Type::Compute:
          IF = calculateComputeIF(Cont);
          break;

        case Container::Type::Store:
          IF = calculateStoreIF(Cont);
          break;

        default:
          llvm_unreachable("iter-ilv: Invalid container type");
        }

        LLVM_DEBUG(dbgs() << "iter-ilv::isomorphicCM: Setting " << Cont
                          << "'s IF: " << IF << "\n");
        ContainerIFMap[Cont.getType()] = IF;
      }

      return IF;
    }

    /// Forces the cost-model to choose IF \p IF for the container \p Cont
    void setIF(const Container &Cont, int IF) {
      auto It = ContainerIFMap.find(Cont.getType());
      if (It == ContainerIFMap.end())
        return;

      ContainerIFMap[Cont.getType()] = IF;
    }

    explicit IsomorphicCostModel(const TargetTransformInfo *TTI, unsigned UF,
                                 const std::list<Container> &COI)
        : TTI(TTI), UF(UF), ContainersOfInterest(COI) {
      // Unset IF of each containers in `ContainersOfInterest`
      for (const Container &Cont : ContainersOfInterest) {
        ContainerIFMap[Cont.getType()] = -1;
      }
    }
  };

  std::unique_ptr<IsomorphicCostModel> IsomorphicCM;

  /// Gets the unroll-factor of \p L
  unsigned getLoopUF(const Loop *L) const { return CI->getNumClusters(); }

  /// Sets relevant member-variables for \p TheLoop
  void setLoopInfo() { TheLoopHeader = TheLoop->getHeader(); }

  /// Eliminates the false-dependence between coord-updates of the same index in
  /// \p TheLoop
  void elimCoordUpdateFalseDeps() {
    for (Instruction *CoordUpdate : CoordUpdates) {
      if (!isa<InsertElementInst>(CoordUpdate))
        continue;

      Value *CoordRegVal = CoordUpdate->getOperand(0);
      Instruction *CoordReg = dyn_cast<InsertElementInst>(CoordRegVal);
      if (!CoordReg)
        continue;

      if (CoordReg->getParent() != CoordUpdate->getParent())
        continue;

      if (CoordUpdate->getOperand(2) != CoordReg->getOperand(2))
        continue;

      CoordUpdate->replaceUsesOfWith(CoordReg, CoordReg->getOperand(0));
    }
  }

  /// Trims coord-update live-ranges by moving them immediately before it's
  /// first user if possible.
  void trimCoordUpdateLiveRange() {
    for (Instruction *CoordUpdate : CoordUpdates) {
      Instruction *FirstUser = getFirstNonPHIUserInBB(CoordUpdate);
      if (!FirstUser)
        continue;
      CoordUpdate->moveBefore(FirstUser);
    }
  }

  /// Returns the cluster-set from \p ClusterSetsOfInterest that holds \p Inst
  SmallSet<Instruction *, 4> *getClusterSetHaving(const Instruction *Inst) {
    unsigned NumClusters = CI->getNumClusters();
    for (unsigned I = 0; I < NumClusters; ++I) {
      if (ClusterSetsOfInterest[I].find(Inst) !=
          ClusterSetsOfInterest[I].end()) {
        return &ClusterSetsOfInterest[I];
      }
    }

    return nullptr;
  }

  /// Returns true if there are no unclustered-clustered instruction
  /// dependencies and no inter-cluster instruction dependencies.
  bool hasSimpleClusterDeps() {
    // Check if there are inter-cluster non-coord-update dependencies
    unsigned NumClusters = CI->getNumClusters();
    for (unsigned I = 0; I < NumClusters; ++I) {
      for (Instruction *Inst : ClusterSetsOfInterest[I]) {
        if (isCoordUpdate(Inst))
          continue;

        for (User *Usr : Inst->users()) {
          Instruction *UsrInst = dyn_cast<Instruction>(Usr);
          if (!UsrInst)
            continue;

          if (isCoordUpdate(UsrInst))
            continue;

          SmallSet<Instruction *, 4> *UsrCluster = getClusterSetHaving(UsrInst);
          if (!UsrCluster)
            continue;
          if (UsrCluster != &ClusterSetsOfInterest[I])
            return false;
        }
      }
    }

    return true;
  }

  /// Returns the command-line enforced IF for the container \p Cont. Note that
  /// -1 means IF is not enforced via the command-line.
  int getIFOptFor(const Container &Cont) const {
    switch (Cont.getType()) {
    case Container::Type::CoordUpdate:
      return CoordUpdateIFOpt;

    case Container::Type::Load:
      return LoadIFOpt;

    case Container::Type::Compute:
      return ComputeIFOpt;

    case Container::Type::Store:
      return StoreIFOpt;

    default:
      llvm_unreachable("iter-ilv: Invalid container type");
    }
  }

  /// Sets interleave-factor \p IF for the container \p Cont if it is overridden
  /// by anything outside the cost-model.
  void overrideIF(const Container &Cont, unsigned &IF) const {
    int IFOpt = getIFOptFor(Cont);
    if (IFOpt == -1)
      return;

    IF = IFOpt;
    LLVM_DEBUG(dbgs() << "iter-ilv::ismorphic: Overriding "
                      << Cont.getTypeAsString() << "-container's IF to " << IF
                      << " (user enforced)\n");
  }

  enum class IsomorphicInterleaveState {
    Interleaved,
    NotInterleaved,
    Unknown,
    Unset
  } CurrentIsomorphicInterleaveState = IsomorphicInterleaveState::Unset;

  /// Returns the state of isomorphic interleave of \p TheLoop
  IsomorphicInterleaveState getIsomorphicInterleaveState() {
    if (CI->getNumClusters() < 2)
      return IsomorphicInterleaveState::NotInterleaved;

    unsigned LoadsPerIteration = CI->HeaderLoadInstsVec[0].size();
    unsigned StoresPerIteration = CI->HeaderStoreInstsVec[0].size();

    if (!LoadsPerIteration && !StoresPerIteration)
      return IsomorphicInterleaveState::Unknown;

    // If we get a compute instruction in between mem-access of 2 consecutive
    // iterations of the unrolled loop, we guess that the loop is not
    // interleaved.

    unsigned MemAccessPerIteration = StoresPerIteration;
    VecInstVecType *MemAccessInstsVec = &CI->HeaderStoreInstsVec;
    Intrinsic::ID MemAccessType = Intrinsic::tpc_st_tnsr;

    if (!StoresPerIteration) {
      MemAccessPerIteration = LoadsPerIteration;
      MemAccessInstsVec = &CI->HeaderLoadInstsVec;
      MemAccessType = Intrinsic::tpc_ld_tnsr;
    }

    if (MemAccessPerIteration < 1)
      return IsomorphicInterleaveState::Unknown;

    BasicBlock::iterator FirstIterMemAcc =
        (*MemAccessInstsVec)[0][0]->getIterator();
    BasicBlock::iterator SecondIterMemAcc =
        (*MemAccessInstsVec)[1][0]->getIterator();

    for (BasicBlock::iterator I = FirstIterMemAcc; I != SecondIterMemAcc; ++I) {
      const Instruction &Inst = *I;
      if (isCoordUpdate(&Inst))
        continue;

      if (isIntrinsicOfID(&Inst, MemAccessType))
        continue;

      return IsomorphicInterleaveState::NotInterleaved;
    }

    return IsomorphicInterleaveState::Interleaved;
  }

  /// Does an isomorphic interleave of container \p Cont
  void isomorphicInterleave(const Container &Cont) {
    const VecInstVecType &InstLists = Cont.InstLists;
    unsigned NI, NJ;

    if (Cont.InstLists.size() == 0 || Cont.InstLists[0].size() == 0) {
      LLVM_DEBUG(dbgs() << "iter-ilv::isomorphic: Skipping "
                        << Cont.getTypeAsString()
                        << "-container since it's empty\n");
      return;
    }

    NI = InstLists[0].size();
    NJ = IsomorphicCM->getIF(Cont);
    overrideIF(Cont, /* &IF */ NJ);
    if (!NJ)
      return;

    for (unsigned I = 0; I < NI; ++I) {
      Instruction *To = InstLists[0][I];
      for (unsigned J = 1; J < NJ; ++J) {
        moveNonPHIInBBAfter(InstLists[J][I], To);
        To = InstLists[J][I];
      }
    }

    NumInstsInterleaved += NI * (NJ - 1);
  }

  /// Does an isomorphic interleave of container(s) in \p ContainersOfInterest
  void isomorphicInterleave() {
    if (CurrentIsomorphicInterleaveState ==
            IsomorphicInterleaveState::Interleaved ||
        CurrentIsomorphicInterleaveState ==
            IsomorphicInterleaveState::Unknown) {
      LLVM_DEBUG(dbgs() << "iter-ilv::isomorphic: Maybe already interleaved\n");
      return;
    }

    for (const Container &Cont : ContainersOfInterest) {
      isomorphicInterleave(Cont);
    }
  }

  /// Returns true if \p Inst is a software-pipelined instruction in \p TheLoop
  bool isSWPInst(const Instruction *Inst) const {
    // We asssume `Inst` is an SWP instruction if it has one user of type
    // PHINode in it's BB.
    unsigned PHIUsrCount = 0;
    for (const User *Usr : Inst->users()) {
      const Instruction *UsrDef = dyn_cast<Instruction>(Usr);
      if (!UsrDef)
        continue;

      if (UsrDef->getParent() != Inst->getParent())
        continue;

      if (!isa<PHINode>(UsrDef))
        return false;

      PHIUsrCount++;
    }

    if (PHIUsrCount == 1)
      return true;

    return false;
  }

  /// This strategy groups computes before loads (i.e. SWP loads) and stores.
  /// After that, the coord-updates will be filled between the loads, stores and
  /// computes.
  bool interleaveSWPStrategyA() {
    const Container *LoadContainer =
        getContainerOfInterest(Container::Type::Load);
    const Container *ComputeContainer =
        getContainerOfInterest(Container::Type::Compute);
    const Container *StoreContainer =
        getContainerOfInterest(Container::Type::Store);
    if (!LoadContainer || !ComputeContainer || !StoreContainer)
      return false;

    LLVM_DEBUG(dbgs() << "iter-ilv::SWP: Using strategy A\n");

    // Group loads after computes
    moveNonPHIInBBAfter(LoadContainer->InstLists[0].front(),
                        ComputeContainer->InstLists[UF - 1].back());
    IsomorphicCM->setIF(*LoadContainer, UF);
    isomorphicInterleave(*LoadContainer);

    // Group stores after loads
    moveNonPHIInBBAfter(StoreContainer->InstLists[0].front(),
                        LoadContainer->InstLists[UF - 1].back());
    IsomorphicCM->setIF(*StoreContainer, UF);
    isomorphicInterleave(*StoreContainer);

    std::deque<Instruction *> CoordUpdatesQ;

    unsigned LoadsPerIteration = LoadContainer->getNumInstPerIteration();
    unsigned ComputesPerIteration = ComputeContainer->getNumInstPerIteration();

    // Ignore bitcasts
    for (const Instruction *Compute : ComputeContainer->InstLists[0]) {
      if (isa<BitCastInst>(Compute))
        --ComputesPerIteration;
    }

    // Fill coord-updates between computes if we have a > 1 compute-to-load
    // ratio
    if (LoadsPerIteration && ComputesPerIteration &&
        (ComputesPerIteration / LoadsPerIteration) > 1) {
      LLVM_DEBUG(dbgs() << "iter-ilv::SWP: Got compute-to-load ratio > 1\n");

      for (Instruction *CoordUpdate : CoordUpdates) {
        CoordUpdatesQ.push_back(CoordUpdate);
      }

      for (unsigned I = 0; I < UF; ++I) {
        for (Instruction *Compute : ComputeContainer->InstLists[I]) {
          if (CoordUpdatesQ.empty())
            break;

          Instruction *CoordUpdate = CoordUpdatesQ.front();
          moveNonPHIInBBAfter(CoordUpdate, Compute);
          CoordUpdatesQ.pop_front();
          NumInstsInterleaved++;
        }
      }

      IsomorphicCM->setIF(*ComputeContainer, UF);
      isomorphicInterleave(*ComputeContainer);

    } else {
      LLVM_DEBUG(dbgs() << "iter-ilv::SWP: Got compute-to-load ratio <= 1\n");
      for (Instruction *CoordUpdate : CoordUpdates) {
        CoordUpdatesQ.push_front(CoordUpdate);
      }
    }

    // Fill remaining coord-updates between loads and stores
    for (Instruction *Load : Loads) {
      if (CoordUpdatesQ.empty())
        break;

      Instruction *CoordUpdate = CoordUpdatesQ.front();
      moveNonPHIInBBAfter(CoordUpdate, Load);
      CoordUpdatesQ.pop_front();
      NumInstsInterleaved++;
    }

    for (Instruction *Store : Stores) {
      if (CoordUpdatesQ.empty())
        break;

      Instruction *CoordUpdate = CoordUpdatesQ.front();
      moveNonPHIInBBAfter(CoordUpdate, Store);
      CoordUpdatesQ.pop_front();
      NumInstsInterleaved++;
    }

    removeContainerOfInterest(*LoadContainer);
    removeContainerOfInterest(*ComputeContainer);
    removeContainerOfInterest(*StoreContainer);

    return true;
  }

  /// This strategy fills \p SWPLoads between computes and stores
  bool interleaveSWPStrategyB(std::deque<Instruction *> &SWPLoads) {
    if (SWPLoads.empty()) {
      return false;
    }

    const Container *LoadContainer =
        getContainerOfInterest(Container::Type::Load);
    if (!LoadContainer)
      return false;

    LLVM_DEBUG(dbgs() << "iter-ilv::SWP: Using strategy B\n");

    // We want the DAG rooted at each loads at the top of `TheLoopHeader` since
    // that'll allow us to move the loads between computes and stores without
    // worrying about it's DAG ruining the packetizer.
    isomorphicInterleave(*LoadContainer);
    removeContainerOfInterest(*LoadContainer);

    for (unsigned I = 0; I < UF; ++I) {
      for (Instruction *ComputeInst : CI->HeaderComputeInstsVec[I]) {
        Instruction *SWPLoad = SWPLoads.front();
        moveNonPHIInBBAfter(SWPLoad, ComputeInst);
        NumInstsInterleaved++;
        SWPLoads.pop_front();
        if (SWPLoads.empty())
          return true;
      }

      for (Instruction *StoreInst : CI->HeaderStoreInstsVec[I]) {
        Instruction *SWPLoad = SWPLoads.front();
        moveNonPHIInBBAfter(SWPLoad, StoreInst);
        NumInstsInterleaved++;
        SWPLoads.pop_front();
        if (SWPLoads.empty())
          return true;
      }
    }

    return true;
  }

  /// Interleave by taking advantage of SWP insts (if any)
  void interleaveSWPInsts() {
    if (CurrentIsomorphicInterleaveState ==
        IsomorphicInterleaveState::Interleaved) {
      return;
    }

    std::deque<Instruction *> SWPLoads;
    for (Instruction *Load : Loads) {
      if (isSWPInst(Load))
        SWPLoads.push_back(Load);
    }

    if (SWPLoads.empty())
      return;

    const Container *LoadContainer =
        getContainerOfInterest(Container::Type::Load);
    if (!LoadContainer)
      return;

    if (SWPLoads.size() != LoadContainer->getNumInst())
      return;

    LLVM_DEBUG(dbgs() << "iter-ilv::SWP: Found " << SWPLoads.size()
                      << " SWP loads\n");

    if (!interleaveSWPStrategyA()) {
      interleaveSWPStrategyB(SWPLoads);
    }
  }

  /// The iteration interleave driver
  void interleave() {
    CurrentIsomorphicInterleaveState = getIsomorphicInterleaveState();
    interleaveSWPInsts();
    isomorphicInterleave();
  }

  /// Return true if we should attempt the transformation. Strictly speaking,
  /// this is not just checking legality, but more precisely, if the current
  /// implementation can interleave \p TheLoop.
  bool isReasonable() {
    if (!TheLoop->getSubLoops().empty()) {
      LLVM_DEBUG(dbgs() << "iter-ilv: Not an inner most loop\n");
      return false;
    }

    if (TheLoop->getNumBlocks() > 1) {
      LLVM_DEBUG(dbgs() << "iter-ilv: Has more than 1 number of blocks\n");
      return false;
    }

    if (!TheLoop->isRotatedForm()) {
      LLVM_DEBUG(dbgs() << "iter-ilv: Not a rotated loop\n");
      return false;
    }

    CI = std::make_unique<ClusterInfo>(TheLoop, SE);
    if (!CI->clusterInstructions()) {
      LLVM_DEBUG(dbgs() << "iter-ilv: ClusterInfo failed\n");
      return false;
    }
    UF = getLoopUF(TheLoop);

    if (!UF) {
      LLVM_DEBUG(dbgs() << "iter-ilv: Could not determine unroll-factor\n");
      return false;
    }

    if (UF < 2) {
      LLVM_DEBUG(dbgs() << "iter-ilv: Not an unrolled loop\n");
      return false;
    }

    for (Instruction &I : *TheLoopHeader) {
      Container::Type ContType = Container::getTypeFor(&I);
      switch (ContType) {
      case Container::Type::CoordUpdate:
        CoordUpdates.push_back(&I);
        break;
      case Container::Type::Load:
        Loads.push_back(&I);
        break;
      case Container::Type::Store:
        Stores.push_back(&I);
        break;
      default:
        break;
      }
    }

    // Populate `CI->HeaderCoordUpdateInstsVec` from `CoordUpdates`
    unsigned CoordUpdatesPerIteration = CoordUpdates.size() / UF;
    if (!CoordUpdatesPerIteration)
      CoordUpdatesPerIteration = CoordUpdates.size();

    unsigned I = 0;
    for (Instruction *Inst : CoordUpdates) {
      CI->HeaderCoordUpdateInstsVec.push_back(InstructionVecType());
      CI->HeaderCoordUpdateInstsVec[I / CoordUpdatesPerIteration].push_back(
          Inst);
      I++;
    }

    // Populate `CI->HeaderLoadInstsVec` from `Loads` if `CI` couldn't
    unsigned LoadsPerIteration = Loads.size() / UF;
    if (CI->HeaderLoadInstsVec[0].empty() && LoadsPerIteration > 0 &&
        (LoadsPerIteration * UF) == Loads.size()) {
      unsigned I = 0;
      for (Instruction *Inst : Loads) {
        CI->HeaderLoadInstsVec[I / LoadsPerIteration].push_back(Inst);
        I++;
      }
    }

    addContainerOfInterest(CI->HeaderCoordUpdateInstsVec,
                           Container::Type::CoordUpdate);
    addContainerOfInterest(CI->HeaderLoadInstsVec, Container::Type::Load);
    addContainerOfInterest(CI->HeaderComputeInstsVec, Container::Type::Compute);
    addContainerOfInterest(CI->HeaderStoreInstsVec, Container::Type::Store);

    if (CI->getNumClusters() >= 2) {
      unsigned LoadsPerIteration = CI->HeaderLoadInstsVec[0].size();

      if (LoadsPerIteration * UF >= 16) {
        LLVM_DEBUG(
            dbgs() << "iter-ilv: Interleaving will stack up too many loads\n");
        return false;
      }
    }

    // Populate `ClusterSetsOfInterest` from `ContainersOfInterest`
    unsigned NumClusters = CI->getNumClusters();
    ClusterSetsOfInterest.resize(NumClusters);
    for (unsigned I = 0; I < NumClusters; ++I) {
      for (const Container &Cont : ContainersOfInterest) {
        if (Cont.isCoordUpdateType())
          continue;

        for (Instruction *Inst : Cont.InstLists[I]) {
          ClusterSetsOfInterest[I].insert(Inst);
        }
      }
    }

    if (!hasSimpleClusterDeps()) {
      LLVM_DEBUG(dbgs() << "iter-ilv: Interleaving is not supported for the "
                           "given cluster dependencies\n");
      return false;
    }

    return true;
  }

public:
  IterationInterleaver(ScalarEvolution *SE, const TargetTransformInfo *TTI,
                       Loop *L)
      : SE(SE), TTI(TTI), TheLoop(L) {}

  bool run() {
    setLoopInfo();

    LLVM_DEBUG(dbgs() << "iter-ilv: Working on " << *TheLoop);

    if (!isReasonable()) {
      LLVM_DEBUG(dbgs() << "iter-ilv: Skipping " << TheLoop->getName() << "\n");
      return false;
    }

    IsomorphicCM =
        std::make_unique<IsomorphicCostModel>(TTI, UF, ContainersOfInterest);

    if (ElimFalseCoordUpdateDepsOpt) {
      elimCoordUpdateFalseDeps();
    }

    LLVM_DEBUG(dbgs() << "iter-ilv: Interleaving " << TheLoop->getName()
                      << "\n");
    interleave();

    if (TrimCoordUpdateLiveRangeOpt) {
      trimCoordUpdateLiveRange();
    }

    NumLoopsInterleaved++;
    return true;
  }
};

struct IterationInterleaveLegacy : public LoopPass {
  static char ID;

  bool runOnLoop(Loop *L, LPPassManager &LPM) override {
    ScalarEvolution *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    const TargetTransformInfo *TTI =
        &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(
            *L->getHeader()->getParent());
    IterationInterleaver II(SE, TTI, L);
    return II.run();
  }

  IterationInterleaveLegacy() : LoopPass(ID) {
    initializeIterationInterleaveLegacyPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();

    AU.setPreservesCFG();
  }
};

} // end anonymous namespace

char IterationInterleaveLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(IterationInterleaveLegacy, "iter-ilv",
                      "Iteration Interleave", false, false)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(IterationInterleaveLegacy, "iter-ilv",
                    "Iteration Interleave", false, false)

Pass *llvm::createIterationInterleavePass() {
  return new IterationInterleaveLegacy();
}
