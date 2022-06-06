// Eliminate redundant linearity requirement in instructions.
//
// Try to replace tpc.convert.linear intrinsic with cheaper tpc.convert.
//
// High level algorithm:
//    1. Find candidate instruction for replacement (tpc.convert.linear)
//    2. Collect connected components of Data Dependency Graph for analysis. Run
//       breadth-first search from the instruction users. Stop search on other
//       possible instructions for replacement.
//    3. Check components bounded by tpc.convert.linear instructions only,
//       whether linearity removal does not change program output.
//    4. Check components bounded by tpc.convert.linear and tpc.st.tnsr,
//       tpc.ld.tnsr instructions only, whether linearity removal does not
//       change program output. It is allowed to apply PACK/UNPACK switches for
//       tensor store / load instructions.
//
// Algorithm of linearity removal availability checking:
//    * For each output node for each output result index:
//    * Build expression evaluation trees for graphs with and without linear
//      instructions.
//    * Ensure trees are equivalent.
//
// Possible improvements:
//  * support more instructions / intrinsics.
//  * some operations are agnostic to elements order in the input vectors,
//    (vector_reduce*), they might allow additional optimizations. But the
//    current recursive checker does not support such instructions well because
//    each permutation for reduce operation source indices should be evaluated,
//    which is very expensive. Another option is to avoid recursion: construct
//    trees for each output index, compare whole sorted vectors of trees for
//    reduce instructions and 1-by-1 for other instructions. It means
//    significant pass refactoring.

#include "MCTargetDesc/InstructionDB.h"
#include "TPCTargetMachine.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/DDG.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IntrinsicsTPC.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/TPCIntrinsicUtils.h"

#include <cassert>
#include <memory>
#include <utility>

#define DEBUG_TYPE "tpc-eliminate-redundant-linearity"

using namespace llvm;

static cl::opt<bool> EnableEliminateRedundantLinearity(
    "tpc-eliminate-redundant-linearity", cl::Hidden,
    cl::desc("Enable Eliminate Redundant Linearity pass."), cl::init(true));

static cl::opt<unsigned> MaxComponentSizeToAnalyze(
    "tpc-erl-comp-size-limit", cl::Hidden, cl::init(250), cl::Optional,
    cl::desc("Maximum size of DDG connected component in instructions to be "
             "analyzed. Analysis is time-consuming operation and should be "
             "restricted to small sub graphs to save compilation time. Set "
             "this value to 0 to ignore limitation."));

static cl::opt<bool> EnableTensorStPack(
    "tpc-erl-enable-store-pack", cl::Hidden, cl::init(true),
    cl::desc("Enable patching tpc.tnsr.st instructions with PACK switch for "
             "more cases to be analyzed and optimized"));

static cl::opt<bool> EnableTensorStPack32To16(
    "tpc-erl-enable-store-pack-32-to-16", cl::Hidden, cl::init(true),
    cl::desc("Enable patching tpc.tnsr.st instructions with PACK_32_to_16"));

static cl::opt<bool> EnableTensorStPack32To8(
    "tpc-erl-enable-store-pack-32-to-8", cl::Hidden, cl::init(true),
    cl::desc("Enable patching tpc.tnsr.st instructions with PACK_32_to_8"));

static cl::opt<bool> EnableTensorStPack16To8(
    "tpc-erl-enable-store-pack-16-to-8", cl::Hidden, cl::init(true),
    cl::desc("Enable patching tpc.tnsr.st instructions with PACK_16_to_8"));

static cl::opt<bool> EnableTensorLdUnpack(
    "tpc-erl-enable-load-unpack", cl::Hidden, cl::init(true),
    cl::desc("Enable patching tpc.tnsr.ld instructions with UNPACK switch "
             "for more cases to be analyzed and optimized"));

STATISTIC(NumOptimizedTPCConvertLinearIntrinsics,
          "tpc.convert.linear intrinsics count optimized by the pass");
STATISTIC(NumPatchedTensorLoadIntrinsics,
          "Tensor load intrinsics count patched by the pass");
STATISTIC(NumPatchedTensorStoreIntrinsics,
          "Tensor store intrinsics count patched by the pass");

using DDGNodes = SmallDenseSet<const DDGNode *, 8>;
using NodeToSwitchPatch = SmallDenseMap<const DDGNode *, std::uint64_t, 4>;
using SimpleDDGNodes = SmallDenseSet<const SimpleDDGNode *, 8>;
using IntrinsicsSet = SmallDenseSet<IntrinsicInst *, 8>;
using IntrinsicToSwitchPatch = SmallDenseMap<IntrinsicInst *, std::uint64_t, 4>;

constexpr std::uint64_t SwPack32To16 = TPCII::SW_PACK | TPCII::SW_PCK_32_TO_16;
constexpr std::uint64_t SwPack32To8 = TPCII::SW_PACK | TPCII::SW_PCK_32_TO_8;
constexpr std::uint64_t SwPack16To8 = TPCII::SW_PACK | TPCII::SW_PCK_16_TO_8;

static bool isSingleInstructionSimpleNode(const DDGNode &N) {
  return isa<SimpleDDGNode>(N) &&
         cast<SimpleDDGNode>(N).getInstructions().size() == 1;
}

static Instruction *getFirstInstruction(const DDGNode &N) {
  assert(isa<SimpleDDGNode>(N));
  return cast<SimpleDDGNode>(N).getFirstInstruction();
}

template <typename ProcessFunc>
void enumerateFwdNodes(const DDGNode &N, const ProcessFunc &F) {
  for (auto *E : N.getEdges()) {
    if (E->isDefUse())
      F(E->getTargetNode());
  }
}

template <typename ProcessFunc>
void enumerateBwdNodes(const DDGNode &N, const ProcessFunc &F) {
  for (auto *E : N.getEdges()) {
    if (E->getKind() == DDGEdge::EdgeKind::RegisterUseDef)
      F(E->getTargetNode());
  }
}

namespace {

/// DDG sub graph component.
class GraphComponent {
public:
  explicit GraphComponent(DDGNodes InNodes) : Nodes(std::move(InNodes)) {}

  GraphComponent(const GraphComponent &) = default;
  GraphComponent(GraphComponent &&) = default;
  GraphComponent &operator=(const GraphComponent &) = default;
  GraphComponent &operator=(GraphComponent &&) = default;

  const DDGNodes &nodes() const { return Nodes; }

  bool isInpNode(const DDGNode &N) const {
    return Nodes.contains(&N) && isInpNodeImpl(N);
  }

  bool isOutNode(const DDGNode &N) const {
    return Nodes.contains(&N) && isOutNodeImpl(N);
  }

  bool isBoundaryNode(const DDGNode &N) const {
    return Nodes.contains(&N) && (isInpNodeImpl(N) || isOutNodeImpl(N));
  }

  template <typename ProcessFunc>
  void enumerateFwdNeighbors(const DDGNode &N, const ProcessFunc &F) const {
    assert(Nodes.contains(&N));
    enumerateFwdNodes(N, [&](const DDGNode &Child) {
      if (Nodes.contains(&Child))
        F(Child);
    });
  }

private:
  bool isInpNodeImpl(const DDGNode &N) const {
    assert(Nodes.contains(&N));
    return any_of(N.getEdges(),
                  [&](DDGEdge *E) {
                    return E->getKind() == DDGEdge::EdgeKind::RegisterUseDef &&
                           !Nodes.contains(&E->getTargetNode());
                  }) ||
           none_of(N.getEdges(), [&](DDGEdge *E) {
             return E->getKind() == DDGEdge::EdgeKind::RegisterUseDef &&
                    Nodes.contains(&E->getTargetNode());
           });
  }

  bool isOutNodeImpl(const DDGNode &N) const {
    assert(Nodes.contains(&N));
    return any_of(N.getEdges(),
                  [&](DDGEdge *E) {
                    return E->isDefUse() &&
                           !Nodes.contains(&E->getTargetNode());
                  }) ||
           none_of(N.getEdges(), [&](DDGEdge *E) {
             return E->isDefUse() && Nodes.contains(&E->getTargetNode());
           });
  }

  DDGNodes Nodes;
};

raw_ostream &operator<<(raw_ostream &OS, const GraphComponent &Component) {
  for (const DDGNode *N : Component.nodes()) {
    const bool IsInp = Component.isInpNode(*N);
    const bool IsOut = Component.isOutNode(*N);
    OS << "  [" << (IsInp ? "i" : "-") << (IsOut ? "o" : "-") << "]  ";
    if (isa<SimpleDDGNode>(*N))
      OS << *getFirstInstruction(*N) << "\n";
    else
      OS << *N << "\n";
  }
  return OS;
}

} // namespace

static bool isIntrinsicOfID(const Instruction &I, const Intrinsic::ID Id) {
  auto *Int = dyn_cast<IntrinsicInst>(&I);
  return Int && Int->getIntrinsicID() == Id;
}

static bool isTpcConvertIntrinsic(const Instruction &I) {
  return isIntrinsicOfID(I, llvm::Intrinsic::tpc_convert);
}

static bool isTpcConvertLinearIntrinsic(const Instruction &I) {
  return isIntrinsicOfID(I, llvm::Intrinsic::tpc_convert_linear);
}

static bool isTensorStIntrinsic(const Instruction &I) {
  return isIntrinsicOfID(I, llvm::Intrinsic::tpc_st_tnsr);
}

static bool isTensorLdIntrinsic(const Instruction &I) {
  return isIntrinsicOfID(I, llvm::Intrinsic::tpc_ld_tnsr);
}

template <typename InstructionCheckFunction>
bool containsInstruction(const DDGNode &N,
                         const InstructionCheckFunction &CheckFunction) {
  return isa<SimpleDDGNode>(N) &&
         any_of(cast<SimpleDDGNode>(N).getInstructions(),
                [&](const Instruction *I) { return CheckFunction(*I); });
}

static bool isTpcConvertLinearNode(const DDGNode &N) {
  return containsInstruction(N, isTpcConvertLinearIntrinsic);
}

static bool isTensorStNode(const DDGNode &N) {
  return containsInstruction(N, isTensorStIntrinsic);
}

static bool isTensorLdNode(const DDGNode &N) {
  return containsInstruction(N, isTensorLdIntrinsic);
}

static std::uint64_t getTensorStLdSwitchValue(const IntrinsicInst &I) {
  assert(isTensorLdIntrinsic(I) || isTensorStIntrinsic(I));
  const unsigned OpIx = isTensorStIntrinsic(I) ? 3 : 2;
  return dyn_cast<llvm::ConstantInt>(I.getOperand(OpIx))->getZExtValue();
}

static void setTensorStLdSwitchValue(IntrinsicInst &I,
                                     const std::uint64_t Value) {
  assert(isTensorLdIntrinsic(I) || isTensorStIntrinsic(I));
  const unsigned OpIx = isTensorStIntrinsic(I) ? 3 : 2;
  I.setArgOperand(
      OpIx, ConstantInt::get(I.getOperand(OpIx)->getType(), Value, false));
}

/// There is no difference in behavior of tpc.convert.linear vs tpc.convert if
/// scalar sizes of the source and destination types are equal.
static bool isLinearityRemovalAffectsInstructionResult(const Instruction &I) {
  assert(isTpcConvertLinearIntrinsic(I));
  const Type *DstTy = I.getType();
  const Type *SrcTy = I.getOperand(0)->getType();
  const FixedVectorType *DstVTy = dyn_cast<FixedVectorType>(DstTy);
  const FixedVectorType *SrcVTy = dyn_cast<FixedVectorType>(SrcTy);
  assert(DstVTy);
  assert(SrcVTy);
  return SrcVTy->getScalarSizeInBits() != DstVTy->getScalarSizeInBits();
}

