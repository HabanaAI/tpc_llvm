//===- TPCCostModelEmitter.cpp --- Cost Model------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// author: Michael Zuckerman
//         mzuckerman@habana.ai
//===----------------------------------------------------------------------===//
//   TPC-COST MODEL creates a flow graph of the tpc kernel. This is done via the
//   following steps:
//
//   1) Build CFG.
//   2) Compute Graph cycle (Using DFS).
//   3) Create segmentation according to cycle.
//   4) Create a source sink graph from the segmented graph.
//   5) Compute cost bottom up.
//   6) Save the result to the object.
//
//   This analysis returns a formula in SCEVGlue language.
//===----------------------------------------------------------------------===//

#include "TPCCostModelEmitter.h"
#include "llvm/InitializePasses.h"

using namespace llvm;

namespace llvm {
void initializeTPCCostModelEmitterPass(PassRegistry &);
} // namespace llvm

namespace TPCCOST {

// By default cost model must be turned off, because pure compiler (invoked with
// -cc1 option) is used for running tests, which are not true kernels. The cost
// model is turned on in driver (llvm/tools/clang/lib/Driver/ToolChain/Clang.cpp,
// under comment "TPC Cost Model") and is activated when user runs tpc-clang.
static cl::opt<bool> dontAnalysis("dontAnalysis", cl::desc("Enable graph output"),
                                 cl::init(true), cl::ZeroOrMore, cl::Hidden);

static cl::opt<bool> debugTPC("debugTpc", cl::desc("Enable graph output"),
                              cl::init(false), cl::Hidden);

static cl::opt<bool> printGraphB("graphOut", cl::desc("Enable graph output"),
                                 cl::init(false), cl::Hidden);

static cl::opt<bool> TPCResult("TPCResult", cl::desc("Enable TPCResult"),
                               cl::init(false), cl::Hidden);

static cl::opt<bool>
    CostModelOffset("CostModelDetectArgs",
                    cl::desc("print function parameters as offset and not name"),
                    cl::init(true), cl::Hidden);

static bool isDominator(const SCEV *LHS, const SCEV *RHS, DominatorTree &DT) {
  const SCEVAddRecExpr *LH = dyn_cast<SCEVAddRecExpr>(LHS);
  const SCEVAddRecExpr *RH = dyn_cast<SCEVAddRecExpr>(RHS);
  if (LH && RH) {
    BasicBlock *LHB = LH->getLoop()->getHeader();
    BasicBlock *RHB = RH->getLoop()->getHeader();
    if (!DT.dominates(LHB, RHB)) {
      TPC_DEBUG(LHB->getName() + " is not dominate " + RHB->getName() +"\n");
      return false;
    }
  }
  return true;
}


bool LeafType::operator<(const LeafType &CC) const {
  return this->priority < CC.priority;
}

void LeafType::setPriority(nodeType val) {
  switch (val) {
  case nodeType::LOOP:
    priority = 0;
    break;
  case nodeType::SWITCH_BRANCH:
    priority = 1;
    break;
  case nodeType::NODE:
    priority = 2;
    break;
  case nodeType::EXIT:
    priority = 3;
    break;
  case nodeType::LATCH:
    priority = 3;
    break;
  };
}

void CostLeaf::sortChildren() {
  std::sort(children.begin(), children.end(), CostLeafCMP());
}

void CostLeaf::pushChild(CostLeaf *CL) {
  if (CL == nullptr)
    return;
  children.push_back(CL);
}

bool CostLeaf::operator<(const CostLeaf &CL) const { return type < CL.type; }

void CostLeaf::removeFromLoop() {
  bool run = true;
  while (run) {
    run = false;
    for (auto child = children.begin(); child != children.end(); child++) {
      if ((*child)->getType() == nodeType::LATCH) {
        auto val = std::find(children.begin(), children.end(), this);
        if (val != children.end()) {
          children.erase(child);
          run = true;
          break;
        }
      }
    }
  }
}

std::string CostLeaf::printHeadLeaf() {
  string retValue = "";
  retValue += to_string(MBB->getNumber()); // Node deceleration
  retValue += " [shape=record, color=";
  switch (type.getNodeType()) {
  case nodeType::LOOP:
    retValue += "crimson";
    break;
  case nodeType::EXIT:
    retValue += "blue1";
    break;
  case nodeType::LATCH:
    retValue += "forestgreen";
    break;
  case nodeType::SWITCH_BRANCH:
    retValue += "goldenrod";
    break;
  default:
    retValue += "black";
    break;
  }
  retValue +=
      ",label=\"<f0> " + to_string(MBB->getNumber()); // Number of the BB
  retValue += " |<f1> " + to_string(cycle);           // BasicBlock cycle
  retValue += " |<f2> ";
  string retValue2;
  raw_string_ostream stream(retValue2);
  SCEVValue->print(stream);
  retValue2 = stream.str();
  bool noChange = false;
  do {
    noChange = false;
    auto a = retValue2.find('<');
    auto b = retValue2.find('>');
    if (a < retValue2.size()) {
      retValue2.replace(a, b + 1 - a, "");
      noChange = true;
    }
    a = retValue2.find('{');
    b = retValue2.find('}');
    if (a < retValue2.size()) {
      retValue2.replace(a, 1, "");
      retValue2.replace(b - 1, 1, "");
      noChange = true;
    }
  } while (noChange);
  retValue = retValue + retValue2 + " |<f3> ";
  raw_string_ostream streamN(retValue);
  streamN << LD;
  retValue = streamN.str();
  retValue += "\"];\n";

  return retValue;
}

std::string CostLeaf::printCostLeaf() {
  string returnValue = "";
  for (auto &child : children) {
    returnValue += to_string(MBB->getNumber()) + "->" +
                   to_string(child->getMachinBB()->getNumber()) + ";\n";
  }
  return returnValue;
}

string CostLeaf::printChildren() {
  string valToReturn = "";
  for (auto node : children) {
    valToReturn += to_string(node->MBB->getNumber()) + "->";
  }
  valToReturn += "end\n";
  return valToReturn;
}

void CostLeaf::removeSelfCycle() {
  auto remove = find(children.begin(), children.end(), this);
  if (remove != children.end())
    children.erase(remove);
}
/// class  CostCircle
void LoopCost::pushNodeToLoop(CostLeaf *Node) {
  NodeCircle.push_back(Node);
  length++;
}

bool LoopCost::operator<(const LoopCost &CC) const {
  return this->length < CC.length;
}

/*! getLoopCostInScev return the circuit cost in SCEV .
 *   This function does it with pass over all nodes connect to the main
 *   node.
 */


const SCEV *LoopCost::getLoopCostInSCEV(DominatorTree &DT) {
  const SCEV *first = SE.getConstant(ConstantInt::get(
      Type::getInt32Ty(LoopHead->getBasicBlock()->getContext()), 0));
  if (NodeCircle.size() > 0) {
    for (auto Node = NodeCircle.begin(); Node != NodeCircle.end(); Node++) {
      if (!(*Node)->getTaken()) {
        // In the case of LF doesn't dominate all RH
        if(!isDominator((*Node)->getSCEVValue(), first, DT))
          return nullptr;
        first = SE.getAddExpr(first, (*Node)->getSCEVValue());
        const SCEV *zero = SE.getConstant(ConstantInt::get(
            Type::getInt32Ty(LoopHead->getBasicBlock()->getContext()), 0));
        (*Node)->setSCEVValue(zero);
        (*Node)->setTaken(true);
      }
    }
  }
  const SCEV *LoopCycle = SE.getConstant(ConstantInt::get(
      Type::getInt32Ty(LoopHead->getBasicBlock()->getContext()),
      this->LoopHead->getCycle()));
  const SCEV *LHS = SE.getAddExpr(LoopCycle, first);
  if (LHS->getType() > LoopHead->getSCEVValue()->getType()) {
    LoopHead->setSCEVValue(
        SE.getZeroExtendExpr(LoopHead->getSCEVValue(), LHS->getType()));
  }
  const SCEV *result = SE.getMulExpr(LHS, LoopHead->getSCEVValue());
  return result;
}

void LoopCost::connectCircuit() {
  if (NodeCircle.size() == 0)
    return;
  CostLeaf *RemoveChild = NodeCircle[0];
  CostLeaf *RealNext = NodeCircle.back();
  auto updateRemoveChild =
      std::find(LoopHead->getChildren()->begin(),
                LoopHead->getChildren()->end(), RemoveChild);
  *updateRemoveChild = RealNext;
  auto it = std::find(RealNext->getChildren()->begin(),
                      RealNext->getChildren()->end(), LoopHead);
  if (it != RealNext->getChildren()->end())
    RealNext->eraseFromChildren(it);
  RealNext->setCycle(0);
}
void printGraph(string file, CostTree &CF) {
  int FD;
  std::string Filename = llvm::createGraphFilename(file, FD);
  raw_fd_ostream OS(FD, /*shouldClose=*/true);
  OS << "digraph before"
     << "{\n";
  OS << "node [shape=recode];\n";
  OS << "struct1  [shape=record,label=\" < f0 > nameBB | <f1> CYCLE |<f2> "
        "COST| "
        "<f3> Loop pointer\"];";
  OS << CF.printTree();
  OS << "}\n";
  OS.close();
  GraphProgram::Name Program = GraphProgram::DOT;
  DisplayGraph(Filename, false, Program);
}

CostTree::CostTree(MachineFunction *MF, LoopInfo &LI, ScalarEvolution &SE,
                   MachineLoopInfo &MLI1, DominatorTree &DT)
          : LI(LI), SE(SE), MLI(MLI1), DT(DT)
  {
    TPC_DEBUG("Phase1: Begin build cost tree\n");
    TPC_DEBUG("-----------------------------\n");
    valid = buildTree(MF);
    if (!valid)
      return;
    TPC_DEBUG("Phase2: Computing graph circle inside cost tree\n");
    TPC_DEBUG("-----------------------------------------------\n");
    sortChildrenByPriority();
    computeCircle(Root);
    TPC_DEBUG("Cost tree is ready\n");
  }

/// class cost tree
void CostTree::consumeCircle() {
  TPC_DEBUG("Consume circle sorting\n");
  std::sort(Circle.begin(), Circle.end(), CostLoopCMP());
  if (debugTPC && printGraphB)
    printGraph("CFG-", *this);
  for (auto Node : Circle) {
    const SCEV *LoopSCEV = Node->getLoopCostInSCEV(DT);
    if (!LoopSCEV) {
      isValidModel = false;
      return;
    }
    Node->getLoopHead()->setSCEVValue(LoopSCEV);
    Node->connectCircuit();
    Node->getLoopHead()->removeSelfCycle();
    if (debugTPC && printGraphB)
      printGraph("CFG-", *this);
  }
  TPC_DEBUG("END Consume circle sorting\n");
}

void CostTree::initVisit() {
  for (auto &Node1 : VectorNode)
    Node1->setVisit(false);
}

const SCEV *CostTree::computeCostTree(CostLeaf *CL, DominatorTree &DT,
                                      string deepADD) {
  TPC_DEBUG(deepADD + "Working on node" +
            to_string(CL->getMachinBB()->getNumber()) + "\n");
  const SCEV *val = SE.getConstant(
      ConstantInt::get(Type::getInt32Ty(CL->getBasicBlock()->getContext()), 0));

  for (auto child : *CL->getChildren()) {
    TPC_DEBUG(deepADD + "-child" +
              to_string(child->getMachinBB()->getNumber()) + "\n");
    deepADD += "----";
    const SCEV *val2 = computeCostTree(child, DT,deepADD);
    if (!val2)
      return nullptr;
    deepADD = deepADD.substr(0, deepADD.size() - 4);
    if (!isDominator(val, val2, DT))
      return nullptr;
    val = SE.getAddExpr(val, val2);
    if (debugTPC) {
      string valueR = "";
      raw_string_ostream stream(valueR);
      val2->print(stream);
      TPC_DEBUG(deepADD + "-child-cost" +
                to_string(child->getMachinBB()->getNumber()) + ":" +
                stream.str() + "\n");
    }
  }

  if (CL->getVisit())
    return SE.getConstant(ConstantInt::get(
        Type::getInt32Ty(CL->getBasicBlock()->getContext()), 0));

  CL->setVisit(true);

  if (CL->getChildren()->size() == 0) {
    if (debugTPC) {
      string valueR = "";
      raw_string_ostream stream(valueR);
      CL->getSCEVValue()->print(stream);
      TPC_DEBUG(deepADD + "node-cost:" + stream.str() + "\n");
    }
    return CL->getSCEVValue();
  }
  if (!isDominator(val, CL->getSCEVValue(), DT))
    return nullptr;
  val = SE.getAddExpr(val, CL->getSCEVValue());

  if (debugTPC) {
    string valueR = "";
    raw_string_ostream stream(valueR);
    val->print(stream);
    TPC_DEBUG(deepADD + "node-cost:" + stream.str() + "\n");
  }

  return val;
}

void SwitchLeafData::searchAndInsert(const SwitchInst *I) {
  for (unsigned op = 1; op < I->getNumOperands(); op += 2) {
    if (BasicBlock *bb = dyn_cast<BasicBlock>(I->getOperand(op))) {
      std::vector<BasicBlock *>::iterator it =
          std::find(ifList.begin(), ifList.end(), bb);
      if (it == ifList.end()) {
        ifList.push_back(bb);
      }
    }
  }
}

void CostTree::nodeContainsSwitchBranch(const BasicBlock *BB) {
  for (auto switchBB = switchBranch.begin(), END = switchBranch.end();
       switchBB != END; switchBB++) {
    if ((*switchBB)->getBBHead() == BB) {
      return;
    }
  }
  for (auto I = BB->begin(), I_END = BB->end(); I != I_END; I++) {
    if (isa<SwitchInst>(I)) {
      const SwitchInst *SwitchInstPtr = dyn_cast<SwitchInst>(I);
      SwitchLeafData *newSwitch = new SwitchLeafData(SwitchInstPtr);
      newSwitch->searchAndInsert(SwitchInstPtr);
      switchBranch.push_back(newSwitch);
    }
  }
}
bool CostTree::isSwitchNode(const BasicBlock *BB) {
  for (auto treeSwitchBB = switchBranch.begin(), END = switchBranch.end();
       treeSwitchBB != END; treeSwitchBB++) {
    std::vector<BasicBlock *> vecIns = (*treeSwitchBB)->getIfList();
    if (std::find(vecIns.begin(), vecIns.end(), BB) != vecIns.end())
      return true;
  }
  return false;
}

CostLeaf *CostTree::createNode(MachineBasicBlock *VMMB) {
  CostLeaf *workingNode;
  const BasicBlock *BB = VMMB->getBasicBlock();
  if (!BB)
    return nullptr;
  bool notLoop = true;
  if (std::find(BBlist.begin(), BBlist.end(), BB) != BBlist.end())
    notLoop = false;
  BBlist.push_back(BB);
  nodeContainsSwitchBranch(BB);
  int cycle = getBBCycles(VMMB, false);
  workingNode = new CostLeaf(BB);
  workingNode->setMachineBB(VMMB);
  MachineLoop *loop = MLI.getLoopFor(VMMB);
  TPC_DEBUG("Working on Basic Block:" + to_string(VMMB->getNumber()) + "\n");
  if (isSwitchNode(BB)) {
    TPC_DEBUG("|---Node is a SwitchNode \n")
    workingNode->setType(nodeType::SWITCH_BRANCH);
    workingNode->setSCEVValue(
        SE.getConstant(Type::getInt32Ty(BB->getContext()), cycle));
  } else if (loop && LI.isLoopHeader(BB) && notLoop) {
    TPC_DEBUG("|---Node is a LoopNode \n")
    workingNode->setType(nodeType::LOOP);
    Loop *Lp = LI.getLoopFor(BB);
    LoopData *val = new LoopData(Lp, &SE, true);
    workingNode->setSCEVValue(val->getLoopSCEV());
    isValidModel &= val->getSCEVStatus();
    workingNode->setLoadData(val);
  } else {
    TPC_DEBUG("|---Node is a Node \n");
    workingNode->setType(nodeType::NODE);
    workingNode->setSCEVValue(
        SE.getConstant(Type::getInt32Ty(BB->getContext()), cycle));
  }
  TPC_DEBUG("|---Number of cycle in the node:" + to_string(cycle) + "\n");
  workingNode->setCycle(cycle);
  return workingNode;
}
string val = "";

bool CostTree::buildTree(MachineFunction *MF) {

  TPC_DEBUG("Init node list\n");
  // Create a list of the basic block
  for (auto &MBB : *MF) {
    if (!MBB.getBasicBlock())
      return false;
    VectorNode.push_back(createNode(&MBB));
  }

  if (!isValidModel)
    return false;

  TPC_DEBUG("Nodes list is ready number of nodes:" +
            to_string(VectorNode.size()) + "\n");
  TPC_DEBUG("\n");

  TPC_DEBUG("Set children list for each Node\n");
  for (auto &MBB : *MF) {
    if (!MBB.getBasicBlock())
      return false;
    TPC_DEBUG("|---" + to_string(MBB.getNumber()) + "\n");
    vector<CostLeaf *> val = *VectorNode[MBB.getNumber()]->getChildren();
    for (auto child : MBB.successors()) {
      if (!child->getBasicBlock())
        continue;
      TPC_DEBUG("|------Child:" + to_string(child->getNumber()) + "\n");
      if (std::find(val.begin(), val.end(), VectorNode[child->getNumber()]) ==
          val.end())
        VectorNode[MBB.getNumber()]->pushChild(VectorNode[child->getNumber()]);
    }
  }
  TPC_DEBUG("Finish set children list for each Node\n");
  TPC_DEBUG("Set EXIT and LATCH info for each LOOP\n");
  for (auto &Node : VectorNode) {
    if (Node->getType() == nodeType::LOOP) {
      TPC_DEBUG("|---LOOP: " + to_string(Node->getMachinBB()->getNumber()) +
                "\n");
      // TODO: Check what to do when more then one latches and exits
      Loop *Lp = LI.getLoopFor(Node->getMachinBB()->getBasicBlock());
      SmallVector<BasicBlock *, 8> Latches;
      SmallVector<BasicBlock *, 8> Exits;
      Lp->getLoopLatches(Latches);
      BasicBlock *Latch = Latches[0];
      Lp->getExitBlocks(Exits);
      BasicBlock *Exit = Exits[0];
      if (!Exit || !Latch)
        assert(0 && "Error");
      for (auto &Node1 : VectorNode) {
        if (Node1->getType() == nodeType::NODE ||
            Node1->getType() == nodeType::SWITCH_BRANCH) {
          if (Node1->getBasicBlock() == Exit) {
            Node1->setType(nodeType::EXIT);
            TPC_DEBUG("|------EXIT: " +
                      to_string(Node1->getMachinBB()->getNumber()) + "\n");
          }
          if (Node1->getBasicBlock() == Latch) {
            Node1->setType(nodeType::LATCH);
            TPC_DEBUG("|------LATCH: " +
                      to_string(Node1->getMachinBB()->getNumber()) + "\n");
          }
        }
      }
    }
  }

  Root = VectorNode.at(0);
  return true;
}

void CostTree::sortChildrenByPriority() {
  TPC_DEBUG("Begin Leaf Prioritize children\n");
  for (auto leaf : this->VectorNode) {
    TPC_DEBUG(leaf->printChildren());
    leaf->sortChildren();
    TPC_DEBUG(leaf->printChildren());
  }
  TPC_DEBUG("End leaf prioritize children\n");
}

void allPath::pushToAllPath(vector<CostLeaf *> in_route, int cost) {
    Path *p_Path = new Path;
    p_Path->cost = cost;
    p_Path->path = in_route;
    route.push_back(p_Path);
    if (cost > maxPathCost) {
      maxPathCost = cost;
      indexPath = route.size() - 1;
      setMaxPath(route.at(indexPath));
    }
}

void allPath::setMaxPath(Path *newPath) {
  CostLeaf *runner = begin;
  for (auto element = newPath->path.begin(), end = newPath->path.end();
       element != end; element++) {
    runner->setChildToVisit((*element));
    runner = *element;
  }
  runner->setChildToVisit(end);
}

status CostTree::computeAllPath(allPath* globalPath,
                                CostLeaf *current, vector<CostLeaf *> list,
                                int costCollect) {
  if (!globalPath)
    return status::LOOPSTATUS;
  list.push_back(current);
  for (auto leaf = current->getChildren()->begin(),
            end = current->getChildren()->end();
       leaf != end; leaf++) {
    if ((*leaf)->getType() == nodeType::LOOP)
      return status::LOOPSTATUS;
    if (*leaf != globalPath->getEnd()) {
      TPC_DEBUG("|-----Child " +
                to_string((*leaf)->getMachinBB()->getNumber()) + "\n");
      if (computeAllPath(globalPath, *leaf, list,
                         (*leaf)->getCycle() + costCollect) ==
          status::LOOPSTATUS)
        return status::LOOPSTATUS;
    } else {
      globalPath->pushToAllPath(list, costCollect);
    }
  }
  return status::FINISH;
}

/*! @breaf computeCircle pass over all the nodes in the tree and add
 *   for each node his circle. Graph circle contains all nodes lead from
 *   Loop node to himself again.
 */
void CostTree::computeCircle(CostLeaf *CF) {
  TPC_DEBUG("|---Enter " + to_string(CF->getMachinBB()->getNumber()) + "\n");
  for (auto child : *CF->getChildren()) {
    CF->setChildToVisit(child);
    if (child->getChildToVisit() && child->getType() == nodeType::LOOP) {
      TPC_DEBUG("|-----Child is a LOOP" +
                to_string(child->getMachinBB()->getNumber()) + "\n");
      vector<CostLeaf *> list;
      allPath globalPath(child, CF);
      computeAllPath(&globalPath, child, list);
      LoopCost *newCircle = new LoopCost(child, this->SE);
      CostLeaf *ptr = child->getChildToVisit();
      while (ptr != child) {
        TPC_DEBUG("|--------Circle" +
                  to_string(ptr->getMachinBB()->getNumber()) + "\n");
        newCircle->pushNodeToLoop(ptr);
        ptr = ptr->getChildToVisit();
      }
      Circle.push_back(newCircle);
      continue;
    }
    if (child->getChildToVisit())
      continue;
    computeCircle(child);
  }
  TPC_DEBUG("|---EXIT " + to_string(CF->getMachinBB()->getNumber()) + "\n");
}

string CostTree::printTree() {
  string val = "";
  for (auto &Node1 : VectorNode)
    val += Node1->printHeadLeaf();
  for (auto &Node1 : VectorNode)
    Node1->setVisit(false);
  val += printTreeVal(Root);
  return val;
}

string CostTree::printTreeVal(CostLeaf *CL) {
  TPC_DEBUG(CL->getMachinBB()->getNumber());
  string val = "";
  if (CL->getVisit() || CL->getChildren()->size() == 0)
    return val;
  CL->setVisit(true);
  for (auto child : *CL->getChildren()) {
    if (CL->getChildToVisit() == child) {
      val  += "edge[color = red]";
    } else {
      val += "edge[color = black]";
    }
    val += to_string(CL->getMachinBB()->getNumber()) + "->" +
           to_string(child->getMachinBB()->getNumber()) + ";\n" +
           printTreeVal(child);
  }
  return val;
}
} // namespace TPCCOST