static unsigned getSwizzleSourceIx(const unsigned DstIx,
                                   const unsigned NumElements,
                                   const unsigned SrcScalarSizeInBits,
                                   const unsigned DstScalarSizeInBits) {
  // Case: no permutation.
  if (DstScalarSizeInBits == SrcScalarSizeInBits)
    return DstIx;

  const bool IsUpConvert = SrcScalarSizeInBits < DstScalarSizeInBits;
  const unsigned BlocksCount =
      (IsUpConvert ? SrcScalarSizeInBits : DstScalarSizeInBits) * NumElements /
      2048;
  const unsigned Ratio = IsUpConvert
                             ? DstScalarSizeInBits / SrcScalarSizeInBits
                             : SrcScalarSizeInBits / DstScalarSizeInBits;
  const unsigned BlockSize = NumElements / BlocksCount;
  const unsigned BucketSize = BlockSize / Ratio;
  const unsigned Start = (DstIx / BlockSize) * BlockSize;
  if (IsUpConvert) {
    const unsigned S = (DstIx - Start) / BucketSize;
    const unsigned Bucket = DstIx - Start - S * BucketSize;
    const unsigned I = Bucket * Ratio;
    return Start + I + S;
  }
  const unsigned Bucket = (DstIx - Start) / Ratio;
  const unsigned I = Bucket * Ratio;
  const unsigned S = DstIx - Start - I;
  return Start + Bucket + BucketSize * S;
}

/// Returns source vector index for the given destination index if operation
/// performs swizzle. For example: tpc.convert, tpc.mac (acc_fp32) ...
///
/// \param DstIx index in the result vector.
/// \param DstTy destination fixed vector type.
/// \param SrcTy source fixed vector type.
/// \return index in the source vector.
static Optional<unsigned>
getSwizzleSourceIx(const unsigned DstIx, const Type *DstTy, const Type *SrcTy) {
  if (!DstTy || !SrcTy)
    return None;

  const FixedVectorType *DstVTy = dyn_cast<FixedVectorType>(DstTy);
  const FixedVectorType *SrcVTy = dyn_cast<FixedVectorType>(SrcTy);
  if (!DstVTy || !SrcVTy ||
      DstVTy->getNumElements() != SrcVTy->getNumElements())
    return None;

  return getSwizzleSourceIx(DstIx, DstVTy->getNumElements(),
                            SrcVTy->getScalarSizeInBits(),
                            DstVTy->getScalarSizeInBits());
}

namespace {
/// Utility class to check whether graph contains cycles.
class DFSCycleChecker {
public:
  bool containsCycle(const GraphComponent &G) {
    Component = &G;
    Nodes = G.nodes();
    NodeToState.reserve(Nodes.size());
    for (const DDGNode *N : Nodes)
      NodeToState[N] = NodeState::NotProcessed;

    while (!Nodes.empty()) {
      if (dfs(**Nodes.begin()))
        return true;
    }
    return false;
  }

private:
  enum class NodeState {
    NotProcessed,
    Processing,
    Processed,
  };

  bool dfs(const DDGNode &N) {
    if (NodeToState[&N] == NodeState::Processing)
      return true; // Cycle in the graph.

    NodeToState[&N] = NodeState::Processing;
    bool HasCycle = false;
    Component->enumerateFwdNeighbors(N, [&](const DDGNode &FwdNeighbor) {
      HasCycle = HasCycle || dfs(FwdNeighbor);
    });
    NodeToState[&N] = NodeState::Processed;
    Result.push_back(&N);
    Nodes.erase(&N);
    return HasCycle;
  }

  const GraphComponent *Component = nullptr;
  DDGNodes Nodes;
  SmallDenseMap<const DDGNode *, NodeState, 8> NodeToState;
  SmallVector<const DDGNode *, 8> Result;
};

class GraphComponentSearcher {
public:
  GraphComponentSearcher() = default;
  GraphComponentSearcher(const GraphComponentSearcher &) = delete;
  GraphComponentSearcher(GraphComponentSearcher &&) = delete;
  GraphComponentSearcher &operator=(const GraphComponentSearcher &) = delete;
  GraphComponentSearcher &operator=(GraphComponentSearcher &&) = delete;

  /// Build connected component for transformation analysis, where N is an input
  /// node of transformable kind (tpc.convert.linear).
  GraphComponent findForInpNode(const DDGNode &N) {
    ProcessedNodes.clear();
    Queue.clear();

    ProcessedNodes.insert(&N);
    enqueueFwdNodes(N);

    return findComponentImpl();
  }

  /// Build connected component for transformation analysis, where N is an
  /// output node of transformable kind (tpc.convert.linear).
  GraphComponent findForOutNode(const DDGNode &N) {
    ProcessedNodes.clear();
    Queue.clear();

    ProcessedNodes.insert(&N);
    enqueueBwdNodes(N);

    return findComponentImpl();
  }

private:
  GraphComponent findComponentImpl() {
    while (!Queue.empty()) {
      const DDGNode *NodeFromQueue = *Queue.begin();
      Queue.erase(Queue.begin());

      ProcessedNodes.insert(NodeFromQueue);

      if (!isSingleInstructionSimpleNode(*NodeFromQueue) ||
          isTransformTargetNode(*NodeFromQueue))
        continue;

      enqueueFwdNodes(*NodeFromQueue);
      enqueueBwdNodes(*NodeFromQueue);
    }
    return GraphComponent(std::move(ProcessedNodes));
  }

  void enqueueFwdNodes(const DDGNode &N) {
    enumerateFwdNodes(N, [&](const DDGNode &Child) {
      if (!ProcessedNodes.contains(&Child))
        Queue.insert(&Child);
    });
  }

  void enqueueBwdNodes(const DDGNode &N) {
    enumerateBwdNodes(N, [&](const DDGNode &Parent) {
      if (!ProcessedNodes.contains(&Parent))
        Queue.insert(&Parent);
    });
  }

  static bool isTransformTargetNode(const DDGNode &N) {
    assert(isSingleInstructionSimpleNode(N));
    return isTpcConvertLinearNode(N) || isTensorStNode(N) || isTensorLdNode(N);
  }

  DDGNodes ProcessedNodes;
  DDGNodes Queue;
};

} // namespace

static Optional<unsigned> getNumScalarsInType(const Type *T) {
  if (!T)
    return None;
  if (T->isVectorTy()) {
    const FixedVectorType *FV = dyn_cast<FixedVectorType>(T);
    return FV ? Optional<unsigned>(FV->getNumElements()) : Optional<unsigned>();
  }
  return T->isSingleValueType() ? Optional<unsigned>(1) : Optional<unsigned>();
}

static bool isSameVEReturnType(const Instruction &LHS, const Instruction &RHS) {
  const Type *TL = LHS.getType();
  const Type *TR = RHS.getType();

  if (!TL)
    return !TR;

  if (TL->isVoidTy())
    return TR->isVoidTy();

  if (TL->isVectorTy() != TR->isVectorTy())
    return false;

  if (TL->isVectorTy()) {
    const FixedVectorType *VTL = dyn_cast<FixedVectorType>(TL);
    const FixedVectorType *VTR = dyn_cast<FixedVectorType>(TR);
    return VTL && VTR && VTL->getNumElements() == VTR->getNumElements() &&
           VTL->getElementType()->getTypeID() ==
               VTR->getElementType()->getTypeID();
  }
  return TL->isSingleValueType() && TL->getTypeID() == TR->getTypeID();
}

/// Check for same return type with logging.
static bool checkSameVEReturnType(const Instruction &LHS,
                                  const Instruction &RHS) {
  if (!isSameVEReturnType(LHS, RHS)) {
    LLVM_DEBUG(dbgs() << "Not the same return type:\n"
                      << "  " << LHS << "\n"
                      << "  " << RHS << "\n");
    return false;
  }
  return true;
}

namespace {
class VectorExprProvider;

class VectorExpr;

class VectorExprResult {
public:
  enum class ReturnCode {
    /// Vector expression construction succeed.
    SUCCEED,

    /// Requested value is not an instruction and vector expression does not
    /// exist for it.
    NOT_AN_INSTRUCTION,

    /// Requested instruction is not in the component under analysis
    NOT_IN_COMPONENT,

    /// Requested instruction analysis is not supported.
    NOT_SUPPORTED,
  };

  explicit VectorExprResult(VectorExpr &VE) : Result(&VE) {}

  explicit VectorExprResult(ReturnCode Code) : RetCode(Code) {
    assert(RetCode != ReturnCode::SUCCEED);
  }

  ReturnCode returnCode() const { return RetCode; }

  VectorExpr &expr() const {
    assert(RetCode == ReturnCode::SUCCEED);
    return *Result;
  }

private:
  VectorExpr *Result = nullptr;
  ReturnCode RetCode = ReturnCode::SUCCEED;
};

/// VectorExpr class is a lightweight helper per DDG node for analysis of
/// evaluation expressions for each index in result vector (if result is a
/// vector).
class VectorExpr {
public:
  /// Expression kind.
  ///
  /// Note that if instruction is going to be transformed, expression kind for
  /// this instruction is equal to original kind for original graph checking and
  /// is equal to transformed kind for transformed graph checking.
  enum class Kind {
    /// 1-1 vectors processing instructions (tpc.add / tpc.sub ...)
    Composition,

    /// tpc.mac (in some cases result is swizzled)
    TPCMac,

    /// tpc.mac (in some cases result is swizzled)
    TPCMadd,

    /// tpc.mul (in some cases result is swizzled)
    TPCMul,

    /// Combination of 2 vectors
    ShuffleVector,

    /// tpc.convert
    Convert,

    /// tpc.convert.linear
    ConvertLinear,

    /// tpc.st.tnsr
    TensorStore,

    /// tpc.ld.tnsr
    TensorLoad,

    /// tpc.ld.g
    TensorBroadcastLoad,

    /// tpc.gen.addr
    GenAddr,
  };

  VectorExpr(const Kind InKind, const DDGNode &N,
             const std::uint64_t InSwitchPatch, VectorExprProvider &VEP)
      : VEKind(InKind), SwitchPatch(InSwitchPatch), Node(N), ExprProvider(VEP) {
    assert(isSingleInstructionSimpleNode(N));
  }

  virtual ~VectorExpr() = default;
  VectorExpr(const VectorExpr &) = delete;
  VectorExpr(VectorExpr &&) = delete;
  VectorExpr &operator=(const VectorExpr &) = delete;
  VectorExpr &operator=(VectorExpr &&) = delete;

  const Instruction &instruction() const {
    return *getFirstInstruction(Node.get());
  }

  Kind getKind() const { return VEKind; }

  std::uint64_t getSwitchPatch() const { return SwitchPatch; }

  VectorExprResult getExpression(const Value *V) const;

  const GraphComponent &component() const;

private:
  Kind VEKind;
  std::uint64_t SwitchPatch;
  std::reference_wrapper<const DDGNode> Node;
  std::reference_wrapper<VectorExprProvider> ExprProvider;
};

/// Helper class to organize check if two VectorExpr objects from the same
/// component (one VectorExpr is original, another one is a transformation
/// proposal) are identical.
class VectorExprEqChecker {
public:
  explicit VectorExprEqChecker(const GraphComponent &Component)
      : TargetComponent(&Component) {}

  bool isIdentical(const VectorExpr &LHS, const VectorExpr &RHS) {
    assert(&LHS.component() == TargetComponent);
    assert(&RHS.component() == TargetComponent);

    const Instruction &LI = LHS.instruction();
    const Instruction &RI = RHS.instruction();

    if (!checkSameVEReturnType(LI, RI))
      return false;

    const Optional<unsigned> NumScalars =
        isTensorStIntrinsic(LI)
            ? getNumScalarsInType(LI.getOperand(2)->getType())
            : getNumScalarsInType(LI.getType());
    if (!NumScalars.hasValue()) {
      LLVM_DEBUG(dbgs() << "Failed to get num scalars in result: " << LI
                        << "\n");
      return false;
    }

    for (unsigned Ix = 0; Ix < NumScalars.getValue(); ++Ix) {
      if (!isIdenticalScalarExpr(LHS, Ix, RHS, Ix)) {
        LLVM_DEBUG(dbgs() << "Not identical scalar expression:\n"
                          << "  Ix: " << Ix << "\n"
                          << LI << "\n"
                          << RI << "\n");
        return false;
      }
    }

    return true;
  }

private:
  /// Check for identical scalar expressions if vector expression is provided.
  ///
  /// \details Performs in-depth check along DDG.
  bool isIdenticalScalarExpr(const VectorExpr &LHS, const unsigned LScalarIx,
                             const VectorExpr &RHS, const unsigned RScalarIx) {
    switch (LHS.getKind()) {
    case VectorExpr::Kind::Composition:
      // Composition means no swizzled operands.
      return isIdenticalScalarExprForInstructionWithPartialSwizzles(
          VectorExpr::Kind::Composition, LHS, LScalarIx, RHS, RScalarIx, {},
          {});
    case VectorExpr::Kind::TPCMac:
    case VectorExpr::Kind::TPCMadd:
    case VectorExpr::Kind::TPCMul:
      // Operands 0 and 1 are swizzled in tpc.mac, tpc.madd, tpc.mul
      return isIdenticalScalarExprForInstructionWithPartialSwizzles(
          LHS.getKind(), LHS, LScalarIx, RHS, RScalarIx, {0, 1}, {});
    case VectorExpr::Kind::ShuffleVector:
      return isIdenticalScalarExprForShuffle(LHS, LScalarIx, RHS, RScalarIx);
    case VectorExpr::Kind::Convert:
    case VectorExpr::Kind::ConvertLinear:
      return isIdenticalScalarExprForConverts(LHS, LScalarIx, RHS, RScalarIx);
    case VectorExpr::Kind::TensorLoad:
      return isIdenticalScalarExprForTensorLd(LHS, LScalarIx, RHS, RScalarIx);
    case VectorExpr::Kind::TensorStore:
      return isIdenticalScalarExprForTensorSt(LHS, LScalarIx, RHS, RScalarIx);
    case VectorExpr::Kind::TensorBroadcastLoad:
      // tpc.ld.g has no swizzles, regular check is nice here.
      // Operands:
      //    0 - load address
      //    1 - switches
      //    2 - income
      //    3 - predicate
      //    4 - polarity
      return isIdenticalScalarExprForInstructionWithPartialSwizzles(
          LHS.getKind(), LHS, LScalarIx, RHS, RScalarIx, {}, {0, 1, 3, 4});
    case VectorExpr::Kind::GenAddr:
      // Operands:
      //    0 - indices
      //    1 - tensor id
      //    2 - switches
      //    4 - predicate
      //    5 - polarity
      return isIdenticalScalarExprForInstructionWithPartialSwizzles(
          VectorExpr::Kind::GenAddr, LHS, LScalarIx, RHS, RScalarIx, {},
          {0, 1, 2, 4, 5});
    }
    llvm_unreachable("unexpected expression kind");
  }

  /// Check for identical scalar expressions if there is not related vector
  /// expression in the given DDG sub graph and llvm::Value is available only.
  ///
  /// \details Performs check for values only without deep dive into DDG for
  ///     simplicity.
  bool isIdenticalScalarExpr(const Value *LOp, const unsigned LScalarIx,
                             const Value *ROp, const unsigned RScalarIx) {
    if (LOp != ROp) {
      LLVM_DEBUG(dbgs() << "Not the same value:\n"
                        << "  LOp: " << *LOp << "\n"
                        << "  ROp: " << *ROp << "\n");
      return false;
    }

    // It is a check for the same Value without deep dive into sub expressions
    // by design. So, indices equality (the most frequent scenario) must lead to
    // identical expression.
    if (LScalarIx == RScalarIx)
      return true;

    // Allow both undef.
    if (dyn_cast<UndefValue>(LOp) != nullptr)
      return true;

    Type *Ty = LOp->getType();

    // Allow scalars.
    if (Ty->isSingleValueType() && !Ty->isVectorTy())
      return true;

    // Checks for constants.
    const Constant *C = dyn_cast<Constant>(LOp);
    if (C) {
      // Allow splats.
      if (C->getSplatValue())
        return true;

      // Check for single constant vector element if possible.
      if (const ConstantVector *CV = dyn_cast<ConstantVector>(C)) {
        // TODO: should be an assert, but not right before release drop.
        if (CV->getType()->getNumElements() <= LScalarIx ||
            CV->getType()->getNumElements() <= RScalarIx) {
          LLVM_DEBUG(dbgs() << "Ix request out of bounds:\n"
                            << "  Value: " << *CV << "\n"
                            << "  LScalarIx: " << LScalarIx << "\n"
                            << "  RScalarIx: " << RScalarIx << "\n"
                            << "  NumElements: "
                            << CV->getType()->getNumElements() << "\n");
          return false;
        }

        const Constant *LC = CV->getAggregateElement(LScalarIx);
        const Constant *RC = CV->getAggregateElement(RScalarIx);
        if (!LC || !RC) {
          LLVM_DEBUG(dbgs() << "Failed to get element of vector:\n"
                            << "  Value: " << *CV << "\n"
                            << "  LScalarIx: " << LScalarIx << "\n"
                            << "  RScalarIx: " << RScalarIx << "\n");
          return false;
        }

        const ConstantInt *LCI = dyn_cast<ConstantInt>(LC);
        const ConstantInt *RCI = dyn_cast<ConstantInt>(RC);
        if (LCI && RCI && LCI->getSExtValue() == RCI->getSExtValue())
          return true;

        const ConstantFP *LCF = dyn_cast<ConstantFP>(LC);
        const ConstantFP *RCF = dyn_cast<ConstantFP>(RC);
        if (LCF && RCF && LCF->getValue().bitwiseIsEqual(RCF->getValue()))
          return true;

        LLVM_DEBUG(dbgs() << "Not recognized as identical vectors:\n"
                          << "  LC: " << *LC << "\n"
                          << "  RC: " << *RC << "\n");
        return false;
      }
      if (const ConstantDataVector *CDV = dyn_cast<ConstantDataVector>(C)) {
        // TODO: should be an assert, but not right before release drop.
        if (CDV->getType()->getNumElements() <= LScalarIx ||
            CDV->getType()->getNumElements() <= RScalarIx) {
          LLVM_DEBUG(dbgs() << "Ix request out of bounds:\n"
                            << "  Value: " << *CDV << "\n"
                            << "  LScalarIx: " << LScalarIx << "\n"
                            << "  RScalarIx: " << RScalarIx << "\n"
                            << "  NumElements: "
                            << CDV->getType()->getNumElements() << "\n");
          return false;
        }

        if (CDV->getElementType()->isIntegerTy()) {
          if (CDV->getElementAsInteger(LScalarIx) !=
              CDV->getElementAsInteger(RScalarIx)) {
            LLVM_DEBUG(dbgs() << "Not equal constants:\n"
                              << "  Value: " << *CDV << "\n"
                              << "  LScalarIx: " << LScalarIx << "\n"
                              << "  RScalarIx: " << RScalarIx << "\n");
            return false;
          }
          return true;
        }

        if (CDV->getElementType()->isFloatTy()) {
          if (CDV->getElementAsFloat(LScalarIx) !=
              CDV->getElementAsFloat(RScalarIx)) {
            LLVM_DEBUG(dbgs() << "Not equal constants:\n"
                              << "  Value: " << *CDV << "\n"
                              << "  LScalarIx: " << LScalarIx << "\n"
                              << "  RScalarIx: " << RScalarIx << "\n");
            return false;
          }
          return true;
        }

        LLVM_DEBUG(dbgs() << "Not recognized as identical constants:\n"
                          << "  Value: " << *CDV << "\n"
                          << "  LScalarIx: " << LScalarIx << "\n"
                          << "  RScalarIx: " << RScalarIx << "\n");
        return false;
      }

      // Avoid deeper recursion check for ConstantExpr for now. It might be
      // implemented in future if required.
      LLVM_DEBUG(dbgs() << "Not recognized as identical constants:\n"
                        << "  Value: " << *C << "\n"
                        << "  LScalarIx: " << LScalarIx << "\n"
                        << "  RScalarIx: " << RScalarIx << "\n");
      return false;
    }

    // Avoid complex recursion analysis. This simple check should cover most
    // optimization scenarios.
    const bool IsIdentityGuaranteed =
        Ty->isVectorTy() && LScalarIx == RScalarIx;
    if (!IsIdentityGuaranteed) {
      LLVM_DEBUG(dbgs() << "Not recognized as identical values:\n"
                        << "  Value: " << *LOp << "\n"
                        << "  LScalarIx: " << LScalarIx << "\n"
                        << "  RScalarIx: " << RScalarIx << "\n");
      return false;
    }

    return true;
  }

  bool checkOperandForScalarExprIdentity(const VectorExpr &LHS,
                                         const unsigned LScalarIx,
                                         const unsigned LOperandIx,
                                         const VectorExpr &RHS,
                                         const unsigned RScalarIx,
                                         const unsigned ROperandIx) {
    const Instruction &LI = LHS.instruction();
    const Instruction &RI = RHS.instruction();
    assert(LOperandIx < LI.getNumOperands());
    assert(ROperandIx < RI.getNumOperands());

    const Value *LOp = LI.getOperand(LOperandIx);
    const Value *ROp = RI.getOperand(ROperandIx);

    const VectorExprResult LExpr = LHS.getExpression(LOp);
    const VectorExprResult RExpr = RHS.getExpression(LOp);

    if (LExpr.returnCode() != RExpr.returnCode()) {
      LLVM_DEBUG(dbgs() << "Expression mismatch in graphs:\n"
                        << *LOp << "\n"
                        << *ROp << "\n");
      return false;
    }

    switch (LExpr.returnCode()) {
    case VectorExprResult::ReturnCode::SUCCEED:
      return isIdenticalScalarExpr(LExpr.expr(), LScalarIx, RExpr.expr(),
                                   RScalarIx);
    case VectorExprResult::ReturnCode::NOT_AN_INSTRUCTION:
    case VectorExprResult::ReturnCode::NOT_IN_COMPONENT:
      return isIdenticalScalarExpr(LOp, LScalarIx, ROp, RScalarIx);
    case VectorExprResult::ReturnCode::NOT_SUPPORTED:
      LLVM_DEBUG(dbgs() << "Expression for unsupported instruction:\n"
                        << "  LOp: " << *LOp << "\n"
                        << "  ROp: " << *ROp << "\n");
      return false;
    }

    llvm_unreachable("unexpected return code");
  }

  bool checkOperandsForScalarExprIdentity(
      const VectorExpr &LHS, const unsigned LScalarIx,
      const ArrayRef<unsigned> LSwizzledOperandsIxs, const VectorExpr &RHS,
      const unsigned RScalarIx, const ArrayRef<unsigned> RSwizzledOperandsIxs,
      const ArrayRef<unsigned> FullEqualityCheckOperandsIxs) {
    const Instruction &LI = LHS.instruction();
    const Instruction &RI = RHS.instruction();

    const unsigned OperandsCount = LI.getNumOperands();
    if (RI.getNumOperands() != OperandsCount)
      return false;

    const auto GetSrcIx = [](const Instruction &I, const unsigned OperandIx,
                             const ArrayRef<unsigned> SwizzledOperandsIxs,
                             const unsigned DstScalarIx) -> Optional<unsigned> {
      if (!is_contained(SwizzledOperandsIxs, OperandIx))
        return DstScalarIx;

      const Type *DstTy = I.getType();
      const Type *SrcTy = I.getOperand(OperandIx)->getType();
      const Optional<unsigned> Ix =
          getSwizzleSourceIx(DstScalarIx, DstTy, SrcTy);
      LLVM_DEBUG(if (!Ix.hasValue()) {
        dbgs() << "Failed to get source index for:\n"
               << "  DstIx: " << DstScalarIx << "\n"
               << "  OperandIx: " << OperandIx << "\n"
               << "  DstTy: " << *DstTy << "\n"
               << "  SrcTy: " << *SrcTy << "\n"
               << "  Instr: " << I << "\n";
      });
      return Ix;
    };

    for (unsigned OpIx = 0; OpIx < OperandsCount; ++OpIx) {
      const Type* const OperandTy = LI.getOperand(OpIx)->getType();
      const bool IsFullCheckTy =
          OperandTy->isSingleValueType() && !OperandTy->isVectorTy();
      if (IsFullCheckTy || is_contained(FullEqualityCheckOperandsIxs, OpIx)) {
        if (!fullCheckOperandForEquality(LHS, RHS, OpIx)) {
          LLVM_DEBUG(dbgs() << "Non-equal operands\n"
                            << LI << "\n"
                            << RI << "\n"
                            << "  OpIx: " << OpIx << "\n");
          return false;
        }
        continue;
      }

      const Optional<unsigned> SrcLIx =
          GetSrcIx(LI, OpIx, LSwizzledOperandsIxs, LScalarIx);
      const Optional<unsigned> SrcRIx =
          GetSrcIx(RI, OpIx, RSwizzledOperandsIxs, RScalarIx);
      if (!SrcLIx.hasValue() || !SrcRIx.hasValue())
        return false;

      if (!checkOperandForScalarExprIdentity(LHS, SrcLIx.getValue(), OpIx, RHS,
                                             SrcRIx.getValue(), OpIx)) {
        LLVM_DEBUG(dbgs() << "Operands scalar expr identity check failed\n"
                          << LI << "\n"
                          << RI << "\n"
                          << "  OpIx: " << OpIx << "\n"
                          << "  DstLIx: " << LScalarIx
                          << "  <- SrcLIx: " << SrcLIx.getValue() << "\n"
                          << "  DstRIx: " << RScalarIx
                          << "  <- SrcRIx: " << SrcRIx.getValue() << "\n");

        return false;
      }
    }
    return true;
  }