using namespace TPCCOST;
class TPCCostModelEmitter : public MachineFunctionPass {

public:
  static char ID;
  TPCCostModelEmitter() : MachineFunctionPass(ID){};
  // void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnMachineFunction(MachineFunction &Fn) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<MachineLoopInfo>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  SCEVExpander *SCE = nullptr;
  /*!
   * @param scev contains the SCEV to visit
   * @param functionArgs contains function's arguments
   * @return print helper -> convert SCEV value to printable string
   */
  string printHelper(const SCEV *scev,
                     std::vector<const llvm::Argument *> functionArgs);
  /*!
   * @breaf visualization of the present graph.
   * @param file to write the result.
   * @param CF a cost tree to present.
   */
  void printGraph(string file, CostTree &CF);
  /*!
   * @param input string to convert to vector
   * @param Int8Ty llvm type
   * @return
   */
  vector<Constant *> createAString(std::string input, Type *Int8Ty);
  bool isValidCostModel = true;
};

char TPCCostModelEmitter::ID = 0;

INITIALIZE_PASS_BEGIN(TPCCostModelEmitter, "TPCCostModelEmitter",
                      "TPC Cost Model Emitter", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachineBranchProbabilityInfo)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_DEPENDENCY(MachineScheduler)
INITIALIZE_PASS_END(TPCCostModelEmitter, "TPCCostModelEmitter",
                    "TPC Cost Model Emitter", false, false)