  bool isIdenticalScalarExprForInstructionWithPartialSwizzles(
      const VectorExpr::Kind ExpectedVEKind, const VectorExpr &LHS,
      const unsigned LScalarIx, const VectorExpr &RHS, const unsigned RScalarIx,
      const ArrayRef<unsigned> SwizzledOperandsIxs,
      const ArrayRef<unsigned> FullEqualityCheckOperandsIxs) {
    const Instruction &LI = LHS.instruction();
    const Instruction &RI = RHS.instruction();
    return RHS.getKind() == ExpectedVEKind && LHS.getKind() == ExpectedVEKind &&
           checkSameVEReturnType(LI, RI) &&
           checkOperandsForScalarExprIdentity(
               LHS, LScalarIx, SwizzledOperandsIxs, RHS, RScalarIx,
               SwizzledOperandsIxs, FullEqualityCheckOperandsIxs);
  }

  bool isIdenticalScalarExprForShuffle(const VectorExpr &LHS,
                                       const unsigned LScalarIx,
                                       const VectorExpr &RHS,
                                       const unsigned RScalarIx) {

    const Instruction &LI = LHS.instruction();
    const Instruction &RI = RHS.instruction();

    if (LHS.getKind() != VectorExpr::Kind::ShuffleVector ||
        RHS.getKind() != VectorExpr::Kind::ShuffleVector ||
        !checkSameVEReturnType(LI, RI))
      return false;

    const ShuffleVectorInst *LShuffleI = dyn_cast<ShuffleVectorInst>(&LI);
    const ShuffleVectorInst *RShuffleI = dyn_cast<ShuffleVectorInst>(&RI);
    assert(LShuffleI);
    assert(RShuffleI);
    assert(LI.getNumOperands() >= 2);
    assert(RI.getNumOperands() >= 2);

    const auto CheckFixedVectorsOfSameSize = [](const Value &LV,
                                                const Value &RV) {
      const FixedVectorType *LFVTy = cast<FixedVectorType>(LV.getType());
      const FixedVectorType *RFVTy = cast<FixedVectorType>(RV.getType());
      const bool IsSame = LFVTy && RFVTy &&
                          LFVTy->getNumElements() == RFVTy->getNumElements() &&
                          LFVTy->getScalarType()->getTypeID() ==
                              RFVTy->getScalarType()->getTypeID();
      LLVM_DEBUG(if (!IsSame) {
        dbgs() << "Does not look like a same fixed vector types:\n"
               << "  " << LV << "\n"
               << "  " << RV << "\n";
      });
      return IsSame;
    };

    const Value *LV0 = LI.getOperand(0);
    const Value *RV0 = RI.getOperand(0);
    if (!CheckFixedVectorsOfSameSize(*LV0, *RV0))
      return false;

    const Value *LV1 = LI.getOperand(1);
    const Value *RV1 = RI.getOperand(1);
    if (!CheckFixedVectorsOfSameSize(*LV1, *RV1))
      return false;

    const unsigned V0Size = getNumScalarsInType(LV0->getType()).getValue();

    const int LMaskIx = LShuffleI->getMaskValue(LScalarIx);
    const int RMaskIx = RShuffleI->getMaskValue(RScalarIx);

    const bool IsLUndef = LMaskIx == -1;
    const bool IsRUndef = LMaskIx == -1;

    if (IsLUndef != IsRUndef) {
      LLVM_DEBUG(dbgs() << "Not equivalent undef index:\n"
                        << "  LScalarIx: " << LScalarIx << "\n"
                        << "  RScalarIx: " << RScalarIx << "\n"
                        << "  LMaskIx: " << LMaskIx << "\n"
                        << "  RMaskIx: " << RMaskIx << "\n"
                        << LI << "\n"
                        << RI << "\n");
      return false;
    }

    // Case: undef.
    if (IsLUndef)
      return true;

    // Case: defined value.
    assert(LMaskIx >= 0);
    assert(RMaskIx >= 0);
    const unsigned LMaskUIx = LMaskIx;
    const unsigned RMaskUIx = RMaskIx;
    const unsigned LIx = LMaskUIx < V0Size ? LMaskUIx : LMaskUIx - V0Size;
    const unsigned RIx = RMaskUIx < V0Size ? RMaskUIx : RMaskUIx - V0Size;
    const unsigned LOpIx = LMaskUIx < V0Size ? 0 : 1;
    const unsigned ROpIx = RMaskUIx < V0Size ? 0 : 1;
    if (!checkOperandForScalarExprIdentity(LHS, LIx, LOpIx, RHS, RIx, ROpIx)) {
      LLVM_DEBUG(
          dbgs() << "Not equivalent expression for shuffle result index:\n"
                 << "  LScalarIx: " << LScalarIx << "\n"
                 << "  RScalarIx: " << RScalarIx << "\n"
                 << "  LMaskIx: " << LMaskIx << "\n"
                 << "  RMaskIx: " << RMaskIx << "\n"
                 << "  LIx: " << LIx << "\n"
                 << "  RIx: " << RIx << "\n"
                 << "  LOpIx: " << LOpIx << "\n"
                 << "  ROpIx: " << ROpIx << "\n"
                 << LI << "\n"
                 << RI << "\n");
      return false;
    }

    return true;
  }

  /// Check whether expressions sub tree for the given scalars are identical in
  /// the given expressions.
  ///
  /// \details Expressions are expected to be created from tpc.convert.linear or
  ///     tpc.convert instructions.
  bool isIdenticalScalarExprForConverts(const VectorExpr &LHS,
                                        const unsigned LScalarIx,
                                        const VectorExpr &RHS,
                                        const unsigned RScalarIx) {
    const Instruction &LI = LHS.instruction();
    const Instruction &RI = RHS.instruction();
    if ((RHS.getKind() != VectorExpr::Kind::Convert &&
         RHS.getKind() != VectorExpr::Kind::ConvertLinear) ||
        (LHS.getKind() != VectorExpr::Kind::Convert &&
         LHS.getKind() != VectorExpr::Kind::ConvertLinear) ||
        !checkSameVEReturnType(LI, RI))
      return false;
    assert(isTpcConvertLinearIntrinsic(LI) || isTpcConvertIntrinsic(LI));
    assert(isTpcConvertLinearIntrinsic(RI) || isTpcConvertIntrinsic(RI));

    SmallVector<unsigned, 2> LSwizzledOpIxs;
    if (LHS.getKind() != VectorExpr::Kind::ConvertLinear)
      LSwizzledOpIxs.push_back(0);

    SmallVector<unsigned, 2> RSwizzledOpIxs;
    if (RHS.getKind() != VectorExpr::Kind::ConvertLinear)
      RSwizzledOpIxs.push_back(0);

    if (!checkOperandsForScalarExprIdentity(LHS, LScalarIx, LSwizzledOpIxs, RHS,
                                            RScalarIx, RSwizzledOpIxs, {})) {
      LLVM_DEBUG(
          dbgs()
          << "Operands scalar expr identity check failed for conversions:\n"
          << LI << "\n"
          << RI << "\n"
          << "  DstLIx: " << LScalarIx << "  <- SrcLIx: " << LScalarIx << "\n"
          << "  DstRIx: " << RScalarIx << "  <- SrcRIx: " << RScalarIx << "\n");
      return false;
    }

    return true;
  }

  bool isIdenticalScalarExprForTensorSt(const VectorExpr &LHS,
                                        const unsigned LScalarIx,
                                        const VectorExpr &RHS,
                                        const unsigned RScalarIx) {
    const Instruction &LI = LHS.instruction();
    const Instruction &RI = RHS.instruction();
    if (LHS.getKind() != VectorExpr::Kind::TensorStore ||
        RHS.getKind() != VectorExpr::Kind::TensorStore ||
        !checkSameVEReturnType(LI, RI))
      return false;
    assert(isTensorStIntrinsic(LI));
    assert(isTensorStIntrinsic(RI));
    assert(LI.getNumOperands() == RI.getNumOperands());
    assert(LI.getNumOperands() >= 5); // Extra operands for metadata should not
                                      // affect pass.

    // Operands:
    //    0 - tensor indices
    //    1 - tensor id
    //    2 - source vector
    //    3 - switches
    //    4 - predicate
    //    5 - polarity
    //
    // 0, 1, 4, 5 - must be fully equal without respect to LScalarIx /
    // RScalarIx. 2          - depends on LScalarIx / RScalarIx. 3          -
    // controlled by transformation algorithm.

    // Tensor indices, tensor id, predicate, polarity.
    for (const unsigned OpIx : {0, 1, 4, 5}) {
      if (!fullCheckOperandForEquality(LHS, RHS, OpIx)) {
        LLVM_DEBUG(dbgs() << "Non-equal operands\n"
                          << LI << "\n"
                          << RI << "\n"
                          << "  OpIx: " << OpIx << "\n");
        return false;
      }
    }

    // Source vector.
    const auto GetTensorStoreSrcIx =
        [](const VectorExpr &VE, const unsigned DstScalarIx) -> unsigned {
      const std::uint64_t SwitchPatch = VE.getSwitchPatch();
      const unsigned NumElements =
          getNumScalarsInType(VE.instruction().getOperand(2)->getType())
              .getValue();

      if (SwitchPatch == SwPack32To16)
        return getSwizzleSourceIx(DstScalarIx, NumElements, 16, 32);

      if (SwitchPatch == SwPack32To8)
        return getSwizzleSourceIx(DstScalarIx, NumElements, 8, 32);

      if (SwitchPatch == SwPack16To8)
        return getSwizzleSourceIx(DstScalarIx, NumElements, 8, 16);

      assert(SwitchPatch == 0);
      return DstScalarIx;
    };
    const unsigned SrcLIx = GetTensorStoreSrcIx(LHS, LScalarIx);
    const unsigned SrcRIx = GetTensorStoreSrcIx(RHS, RScalarIx);
    if (!checkOperandForScalarExprIdentity(LHS, SrcLIx, 2, RHS, SrcRIx, 2)) {
      LLVM_DEBUG(dbgs() << "Operands scalar expr identity check failed\n"
                        << LI << "\n"
                        << RI << "\n"
                        << "  LIx: " << LScalarIx << " <- " << SrcLIx << "\n"
                        << "  RIx: " << RScalarIx << " <- " << SrcRIx << "\n");
      return false;
    }

    return true;
  }

  bool isIdenticalScalarExprForTensorLd(const VectorExpr &LHS,
                                        const unsigned LScalarIx,
                                        const VectorExpr &RHS,
                                        const unsigned RScalarIx) {
    const Instruction &LI = LHS.instruction();
    const Instruction &RI = RHS.instruction();
    if (LHS.getKind() != VectorExpr::Kind::TensorLoad ||
        RHS.getKind() != VectorExpr::Kind::TensorLoad ||
        !checkSameVEReturnType(LI, RI))
      return false;
    assert(isTensorLdIntrinsic(LI));
    assert(isTensorLdIntrinsic(RI));

    const unsigned OperandsCount = LI.getNumOperands();
    if (RI.getNumOperands() != OperandsCount)
      return false;

    // Check memory indices.
    const auto GetTensorLoadSrcIx = [](const VectorExpr &VE,
                                       const unsigned DstScalarIx) -> unsigned {
      const std::uint64_t SwitchPatch = VE.getSwitchPatch();
      const unsigned NumElements =
          getNumScalarsInType(VE.instruction().getType()).getValue();

      if (SwitchPatch == (TPCII::SW_UNPACK | TPCII::SW_UNPCK_16_TO_32))
        return getSwizzleSourceIx(DstScalarIx, NumElements, 32, 16);

      if (SwitchPatch == (TPCII::SW_UNPACK | TPCII::SW_UNPCK_8_TO_32))
        return getSwizzleSourceIx(DstScalarIx, NumElements, 32, 8);

      if (SwitchPatch == (TPCII::SW_UNPACK | TPCII::SW_UNPCK_8_TO_16))
        return getSwizzleSourceIx(DstScalarIx, NumElements, 16, 8);

      assert(SwitchPatch == 0);
      return DstScalarIx;
    };
    const unsigned LoadLSrcIx = GetTensorLoadSrcIx(LHS, LScalarIx);
    const unsigned LoadRSrcIx = GetTensorLoadSrcIx(RHS, RScalarIx);
    if (LoadLSrcIx != LoadRSrcIx) {
      LLVM_DEBUG(
          dbgs() << "Mismatch in tensor load memory indices\n"
                 << LI << "\n"
                 << RI << "\n"
                 << "  DstLIx: " << LScalarIx << " <- " << LoadLSrcIx << "\n"
                 << "  DstRIx: " << RScalarIx << " <- " << LoadRSrcIx << "\n");
      return false;
    }

    // Tensor load operands:
    //    0 - loading indices
    //    1 - tensor id
    //    2 - switches
    //    3 - income
    //    4 - predicate
    //    5 - polarity
    //
    //  0, 1, 4, 5 - should not depend on result indices swizzle, must be equal
    //               between LHS and RHS
    //  2          - controlled by checking algorithm
    //  3          - directly maps to result, must be checked for indices
    //               mapping

    // Indices, tensor id, predicate, polarity.
    for (const unsigned OpIx : {0, 1, 4, 5}) {
      if (!fullCheckOperandForEquality(LHS, RHS, OpIx)) {
        LLVM_DEBUG(
            dbgs() << "Mismatch in tpd.tnsr.ld operand has been detected\n"
                   << LI << "\n"
                   << RI << "\n"
                   << "  OpIx: " << OpIx << "\n");
        return false;
      }
    }
    // Income.
    const unsigned IncomeOpIx = 3;
    if (!checkOperandForScalarExprIdentity(LHS, LScalarIx, IncomeOpIx, RHS,
                                           RScalarIx, IncomeOpIx)) {
      LLVM_DEBUG(
          dbgs() << "Operands scalar expr identity check failed for income\n"
                 << LI << "\n"
                 << RI << "\n"
                 << "  LScalarIx " << LScalarIx << "\n"
                 << "  RScalarIx " << RScalarIx << "\n");
      return false;
    }
    return true;
  }

  /// Function check that the given operand in two vector expressions are fully
  /// equal by all indices. Other functions checks only the given vector index.
  bool fullCheckOperandForEquality(const VectorExpr &LHS, const VectorExpr &RHS,
                                   const unsigned OperandIx) {
    const Instruction &LI = LHS.instruction();
    const Instruction &RI = RHS.instruction();
    assert(&LI == &RI);

    Optional<bool> &Result =
        FullOperandCheckCachedResults[std::make_pair(&LI, OperandIx)];

    if (Result.hasValue())
      return Result.getValue();

    Result = fullCheckOperandForEqualityImpl(LHS, RHS, OperandIx);
    return Result.getValue();
  }

  bool fullCheckOperandForEqualityImpl(const VectorExpr &LHS,
                                       const VectorExpr &RHS,
                                       const unsigned OperandIx) {
    const Instruction &LI = LHS.instruction();
    const Instruction &RI = RHS.instruction();

    const Value *const LIxsValue = LI.getOperand(OperandIx);
    const Value *const RIxsValue = RI.getOperand(OperandIx);
    const Type *const LIxsTy = LIxsValue->getType();
    const Type *const RIxsTy = RIxsValue->getType();
    const Optional<unsigned> NumScalarsL = getNumScalarsInType(LIxsTy);
    const Optional<unsigned> NumScalarsR = getNumScalarsInType(RIxsTy);
    if (NumScalarsL != NumScalarsR || !NumScalarsL.hasValue()) {
      LLVM_DEBUG(dbgs() << "Unexpected different scalars count in operands\n"
                        << LI << "\n"
                        << RI << "\n"
                        << "  OperandIx: " << OperandIx << "\n");
      return false;
    }
    const unsigned NumIndices = NumScalarsL.getValue();
    for (unsigned I = 0; I < NumIndices; ++I) {
      if (!checkOperandForScalarExprIdentity(LHS, I, OperandIx, RHS, I,
                                             OperandIx)) {
        LLVM_DEBUG(
            dbgs() << "Operands scalar expr identity check failed for indices\n"
                   << LI << "\n"
                   << RI << "\n"
                   << "  I " << I << "\n");
        return false;
      }
    }
    return true;
  }

  DenseMap<std::pair<const Instruction *, unsigned>, Optional<bool>>
      FullOperandCheckCachedResults;

  const GraphComponent *TargetComponent;
};

namespace {
struct TransformPlan {
  DDGNodes TPCConvertLinearNodes;
  NodeToSwitchPatch TensorLdNodeToSwitchPatch;
  NodeToSwitchPatch TensorStNodeToSwitchPatch;
};
} // namespace

/// Provider for vector indices expressions.
///     1. Constructs VectorExpr for the given DDG node / instruction.
///     2. Owns VectorExpr objects.
class VectorExprProvider {
public:
  explicit VectorExprProvider(const GraphComponent &InComponent)
      : VectorExprProvider(InComponent, nullptr) {}

  VectorExprProvider(const GraphComponent &InComponent,
                     const TransformPlan &InPlan)
      : VectorExprProvider(InComponent, &InPlan) {}

  VectorExprProvider(const VectorExprProvider &) = delete;
  VectorExprProvider(VectorExprProvider &&) = delete;
  VectorExprProvider &operator=(const VectorExprProvider &) = delete;
  VectorExprProvider &operator=(VectorExprProvider &&) = delete;

  VectorExprResult construct(const DDGNode &N) {
    assert(Component.get().nodes().contains(&N));
    assert(isSingleInstructionSimpleNode(N));

    auto It = NodeToExpr.find(&N);
    if (It != NodeToExpr.end())
      return VectorExprResult(*It->second);

    return constructImpl(N);
  }

  VectorExprResult construct(const Instruction &I) {
    auto It = InstructionToNode.find(&I);
    if (It == InstructionToNode.end())
      return VectorExprResult(VectorExprResult::ReturnCode::NOT_IN_COMPONENT);

    return construct(*It->second);
  }

  const GraphComponent &component() const { return Component.get(); }

private:
  VectorExprProvider(const GraphComponent &InComponent,
                     const TransformPlan *InPlan)
      : Component(InComponent), Plan(InPlan) {
    const DDGNodes &AllNodes = Component.get().nodes();
    InstructionToNode.reserve(AllNodes.size());
    for (const DDGNode *Node : AllNodes)
      InstructionToNode[getFirstInstruction(*Node)] = Node;
  }

  VectorExprResult constructImpl(const DDGNode &N) {
    assert(Component.get().nodes().contains(&N));
    assert(isSingleInstructionSimpleNode(N));
    assert(NodeToExpr.count(&N) == 0);

    const Optional<VectorExpr::Kind> VEKind = getVEKind(N);
    if (!VEKind.hasValue()) {
      // More instructions might be supported if required.
      LLVM_DEBUG(dbgs() << "Unsupported instruction: "
                        << *getFirstInstruction(N) << "\n");
      return VectorExprResult(VectorExprResult::ReturnCode::NOT_SUPPORTED);
    }

    auto Result = std::make_unique<VectorExpr>(VEKind.getValue(), N,
                                               getNodeSwitchPatch(N), *this);
    VectorExpr *RV = Result.get();
    NodeToExpr[&N] = std::move(Result);
    return VectorExprResult(*RV);
  }

  Optional<VectorExpr::Kind> getVEKind(const DDGNode &N) const {
    assert(isSingleInstructionSimpleNode(N));

    Instruction *const I = getFirstInstruction(N);

    // Actually due to algorithm internals compiler is not required to
    // distinguish ordered and unordered compositions, because proposed
    // transformations do not change arguments order, only elements ordering
    // inside argument.
    //
    // Let's use this trick and perform only ordered checks for speed and
    // simplicity. No need to check permutations for unordered compositions.
    if (auto *Int = dyn_cast<IntrinsicInst>(I)) {
      switch (Int->getIntrinsicID()) {
        // Single argument
      case Intrinsic::fabs:
      case Intrinsic::tpc_abs:
      case Intrinsic::tpc_fclass:
        // Unordered composition.
      case Intrinsic::maxnum:
      case Intrinsic::minnum:
      case Intrinsic::tpc_add:
      case Intrinsic::tpc_max:
      case Intrinsic::tpc_min:
        // Ordered composition.
      case Intrinsic::tpc_calc_fp_special:
      case Intrinsic::tpc_sub:
        return VectorExpr::Kind::Composition;

      case Intrinsic::tpc_mac:
        return VectorExpr::Kind::TPCMac;
      case Intrinsic::tpc_madd:
        return VectorExpr::Kind::TPCMadd;
      case Intrinsic::tpc_mul:
        return VectorExpr::Kind::TPCMul;

      case Intrinsic::tpc_ld_tnsr:
        // Vector predicate is not supported for tpc_ld_tnsr intrinsics.
        if (Int->getOperand(4)->getType()->isVectorTy())
          return None;
        assert(Int->getOperand(4)->getType()->isIntegerTy());

        // If tensor load instruction is going to be patched and already has
        // another unpack switch, there is no way to patch such instruction.
        // Stop analysis at this point.
        if (getTensorStLdSwitchValue(*Int) & TPCII::SW_UNPACK)
          return None;

        return VectorExpr::Kind::TensorLoad;

      case Intrinsic::tpc_ld_g:
        // Vector predicate is not supported for tpc_ld_g intrinsics.
        if (Int->getOperand(3)->getType()->isVectorTy())
          return None;
        assert(Int->getOperand(3)->getType()->isIntegerTy());

        return VectorExpr::Kind::TensorBroadcastLoad;

      case Intrinsic::tpc_gen_addr:
        return VectorExpr::Kind::GenAddr;

      case Intrinsic::tpc_st_tnsr:
        // If tensor store instruction is going to be patched and already has
        // another pack switch, there is no way to patch such instruction.
        // Stop analysis at this point.
        if (getTensorStLdSwitchValue(*Int) & TPCII::SW_PACK)
          return None;

        return VectorExpr::Kind::TensorStore;

      case Intrinsic::tpc_convert:
        // Vector predicate is not supported for tpc_convert intrinsics.
        if (Int->getOperand(4)->getType()->isVectorTy())
          return None;
        assert(Int->getOperand(4)->getType()->isIntegerTy());

        return VectorExpr::Kind::Convert;

      case Intrinsic::tpc_convert_linear:
        // Vector predicate is not supported for tpc_convert_linear intrinsics.
        if (Int->getOperand(4)->getType()->isVectorTy())
          return None;
        assert(Int->getOperand(4)->getType()->isIntegerTy());

        return Plan && Plan->TPCConvertLinearNodes.contains(&N)
                   ? VectorExpr::Kind::Convert
                   : VectorExpr::Kind::ConvertLinear;
      }
      return None;
    }

    switch (I->getOpcode()) {
      // Single argument.
    case Instruction::Trunc:
    case Instruction::ZExt:
    case Instruction::SExt:
    case Instruction::FPToUI:
    case Instruction::FPToSI:
    case Instruction::UIToFP:
    case Instruction::SIToFP:
    case Instruction::FPTrunc:
    case Instruction::FPExt:
      // Unordered composition.
    case Instruction::Add:
    case Instruction::FAdd:
    case Instruction::Mul:
    case Instruction::FMul:
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Xor:
      // Ordered composition.
    case Instruction::Sub:
    case Instruction::FSub:
    case Instruction::UDiv:
    case Instruction::SDiv:
    case Instruction::URem:
    case Instruction::SRem:
    case Instruction::FCmp:
    case Instruction::ICmp:
    case Instruction::Select:
    case Instruction::PHI:
      return VectorExpr::Kind::Composition;

    case Instruction::ShuffleVector:
      return VectorExpr::Kind::ShuffleVector;
    }
    return None;
  }