namespace llvm {
FunctionPass *createTPCCostModelEmitter() { return new TPCCostModelEmitter(); }
} // namespace llvm

void TPCCostModelEmitter::printGraph(string file, CostTree &CF) {
  int FD;
  std::string Filename = llvm::createGraphFilename(file, FD);
  raw_fd_ostream OS(FD, /*shouldClose=*/true);
  OS << "digraph before"
     << "{\n";
  OS << "node [shape=recode];\n";
  OS << "struct1  [shape=record,label=\" < f0 > nameBB | <f1> CYCLE |<f2> "
        "COST| "
        "<f3> Loop pointer\"];";

  OS << CF.printTree();
  OS << "}\n";
  OS.close();
  GraphProgram::Name Program = GraphProgram::DOT;
  DisplayGraph(Filename, false, Program);
}

/*!
 *
 * @param scev SCEV value
 * @param functionArgs contains a function's arguments
 * @return the operator as a string
 */
static string returnOperator(const SCEV *scev,
                             std::vector<const llvm::Argument *> functionArgs) {
  switch (scev->getSCEVType()) {
  case llvm::SCEVTypes::scAddExpr:
    return "+";
  case llvm::SCEVTypes::scMulExpr:
    return "*";
  case llvm::SCEVTypes::scSMaxExpr:
    return " SMAX";
  case llvm::SCEVTypes::scUMaxExpr:
    return " UMAX";
  case llvm::SCEVTypes::scConstant:
    return std::to_string(
        ((const SCEVConstant *)scev)->getValue()->getSExtValue());
  case llvm::SCEVTypes::scUnknown: {
    string valR;
    raw_string_ostream stream(valR);
    if (CostModelOffset) {
      if (Argument *unknownVal =
              dyn_cast<Argument>(((const SCEVUnknown *)scev)->getValue())) {
        for (size_t i = 0; i < functionArgs.size(); i++) {
          if (functionArgs.at(i)->getName() == unknownVal->getName()) {
            stream << "%" + std::to_string(i);
            stream.str();
            return valR;
          }
        }
      }
    }
    scev->print(stream);
    stream.str();
    std::size_t locationDot = valR.find('.', 0);
    std::size_t TPCLoad = valR.find("TPCLoad", 0);
    if (TPCLoad != std::string::npos)
      return valR;
    if (locationDot != std::string::npos)
      valR = valR.substr(0, locationDot + 2);
    return valR;
  }
  case llvm::SCEVTypes::scZeroExtend:
  case llvm::SCEVTypes::scTruncate:
  case llvm::SCEVTypes::scSignExtend: {
    string valR;
    raw_string_ostream stream(valR);
    scev->print(stream);
    stream.str();
    return valR;
  }
  default:
    assert(false && "Unsupported Type");
    return "error";
  }
}