  std::uint64_t getNodeSwitchPatch(const DDGNode &N) const {
    if (!Plan)
      return 0;

    if (isTensorStNode(N)) {
      auto It = Plan->TensorStNodeToSwitchPatch.find(&N);
      if (It != Plan->TensorStNodeToSwitchPatch.end())
        return It->second;
    }

    if (isTensorLdNode(N)) {
      auto It = Plan->TensorLdNodeToSwitchPatch.find(&N);
      if (It != Plan->TensorLdNodeToSwitchPatch.end())
        return It->second;
    }

    return 0;
  }

  std::reference_wrapper<const GraphComponent> Component;
  const TransformPlan *Plan = nullptr;

  DenseMap<const DDGNode *, std::unique_ptr<VectorExpr>> NodeToExpr;
  DenseMap<const Instruction *, const DDGNode *> InstructionToNode;
};

VectorExprResult VectorExpr::getExpression(const Value *V) const {
  if (const Instruction *Ins = dyn_cast<Instruction>(V))
    return ExprProvider.get().construct(*Ins);
  return VectorExprResult(VectorExprResult::ReturnCode::NOT_AN_INSTRUCTION);
}

const GraphComponent &VectorExpr::component() const {
  return ExprProvider.get().component();
}

} // namespace

static bool isTransformAffectsConvertNodeResult(const DDGNode &N) {
  assert(isSingleInstructionSimpleNode(N));

  if (isTpcConvertLinearNode(N))
    return isLinearityRemovalAffectsInstructionResult(*getFirstInstruction(N));

  return isTensorStNode(N) || isTensorLdNode(N);
}

/// Check if graph component input and output nodes have tpc.ld.tnsr /
/// tpc.st.tnsr instructions.
static bool hasTensorStLdInOuts(const GraphComponent &Component) {
  return any_of(Component.nodes(), [&](const DDGNode *N) {
    return Component.isBoundaryNode(*N) &&
           (isTensorLdNode(*N) || isTensorStNode(*N));
  });
}

static bool isSafeToTransform(const TransformPlan &Plan,
                              const GraphComponent &Component) {
  // Analysis for graphs with >= 2 instructions per node is not supported.
  if (any_of(Component.nodes(), [](const DDGNode *N) {
        return !isSingleInstructionSimpleNode(*N);
      })) {
    LLVM_DEBUG(
        dbgs() << "Deny transform. DDG contains multi instruction nodes.\n");
    return false;
  }

  // Performance trick: if component has only nodes which transformation does
  // not affect node result, just schedule these instructions to transform,
  // there is no sense to check against transform applicability, it is always
  // applicable.
  if (Plan.TensorStNodeToSwitchPatch.empty() &&
      Plan.TensorLdNodeToSwitchPatch.empty() &&
      all_of(Plan.TPCConvertLinearNodes, [](const DDGNode *N) {
        return !isTransformAffectsConvertNodeResult(*N);
      }))
    return true;

  if (DFSCycleChecker().containsCycle(Component)) {
    LLVM_DEBUG(dbgs() << "Deny transform. DDG contains cycle.\n");
    return false;
  }

  // Check results equal for output nodes after transform.
  VectorExprProvider VEPOriginal(Component);
  VectorExprProvider VEPTransformed(Component, Plan);
  for (const DDGNode *N : Component.nodes()) {
    if (!Component.isOutNode(*N))
      continue;

    if (!isTpcConvertLinearNode(*N) && !isTensorStNode(*N)) {
      LLVM_DEBUG(dbgs() << "Deny transform. Found unexpected output node:\n"
                        << "  " << *getFirstInstruction(*N) << "\n");
      return false;
    }

    const VectorExprResult VEO = VEPOriginal.construct(*N);
    const VectorExprResult VET = VEPTransformed.construct(*N);
    if (VEO.returnCode() != VectorExprResult::ReturnCode::SUCCEED ||
        VET.returnCode() != VectorExprResult::ReturnCode::SUCCEED ||
        !VectorExprEqChecker(Component).isIdentical(VEO.expr(), VET.expr())) {
      LLVM_DEBUG(dbgs() << "Deny transform. Can not guarantee transformed "
                           "expression equivalence for output instruction:\n"
                        << "  " << *getFirstInstruction(*N) << "\n");
      return false;
    }
  }

  return true;
}

static bool hasPath(const DDGNode &StartNode, const DDGNode &FinalNode,
                    const GraphComponent &Component) {
  DDGNodes Queue;
  DDGNodes Visited;
  Queue.insert(&StartNode);

  while (!Queue.empty()) {
    const DDGNode *N = *Queue.begin();
    Queue.erase(Queue.begin());

    if (N == &FinalNode)
      return true;

    Visited.insert(N);

    Component.enumerateFwdNeighbors(*N, [&](const DDGNode &Child) {
      if (!Visited.contains(&Child))
        Queue.insert(&Child);
    });
  }

  return false;
}

static Optional<std::uint64_t>
getSwitchPatchProposalForLoadUnpackFromOutConvert(
    const DDGNode &OutTPCConvertLinearNode, const DDGNode &InpTensorLdNode) {
  assert(isTpcConvertLinearNode(OutTPCConvertLinearNode));
  assert(isTensorLdNode(InpTensorLdNode));

  if (!isSingleInstructionSimpleNode(OutTPCConvertLinearNode) ||
      !isSingleInstructionSimpleNode(InpTensorLdNode))
    return None;

  const Instruction *ConvertI = getFirstInstruction(OutTPCConvertLinearNode);
  const unsigned ScalarSizeBitsOfConvertDstTy =
      ConvertI->getType()->getScalarSizeInBits();
  const unsigned ScalarSizeBitsOfConvertSrcTy =
      ConvertI->getOperand(0)->getType()->getScalarSizeInBits();
  const unsigned ScalarSizeBitsOfTensorLoadDstTy =
      getFirstInstruction(InpTensorLdNode)->getType()->getScalarSizeInBits();

  if (ScalarSizeBitsOfConvertSrcTy != ScalarSizeBitsOfTensorLoadDstTy)
    return None;

  if (ScalarSizeBitsOfConvertSrcTy == 16 && ScalarSizeBitsOfConvertDstTy == 32)
    return TPCII::SW_UNPCK_16_TO_32 | TPCII::SW_UNPACK;

  if (ScalarSizeBitsOfConvertSrcTy == 8 && ScalarSizeBitsOfConvertDstTy == 16)
    return TPCII::SW_UNPCK_8_TO_16 | TPCII::SW_UNPACK;

  if (ScalarSizeBitsOfConvertSrcTy == 8 && ScalarSizeBitsOfConvertDstTy == 32)
    return TPCII::SW_UNPCK_8_TO_32 | TPCII::SW_UNPACK;

  return None;
}

static Optional<std::uint64_t> getSwitchPatchProposalForStorePackFromInpConvert(
    const DDGNode &InpTPCConvertLinearNode, const DDGNode &OutTensorStNode) {
  assert(isTpcConvertLinearNode(InpTPCConvertLinearNode));
  assert(isTensorStNode(OutTensorStNode));

  if (!isSingleInstructionSimpleNode(InpTPCConvertLinearNode) ||
      !isSingleInstructionSimpleNode(OutTensorStNode))
    return None;

  const Instruction *ConvertI = getFirstInstruction(InpTPCConvertLinearNode);
  const unsigned ScalarSizeBitsOfConvertDstTy =
      ConvertI->getType()->getScalarSizeInBits();
  const unsigned ScalarSizeBitsOfConvertSrcTy =
      ConvertI->getOperand(0)->getType()->getScalarSizeInBits();
  const unsigned ScalarSizeBitsOfTensorStoreSrcTy =
      getFirstInstruction(OutTensorStNode)
          ->getOperand(2)
          ->getType()
          ->getScalarSizeInBits();

  if (ScalarSizeBitsOfConvertDstTy != ScalarSizeBitsOfTensorStoreSrcTy)
    return None;

  if (ScalarSizeBitsOfConvertSrcTy == 32 && ScalarSizeBitsOfConvertDstTy == 16)
    return SwPack32To16;

  if (ScalarSizeBitsOfConvertSrcTy == 16 && ScalarSizeBitsOfConvertDstTy == 8)
    return SwPack16To8;

  if (ScalarSizeBitsOfConvertSrcTy == 32 && ScalarSizeBitsOfConvertDstTy == 8)
    return SwPack32To8;

  return None;
}

static std::uint64_t getSwitchPatchProposalForInpTensorLdNode(
    const DDGNode &InputTensorLdNode,
    const DDGNodes &TPCConvertLinearBoundaryNodes,
    const GraphComponent &Component) {
  assert(Component.isInpNode(InputTensorLdNode));

  // Try to detect scenario:
  //    %vec_1 = @llvm.tpc.ld.tnsr ...
  //    %vec_2 = operation %vec_1 ...
  //    %vec_3 = @llvm.tpc.convert.linear %vec_2 ...
  for (const DDGNode *N : TPCConvertLinearBoundaryNodes) {
    if (!Component.isOutNode(*N))
      continue;

    if (!hasPath(InputTensorLdNode, *N, Component))
      continue;

    const Optional<std::uint64_t> SwitchPatch =
        getSwitchPatchProposalForLoadUnpackFromOutConvert(*N,
                                                          InputTensorLdNode);

    if (SwitchPatch.hasValue())
      return SwitchPatch.getValue();
  }

  // Leave instruction as is and hope it helps.
  return 0;
}

static std::uint64_t getSwitchPatchProposalForOutTensorStNode(
    const DDGNode &OutputTensorStNode,
    const DDGNodes &TPCConvertLinearBoundaryNodes,
    const GraphComponent &Component) {
  assert(Component.isOutNode(OutputTensorStNode));

  // Try to detect scenario:
  //    %vec_1 = @llvm.tpc.convert.linear ...
  //    %vec_2 = operation %vec_1 ...
  //    @llvm.tpc.st.tnsr %vec_2 ...
  for (const DDGNode *N : TPCConvertLinearBoundaryNodes) {
    if (!Component.isInpNode(*N))
      continue;

    if (!hasPath(*N, OutputTensorStNode, Component))
      continue;

    const Optional<std::uint64_t> SwitchPatch =
        getSwitchPatchProposalForStorePackFromInpConvert(*N,
                                                         OutputTensorStNode);

    if (SwitchPatch.hasValue())
      return SwitchPatch.getValue();
  }

  // Leave instruction as is and hope it helps.
  return 0;
}

/// Return proposal transform plan for the given component. There is no
/// guarantee that transform plan is correct. It should be checked separately.
static TransformPlan getTransformProposal(const GraphComponent &Component) {
  TransformPlan RV;

  // Collect tpc.convert.linear boundary nodes.
  for (const DDGNode *N : Component.nodes()) {
    if (Component.isBoundaryNode(*N) && isTpcConvertLinearNode(*N))
      RV.TPCConvertLinearNodes.insert(N);
  }

  // Collect tensor store / load nodes and make switch patch proposal.
  for (const DDGNode *N : Component.nodes()) {
    if (isTensorLdNode(*N) && Component.isInpNode(*N))
      RV.TensorLdNodeToSwitchPatch[N] =
          getSwitchPatchProposalForInpTensorLdNode(*N, RV.TPCConvertLinearNodes,
                                                   Component);

    if (isTensorStNode(*N) && Component.isOutNode(*N))
      RV.TensorStNodeToSwitchPatch[N] =
          getSwitchPatchProposalForOutTensorStNode(*N, RV.TPCConvertLinearNodes,
                                                   Component);
  }

  LLVM_DEBUG(if (!RV.TensorStNodeToSwitchPatch.empty() ||
                 !RV.TensorLdNodeToSwitchPatch.empty()) {
    dbgs() << "Switch patch proposals:\n";
    for (const auto &NodeAndPatch : RV.TensorLdNodeToSwitchPatch)
      dbgs() << "  " << NodeAndPatch.second << " for "
             << *getFirstInstruction(*NodeAndPatch.first) << "\n";
    for (const auto &NodeAndPatch : RV.TensorStNodeToSwitchPatch)
      dbgs() << "  " << NodeAndPatch.second << " for "
             << *getFirstInstruction(*NodeAndPatch.first) << "\n";
  });

  return RV;
}