string TPCCostModelEmitter::printHelper(const SCEV *scev,
                                 std::vector<const llvm::Argument*> functionArgs) {
  string result;
  if (const SCEVAddRecExpr *AddRec = dyn_cast<SCEVAddRecExpr>(scev)) {
    if (!AddRec->getLoop()->getCanonicalInductionVariable()) {
      result = "1";
      isValidCostModel = false;
    } else {
      result =
          string(AddRec->getLoop()->getCanonicalInductionVariable()->getName());
    }
    result = "(" + result + string("*") +
             printHelper(AddRec->getOperand(1), functionArgs) + string("+") +
             printHelper(AddRec->getOperand(0), functionArgs) + ")";
    return result;
  }

  if (const SCEVCommutativeExpr *CommutativeExp =
               dyn_cast<SCEVCommutativeExpr>(scev)) {
    string operatorVal = returnOperator(CommutativeExp, functionArgs);
    string resultStr[10];
    unsigned i = 0;
    for (i = 0; i < CommutativeExp->getNumOperands(); i++) {
      resultStr[i] = printHelper(CommutativeExp->getOperand(i), functionArgs);
    }
    for (i = 0; i < CommutativeExp->getNumOperands() - 1; i++) {
      result += resultStr[i] + operatorVal;
    }
    result += resultStr[i];
    return "(" + result + ")";
  }

  if (const SCEVUDivExpr *divExp = dyn_cast<SCEVUDivExpr>(scev)) {
    string resultStr[10];
    unsigned i = 0;
    resultStr[0] = printHelper(divExp->getLHS(), functionArgs);
    resultStr[1] = printHelper(divExp->getRHS(), functionArgs);
    result += resultStr[i] + string("/") + resultStr[1];
    return "(" + result + ")";
  }

  return returnOperator(scev, functionArgs);
}