static TransformPlan getTransformPlan(
    const GraphComponent &Component, const bool AllowTensorStorePack32To16,
    const bool AllowTensorStorePack32To8, const bool AllowTensorStorePack16To8,
    const bool AllowTensorLoadUnpackPatching) {
  const TransformPlan RV = getTransformProposal(Component);

  for (const auto &NodeAndPatch : RV.TensorStNodeToSwitchPatch) {
    if (NodeAndPatch.second == 0)
      continue;

    if (NodeAndPatch.second == SwPack32To16) {
      if (!AllowTensorStorePack32To16) {
        LLVM_DEBUG(dbgs() << "Skip. SW_PCK_32_TO_16 is disallowed.\n");
        return TransformPlan();
      }
    } else if (NodeAndPatch.second == SwPack32To8) {
      if (!AllowTensorStorePack32To8) {
        LLVM_DEBUG(dbgs() << "Skip. SW_PCK_32_TO_8 is disallowed.\n");
        return TransformPlan();
      }
    } else if (NodeAndPatch.second == SwPack16To8) {
      if (!AllowTensorStorePack16To8) {
        LLVM_DEBUG(dbgs() << "Skip. SW_PCK_16_TO_8 is disallowed.\n");
        return TransformPlan();
      }
    } else {
      errs() << "Unexpected patch value: " << NodeAndPatch.second << "\n";
      llvm_unreachable("unexpected store pack switch value");
    }
  }

  if (!AllowTensorLoadUnpackPatching && !RV.TensorLdNodeToSwitchPatch.empty()) {
    LLVM_DEBUG(
        dbgs() << "Skip component. SW_UNPACK is disallowed and component "
                  "requires tensor load instruction to be patched.\n");
    return TransformPlan();
  }

  if (!isSafeToTransform(RV, Component)) {
    LLVM_DEBUG(dbgs() << "Skip component. Proposed transform likely changes "
                         "program behavior.\n");
    return TransformPlan();
  }

  return RV;
}

static IntrinsicsSet getInstructions(const DDGNodes &Nodes) {
  IntrinsicsSet RV;
  for (const DDGNode *Node : Nodes) {
    assert(isSingleInstructionSimpleNode(*Node));
    assert(isTpcConvertLinearNode(*Node));
    RV.insert(dyn_cast<IntrinsicInst>(getFirstInstruction(*Node)));
  }
  return RV;
}

static IntrinsicToSwitchPatch
getIntrinsicToSwitchPatch(const NodeToSwitchPatch &NodeToPatch) {
  IntrinsicToSwitchPatch RV;
  for (const auto &NodeAndPatch : NodeToPatch) {
    const DDGNode *N = NodeAndPatch.first;
    assert(isSingleInstructionSimpleNode(*N));
    assert(isTensorStNode(*N) || isTensorLdNode(*N));
    IntrinsicInst *I = dyn_cast<IntrinsicInst>(getFirstInstruction(*N));
    assert(RV.count(I) == 0);
    RV[I] = NodeAndPatch.second;
  }
  return RV;
}

/// Returns container of SimpleDDGNode objects each one represents
/// tpc.convert.linear instruction in DDG.
static SimpleDDGNodes getTPCConvertLinearNodes(const DataDependenceGraph &DDG) {
  SimpleDDGNodes RV;
  SmallVector<const PiBlockDDGNode *, 8> PiBlocksQueue;

  const auto ProcessDDGNodesLayer = [&](const auto &NodesRange) {
    for (const DDGNode *N : NodesRange) {
      // Schedule pi-blocks for further processing.
      if (isa<PiBlockDDGNode>(*N)) {
        PiBlocksQueue.push_back(cast<const PiBlockDDGNode>(N));
        continue;
      }

      // Recognize tpc.convert.linear and add it to Nodes.
      if (isSingleInstructionSimpleNode(*N) && isTpcConvertLinearNode(*N))
        RV.insert(cast<const SimpleDDGNode>(N));
    }
  };

  ProcessDDGNodesLayer(DDG);
  while (!PiBlocksQueue.empty()) {
    const PiBlockDDGNode *N = PiBlocksQueue.pop_back_val();
    ProcessDDGNodesLayer(N->getNodes());
  }
  return RV;
}

using TargetInputNodeToComponentIndices =
    SmallDenseMap<const DDGNode *, SmallVector<unsigned, 4>, 8>;

static TargetInputNodeToComponentIndices
makeTargetInputNodeToComponentIndicesMappings(
    const SmallVector<GraphComponent, 8> &Components) {
  TargetInputNodeToComponentIndices RV;

  for (unsigned Ix = 0; Ix < Components.size(); ++Ix) {
    for (const DDGNode *N : Components[Ix].nodes()) {
      if (!isSingleInstructionSimpleNode(*N) ||
          !isTransformAffectsConvertNodeResult(*N))
        continue;

      if (Components[Ix].isInpNode(*N))
        RV[N].push_back(Ix);
    }
  }

  return RV;
}

static SmallVector<GraphComponent, 8> mergeComponentsWithCommonInputTargetNode(
    const SmallVector<GraphComponent, 8> &Components) {
  // Organize undirected graph G:
  //  * node is an input GraphComponent from Components container
  //  * there is an edge between nodes C1 and C2 iff graph components C1 and C2
  //    has the same input transform target node which changes behavior after
  //    transform. Output transform target is not interesting because
  //    transformation saves output result.
  //
  // Optimize a bit and let node be an index of Components container element.
  const SmallDenseMap<unsigned, SmallVector<unsigned, 4>, 16> Edges = [&]() {
    SmallDenseMap<unsigned, SmallVector<unsigned, 4>, 16> GEdges;

    const TargetInputNodeToComponentIndices TargetNodeToCompIndices =
        makeTargetInputNodeToComponentIndicesMappings(Components);

    for (const auto &NodeAndCompIndices : TargetNodeToCompIndices) {
      const SmallVector<unsigned, 4> &Indices = NodeAndCompIndices.second;
      for (unsigned LIx = 0; LIx < Indices.size(); ++LIx) {
        for (unsigned RIx = LIx + 1; RIx < Indices.size(); ++RIx) {
          GEdges[Indices[LIx]].push_back(Indices[RIx]);
          GEdges[Indices[RIx]].push_back(Indices[LIx]);
        }
      }
    }

    return GEdges;
  }();

  // Search for connected components in the graph G. Merge GraphComponent nodes
  // from the same connected component of G into larger GraphComponent.
  SmallVector<GraphComponent, 8> MergedComponents;

  SmallDenseSet<unsigned, 8> IndicesToProcess;
  IndicesToProcess.reserve(Components.size());
  for (unsigned Ix = 0; Ix < Components.size(); ++Ix)
    IndicesToProcess.insert(Ix);

  while (!IndicesToProcess.empty()) {
    const unsigned Ix = *IndicesToProcess.begin();
    IndicesToProcess.erase(IndicesToProcess.begin());

    // Build the connected component which contains node with index Ix.
    SmallDenseSet<unsigned, 8> ConnComponent;
    SmallDenseSet<unsigned, 8> Queue;
    Queue.insert(Ix);
    while (!Queue.empty()) {
      const unsigned IxFromQueue = *Queue.begin();
      Queue.erase(Queue.begin());

      ConnComponent.insert(IxFromQueue);

      auto It = Edges.find(IxFromQueue);
      if (It != Edges.end()) {
        for (const unsigned Neighbor : It->second) {
          if (!ConnComponent.contains(Neighbor))
            Queue.insert(Neighbor);
        }
      }
    }

    // Remove processed indices from processing queue.
    for (const unsigned CompIx : ConnComponent)
      IndicesToProcess.erase(CompIx);

    // Merge GraphComponent elements into connected component.
    assert(!ConnComponent.empty());

    DDGNodes UnitedNodes;
    for (const unsigned CompIx : ConnComponent)
      UnitedNodes.insert(Components[CompIx].nodes().begin(),
                         Components[CompIx].nodes().end());

    MergedComponents.emplace_back(std::move(UnitedNodes));
  }
  return MergedComponents;
}

namespace {
struct ComponentsToAnalyze {
  SmallVector<GraphComponent, 4> ComponentsWithLinearConvertInOuts;
  SmallVector<GraphComponent, 4> ComponentsWithLinearConvertAndStLdInOuts;
};
} // namespace

/// Note that result components might intersect by boundaries, i.e. input node
/// of one component might be an output node of another component.
static ComponentsToAnalyze
collectComponentsForAnalysis(const DataDependenceGraph &DDG,
                             const unsigned MaxComponentSize) {
  LLVM_DEBUG(dbgs() << "Collect components for analysis\n");

  const SimpleDDGNodes TPCConvertLinearNodes = getTPCConvertLinearNodes(DDG);

  SmallVector<GraphComponent, 8> Components;
  for (const SimpleDDGNode *N : TPCConvertLinearNodes) {
    Components.emplace_back(GraphComponentSearcher().findForInpNode(*N));
    Components.emplace_back(GraphComponentSearcher().findForOutNode(*N));
  }

  LLVM_DEBUG({
    dbgs() << "\n"
           << "Components before merge:\n";
    for (const GraphComponent &C : Components) {
      dbgs() << "\n"
             << "  Component of size: " << C.nodes().size() << "\n";
      if (C.nodes().size() < 100)
        dbgs() << C;
      else
        dbgs() << "...\n";
    }
  });

  SmallVector<GraphComponent, 8> MergedComponents =
      mergeComponentsWithCommonInputTargetNode(Components);

  LLVM_DEBUG(dbgs() << "\n"
                    << "Processing components merge:\n");

  ComponentsToAnalyze Result;

  for (GraphComponent &Component : MergedComponents) {

    LLVM_DEBUG(dbgs() << "\n  Found component of size: "
                      << Component.nodes().size() << "\n");

    if (Component.nodes().size() <= 1) {
      LLVM_DEBUG(dbgs() << "Skip too small component.\n");
      continue;
    }

    // Performance trick: skip transformation check if sub graph is big
    // enough.
    //   1. Check is based on time-consuming recursive algorithm.
    //   2. Not so much performance profit is expected in big graphs.
    //
    // Skip big graphs as tradeoff between compile time and performance gain.
    if (MaxComponentSize && MaxComponentSize < Component.nodes().size()) {
      LLVM_DEBUG(dbgs() << "Skip too large component.\n");
      continue;
    }

    // Analysis for nodes which are not single instruction simple node is not
    // supported:
    //  * non-single instruction simple node
    //  * pi-block nodes
    //  * root / exit nodes
    if (any_of(Component.nodes(), [](const DDGNode *N) {
          return !isSingleInstructionSimpleNode(*N);
        })) {
      LLVM_DEBUG(dbgs() << "Skip component with node of unsupported type:\n"
                        << Component);
      continue;
    }

    if (hasTensorStLdInOuts(Component)) {
      LLVM_DEBUG(dbgs() << "  Schedule for analysis component with patching "
                           "for tpc.convert.linear and tensor st-ld in-outs:\n"
                        << Component);
      Result.ComponentsWithLinearConvertAndStLdInOuts.emplace_back(
          std::move(Component));
    } else {
      LLVM_DEBUG(dbgs() << "  Schedule for analysis component with patching "
                           "for tpc.convert.linear in-outs:\n"
                        << Component);
      Result.ComponentsWithLinearConvertInOuts.emplace_back(
          std::move(Component));
    }
  }

  // Sanity check: components must not intersect by target nodes for transform
  // which behavior changes after transform. collectComponentsForAnalysis
  // function must guarantee this.
#ifndef NDEBUG
  SmallVector<GraphComponent, 8> AllResultComponents;
  copy(Result.ComponentsWithLinearConvertInOuts,
       std::back_inserter(AllResultComponents));
  copy(Result.ComponentsWithLinearConvertAndStLdInOuts,
       std::back_inserter(AllResultComponents));
  for (const auto &NodeAndCompIndices :
       makeTargetInputNodeToComponentIndicesMappings(AllResultComponents))
    assert(NodeAndCompIndices.second.size() == 1);
#endif

  return Result;
}

namespace {
struct InstructionsToTransform {
  IntrinsicsSet TPCConvertIntrinsics;
  IntrinsicToSwitchPatch TensorStIntrinsicToSwitchPatch;
  IntrinsicToSwitchPatch TensorLdIntrinsicToSwitchPatch;
};
} // namespace

static InstructionsToTransform getInstructionsToTransform(
    const DataDependenceGraph &DDG, const bool AllowTensorStorePack32To16,
    const bool AllowTensorStorePack32To8, const bool AllowTensorStorePack16To8,
    const bool AllowTensorLoadUnpackPatching) {
  const ComponentsToAnalyze ComponentsForAnalysis =
      collectComponentsForAnalysis(DDG, MaxComponentSizeToAnalyze);

  DDGNodes AllTPCConvertLinearNodesToTransform;
  NodeToSwitchPatch TensorStNodeToSwitchPatch;
  NodeToSwitchPatch TensorLdNodeToSwitchPatch;

  const auto HasScheduledInOutNodeWhichTransformAffectsResult =
      [&](const GraphComponent &Component) {
        return any_of(Component.nodes(), [&](const DDGNode *N) {
          // Only input and output nodes are interesting.
          if (!Component.isBoundaryNode(*N))
            return false;

          if (isTpcConvertLinearNode(*N))
            return isTransformAffectsConvertNodeResult(*N) &&
                   AllTPCConvertLinearNodesToTransform.contains(N);

          if (isTensorStNode(*N))
            return TensorStNodeToSwitchPatch.count(N) > 0;

          if (isTensorLdNode(*N))
            return TensorLdNodeToSwitchPatch.count(N) > 0;

          return false;
        });
      };

  const auto AnalyzeComponent = [&](const GraphComponent &Component) {
    LLVM_DEBUG(dbgs() << "\nCollect intrinsics to transform from component:\n"
                      << Component);

    // Skip transformation check if any affected node is already scheduled to
    // transform.
    //
    // According to the current heuristic transformation is applicable only for
    // the whole component at once, so if some node is already scheduled for
    // transform and transformation affects node behavior, re-scheduling it
    // twice is incorrect.
    //
    // Components can not share the same input or the same output nodes
    // according to collectComponentsForAnalysis algorithm post condition. But
    // component output node might be an input for another component. It is not
    // allowed to apply transforms for both components then.
    if (HasScheduledInOutNodeWhichTransformAffectsResult(Component)) {
      LLVM_DEBUG(
          dbgs() << "Skip component which intersects with nodes "
                    "scheduled for transformation with changed behavior\n");
      return;
    }

    const TransformPlan Plan = getTransformPlan(
        Component, AllowTensorStorePack32To16, AllowTensorStorePack32To8,
        AllowTensorStorePack16To8, AllowTensorLoadUnpackPatching);

    LLVM_DEBUG({
      if (!Plan.TPCConvertLinearNodes.empty()) {
        dbgs() << "Schedule nodes for transformation:\n";
        for (const DDGNode *Node : Plan.TPCConvertLinearNodes)
          dbgs() << "  " << *getFirstInstruction(*Node) << "\n";
        for (const auto NodeAndPatch : Plan.TensorStNodeToSwitchPatch)
          dbgs() << "  " << *getFirstInstruction(*NodeAndPatch.first) << "\n";
        for (const auto NodeAndPatch : Plan.TensorLdNodeToSwitchPatch)
          dbgs() << "  " << *getFirstInstruction(*NodeAndPatch.first) << "\n";
      } else {
        dbgs() << "Skip component. Can not find possible optimization\n";
      }
      dbgs() << "\n";
    });

    AllTPCConvertLinearNodesToTransform.insert(
        Plan.TPCConvertLinearNodes.begin(), Plan.TPCConvertLinearNodes.end());
    TensorStNodeToSwitchPatch.insert(Plan.TensorStNodeToSwitchPatch.begin(),
                                     Plan.TensorStNodeToSwitchPatch.end());
    TensorLdNodeToSwitchPatch.insert(Plan.TensorLdNodeToSwitchPatch.begin(),
                                     Plan.TensorLdNodeToSwitchPatch.end());
  };

  // Step 1:
  // Analyze components with tpc.convert.linear in-outs only.
  for (const GraphComponent &C :
       ComponentsForAnalysis.ComponentsWithLinearConvertInOuts)
    AnalyzeComponent(C);

  // Step 2:
  // Analyze sub graphs bounded with tpc.convert.linear and tensor store/load
  // intrinsics.
  for (const GraphComponent &C :
       ComponentsForAnalysis.ComponentsWithLinearConvertAndStLdInOuts)
    AnalyzeComponent(C);

  InstructionsToTransform RV;
  RV.TPCConvertIntrinsics =
      getInstructions(AllTPCConvertLinearNodesToTransform);
  RV.TensorStIntrinsicToSwitchPatch =
      getIntrinsicToSwitchPatch(TensorStNodeToSwitchPatch);
  RV.TensorLdIntrinsicToSwitchPatch =
      getIntrinsicToSwitchPatch(TensorLdNodeToSwitchPatch);
  return RV;
}

/// Clone llvm.tpc.convert.linear intrinsic to llvm.tpc.convert and insert it
/// before the cloned one.
static CallInst *insertTPCConvertIntClone(Instruction &TPCConvertLinearInt) {
  assert(isTpcConvertLinearIntrinsic(TPCConvertLinearInt));

  Module *M = TPCConvertLinearInt.getModule();
  LLVMContext &Context = M->getContext();

  IRBuilder<> Builder(&TPCConvertLinearInt);
  IntegerType *I1Ty = Type::getInt1Ty(Context);
  IntegerType *I8Ty = Type::getInt8Ty(Context);
  IntegerType *I32Ty = Type::getInt32Ty(Context);

  Type *SrcTy = TPCConvertLinearInt.getOperand(0)->getType();
  Type *DstTy = TPCConvertLinearInt.getType();

  FunctionType *FType =
      FunctionType::get(DstTy, {SrcTy, I8Ty, I32Ty, DstTy, I1Ty, I1Ty}, false);

  Function *Intrinsic = cast<Function>(
      M->getOrInsertFunction(getTPCIntrinsicName(Intrinsic::tpc_convert, FType),
                             FType)
          .getCallee());

  // 6 required + optional meta
  assert(TPCConvertLinearInt.getNumOperands() >= 6);
  const SmallVector<Value *, 8> Operands(
      TPCConvertLinearInt.op_begin(),
      std::next(TPCConvertLinearInt.op_begin(), 6));
  return Builder.CreateCall(Intrinsic, Operands);
}

/// Replace tpc.convert.linear intrinsics with tpc.convert.
static void
removeLinearityFromTPCConvert(const IntrinsicsSet &TPCConvertLinearIntrinsics) {
  for (Instruction *Int : TPCConvertLinearIntrinsics) {
    CallInst *IntrinsicCall = insertTPCConvertIntClone(*Int);
    Int->replaceAllUsesWith(IntrinsicCall);
    Int->eraseFromParent();
  }
}

static void applySwitchPatches(
    const IntrinsicToSwitchPatch &TensorStLdIntrinsicToSwitchPatch) {
  for (const auto &IntrinsicAndSwitchPatch : TensorStLdIntrinsicToSwitchPatch) {
    IntrinsicInst &I = *IntrinsicAndSwitchPatch.first;
    const std::uint64_t Patch = IntrinsicAndSwitchPatch.second;
    setTensorStLdSwitchValue(I, getTensorStLdSwitchValue(I) | Patch);
  }
}

static void applyTransforms(const InstructionsToTransform &Instructions) {
  removeLinearityFromTPCConvert(Instructions.TPCConvertIntrinsics);
  applySwitchPatches(Instructions.TensorStIntrinsicToSwitchPatch);
  applySwitchPatches(Instructions.TensorLdIntrinsicToSwitchPatch);
}

/// Check if function has instructions which might be optimized by the pass.
static bool containsInstructionsToOptimize(const Function &F) {
  for (const BasicBlock &BB : F) {
    for (const Instruction &I : BB) {
      if (isTpcConvertLinearIntrinsic(I))
        return true;
    }
  }
  return false;
}

static TPCEliminateRedundantLinearityPassConfig makeDefaultConfig() {
  TPCEliminateRedundantLinearityPassConfig Cfg;
  Cfg.HasFeatureTensorStorePack32To16 = false;
  Cfg.HasFeatureTensorStorePack32To8 = false;
  Cfg.HasFeatureTensorStorePack16To8 = false;
  Cfg.HasFeatureTensorLoadUnpack = false;
  return Cfg;
}

class EliminateRedundantLinearityLegacyPass : public FunctionPass {
public:
  static char ID;

  EliminateRedundantLinearityLegacyPass()
      : EliminateRedundantLinearityLegacyPass(makeDefaultConfig()) {}

  EliminateRedundantLinearityLegacyPass(
      const TPCEliminateRedundantLinearityPassConfig InCfg)
      : FunctionPass(ID), Cfg(InCfg) {
    initializeEliminateRedundantLinearityLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    if (!EnableEliminateRedundantLinearity)
      return false;

    // Avoid heavy analysis if nothing to optimize here.
    if (!containsInstructionsToOptimize(F))
      return false;

    LLVM_DEBUG(dbgs() << "Pass input:\n" << F << "\n");

    auto &AA = getAnalysis<AAResultsWrapperPass>().getAAResults();
    auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

    // Construct Data Dependence Graph(DDG)
    DependenceInfo DI(&F, &AA, &SE, &LI);
    DataDependenceGraph G(F, DI);

    const bool Changed = runOnDDG(G);

    LLVM_DEBUG(if (Changed) dbgs() << "\nPass output:\n" << F;);

    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AAResultsWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
  }

private:
  bool runOnDDG(const DataDependenceGraph &DDG) {
    const InstructionsToTransform Instructions = getInstructionsToTransform(
        DDG,
        EnableTensorStPack && EnableTensorStPack32To16 &&
            Cfg.HasFeatureTensorStorePack32To16,
        EnableTensorStPack && EnableTensorStPack32To8 &&
            Cfg.HasFeatureTensorStorePack32To8,
        EnableTensorStPack && EnableTensorStPack16To8 &&
            Cfg.HasFeatureTensorStorePack16To8,
        EnableTensorLdUnpack && Cfg.HasFeatureTensorLoadUnpack);

    applyTransforms(Instructions);

    NumOptimizedTPCConvertLinearIntrinsics +=
        Instructions.TPCConvertIntrinsics.size();
    NumPatchedTensorStoreIntrinsics +=
        Instructions.TensorStIntrinsicToSwitchPatch.size();
    NumPatchedTensorLoadIntrinsics +=
        Instructions.TensorLdIntrinsicToSwitchPatch.size();

    // If any tensor store / load instruction was patched, then there must exist
    // transformed tpc.convert.linear instruction. That's why checking against
    // empty Instructions.TPCConvertIntrinsics set is enough to ensure whether
    // IR is changed or not.
    assert((Instructions.TensorStIntrinsicToSwitchPatch.empty() &&
            Instructions.TensorLdIntrinsicToSwitchPatch.empty()) ||
           !Instructions.TPCConvertIntrinsics.empty());
    return !Instructions.TPCConvertIntrinsics.empty();
  }

  const TPCEliminateRedundantLinearityPassConfig Cfg;
};

char EliminateRedundantLinearityLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(EliminateRedundantLinearityLegacyPass, DEBUG_TYPE,
                      "Eliminate redundant linearity", false, false)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(EliminateRedundantLinearityLegacyPass, DEBUG_TYPE,
                    "Eliminate redundant linearity", false, false)

namespace llvm {

FunctionPass *createTPCEliminateRedundantLinearityPass(
    const TPCEliminateRedundantLinearityPassConfig Cfg) {
  return new EliminateRedundantLinearityLegacyPass(Cfg);
}

} // namespace llvm