vector<Constant *> TPCCostModelEmitter::createAString(std::string input,
                                                      Type *Int8Ty) {
  vector<Constant *> Init;
  for (unsigned i = 0; i < input.size(); i++) {
    Init.push_back(ConstantInt::get(Int8Ty, input[i]));
  }
  return Init;
}

bool TPCCostModelEmitter::runOnMachineFunction(MachineFunction &Fn) {
  if (dontAnalysis) {
    TPC_DEBUG("Analysis doesn't work")
    return EXIT_SUCCESS;
  }
  auto &MLI = getAnalysis<MachineLoopInfo>();
  auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  SCE = new SCEVExpander(SE, SE.getDataLayout(), "expander");

  TPC_DEBUG("Init graph presentation:\n");
  CostTree graph(&Fn, LI, SE, MLI, DT);
  if (!graph.getValid())
    return false;
  if (printGraphB)
    printGraph("CFG-", graph);

  TPC_DEBUG("Graph segmenting:\n");
  graph.consumeCircle();
  if (!graph.isValid()) {
    TPC_DEBUG("not valid model");
    return false;
  }
  if (printGraphB)
    printGraph("cfg-cost", graph);

  TPC_DEBUG("Graph calculation:\n");
  const SCEV *init = graph.computeCostTree(graph.getRoot(),DT);
  if (!init) {
    TPC_DEBUG("not valid model");
    return false;
  }
  llvm::Module *M = (llvm::Module *)Fn.getFunction().getParent();
  std::vector<const Argument*> argList;
  for (auto &fArg : Fn.getFunction().args()) {
    argList.push_back(&fArg);
  }
  string val = printHelper(init, argList);
  if (!isValidCostModel) {
    TPC_DEBUG("not valid model");
    return false;
  }
  val = "SCEVBEGIN SCEVCOST:" + val + "#SCEVEND";
  TPC_DEBUG("print result:" + val + "\n");
  if (TPCResult)
    llvm::errs() << "print result: " + val + "\n";

  TPC_DEBUG("Writing result to object");
  StringRef Name = "SCEVCost";
  Type *Int8Ty = llvm::Type::getInt8Ty(Fn.getFunction().getContext());
  vector<Constant *> Init = createAString(val, Int8Ty);
  ArrayType *ATy = ArrayType::get(Int8Ty, Init.size());
  llvm::GlobalVariable *GV0 =
      new llvm::GlobalVariable(*M, ATy, false, GlobalValue::ExternalLinkage,
                               ConstantArray::get(ATy, Init), Name, nullptr);
  GV0->setSection(".SCEVCost");
  TPC_DEBUG("Adding new section: .SCEVCost");
  return EXIT_SUCCESS;
}
