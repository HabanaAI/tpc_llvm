#ifndef LLVM_TPCCOSTMODELEMITTER_H
#define LLVM_TPCCOSTMODELEMITTER_H
//===- TPCCostModelEmitter.cpp --- Cost Model------------------------------===//
//
//
//                     The LLVM Compiler Infrastructure:
//
//              2019 - This pass is a property of Habana labs
//
//Author: Michael Zuckerman
//===----------------------------------------------------------------------===//
//   TPC-COST MODEL creates a flow graph of the tpc kernel. This is done via the
//   following steps:
//
//   1) Build CFG (call flow graph).
//   2) Compute Graph cycle (Using DFS).
//   3) Create segmentation according to cycle.
//   4) Create a source sink graph from the segmented graph.
//   5) Compute cost bottom up.
//   6) Save the result to the object.
//
//   This analysis returns a formula in SCEVGlue language.
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TPCMCTargetDesc.h"
#include "TPCIndexSpace.h"
#include "TPCInstrInfo.h"
#include "TPCMachineScheduler.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/GraphWriter.h"

using namespace llvm;
#undef DEBUG_TYPE
#define DEBUG_TYPE "tpccostmodel"

static inline bool ignoreInstr(MachineInstr *MI) {
  if (MI->isDebugValue())
    return true;
  if (MI->getOpcode() == TargetOpcode::IMPLICIT_DEF)
    return true;
  return false;
}

static inline bool isJmpInstr(const MachineInstr *MI) {
  if (MI->isBundle()) {
    const MachineBasicBlock *MBB1 = MI->getParent();
    MachineBasicBlock::const_instr_iterator MII = MI->getIterator();
    for (++MII; MII != MBB1->instr_end() && MII->isInsideBundle(); ++MII) {
      const MachineInstr &BMI = *MII;
      if (isJmpInstr(&BMI))
        return true;
    }
    return false;
  }

  return MI->getOpcode() == TPC::JMPR || MI->getOpcode() == TPC::JMPR_u;
}

static inline int getBBCycles(MachineBasicBlock *MBB, bool ignore_loop_instr) {
  int Cycle = 0;
  for (MachineBasicBlock::iterator J = MBB->end(), PE = MBB->begin();
       J != PE;) {
    --J;
    MachineInstr &DefMI = *J;
    ;
    if (ignoreInstr(&DefMI)) {
      continue;
    }
    if (DefMI.getOpcode() != TPC::LOOPEND) {
      Cycle++;
    }
    if (ignore_loop_instr && TPCII::isLoopInst(DefMI.getDesc()) &&
        (DefMI.getOpcode() != TPC::LOOPEND)) {
      Cycle--;
    }
  }
  return Cycle;
}

namespace TPCCOST {

#define TPC_DEBUG(x)                                                           \
  if (debugTPC) {                                                              \
    llvm::errs() << x;                                                         \
  } else {                                                                     \
    LLVM_DEBUG(dbgs() << x);                                                   \
  }

/*!
 * @class SwitchLeafData present the llvm's switch branch.
 * switch is a presentation of the if elseif else block.
 */
class SwitchLeafData {
private:
  const BasicBlock *BB = nullptr;   /*< Basic block contains the switch*/
  std::vector<BasicBlock *> ifList; /*< A list of pointers to basic blocks*/

public:
  SwitchLeafData(const SwitchInst *I) : BB(I->getParent()){};
  /*!
   * searchAndinsert collate the basic blocks pointer of the if else expiration.
   * @param I contatins the SwitchInst
   */
  void searchAndInsert(const SwitchInst *I);
  /*!
   * @return a list of basic blocks for the if list.
   */
  std::vector<BasicBlock *> getIfList() { return ifList; }
  /*!
   * @return The head of the basic block
   */
  const BasicBlock *getBBHead() { return BB; }
};

typedef enum nodeType { LOOP, EXIT, LATCH, SWITCH_BRANCH, NODE } nodeType;
/*! @class LeafType contains the four types of the node
 * Each node is machine basic block (MBB).
 * LOOP -> Is a node contains induction and stride.
 * EXIT -> Is a node controll all exit for a loop
 * LATCH -> Is a node returns the IP to the loop.
 * NODE -> Is general node
 */

class LeafType {
private:
  unsigned priority = 3; /*!< 3 is the lowest priority and 0 is the highest.*/
  nodeType NodeValue =
      nodeType::NODE; /*!< NODE is a general type node that was not define yet*/
  /*!
   * \breaf setPriority set the value of node unsigned priority.
   * Where priority can be set to one the following number
   * 0 - The highest.
   * 1 - Middle.
   * 2 - Lowest.
   * \param val gets the node type.
   */
  void setPriority(nodeType val);

public:
  LeafType(){};
  LeafType(nodeType val) : NodeValue(val) { setPriority(val); }
  /*!
   * \param CC
   * \return if this.priority is less then cc.priority
   */
  bool operator<(const LeafType &CC) const;
  /*!
   * \breaf getNodeType returns the type of the node and can be one of
   * \return nodeType enum |LOOP, EXIT, LATCH, NODE|
   */
  nodeType getNodeType() { return NodeValue; }
  /*!
   * \param val nodeType
   */
  void setNodeType(nodeType val) {
    NodeValue = val;
    setPriority(val);
  }
};

/*!
 *
 * \class costLeaf: cost leaf is the basic element of the graph.
 * Each cost leaf contains:
 * 1) Cost in cycles number.
 * 2) SCEV formola.
 * 3) children list.
 * 4) Direction indicates who is the next element to be consider as part if the
 *    cycle.
 * at first the costLeaf creates as it doesn't know nothing and in the continue
 * more information is adding discover by the cost tress.
 */
class CostLeaf {

public:
  CostLeaf() = default;

  CostLeaf(const BasicBlock *BBValue) : BB(BBValue){};

  void setCycle(int cycleValue) { cycle = cycleValue; }

  void setSCEVValue(const SCEV *value) { SCEVValue = value; }

  void setTaken(bool set) { taken = set; }

  bool getTaken() { return taken; }

  void setType(nodeType value) { type.setNodeType(value); }

  void setMachineBB(MachineBasicBlock *value) { MBB = value; }

  void setVisit(bool val) { visit = val; }

  void setLoadData(LoopData *val) { LD = val; }

  void setChildToVisit(CostLeaf *val) { ChildToVisit = val; }

  const SCEV *getSCEVValue() { return SCEVValue; }

  int getCycle() { return cycle; }

  const MachineBasicBlock *getMachinBB() { return MBB; }

  nodeType getType() { return type.getNodeType(); }

  CostLeaf *getChildToVisit() { return ChildToVisit; }

  LoopData *getLoadData() { return LD; }

  bool getVisit() { return visit; }

  const BasicBlock *getBasicBlock() { return BB; }

  vector<CostLeaf *> *getChildren() { return &children; }
  /*!
   * Compare to cost leaf prioritize.
   * \param CL
   * \return this.type < CL.type
   */
  bool operator<(const CostLeaf &CL) const;

  /*!
   * Remove specifics element from children list.
   * \param elementToRemove
   */
  void eraseFromChildren(const vector<CostLeaf *>::iterator elementToRemove) {
    children.erase(elementToRemove);
  }
  /*!
   * The function push costLeaf to a vector of children
   * \param CL pointer to CostLeaf
   */
  void pushChild(CostLeaf *CL);
  /*! /breaf sortChildren sorts the object's children by their prioritize.
   *   This step is importenet since we want discover some node before other.
   */
  void sortChildren();
  /*!
   * In the case the node himself is part of the graph cycle remove him.
   */
  void removeSelfCycle();
  /*!
   * Remove latch from the loop
   */
  void removeFromLoop();
  /*!
   * Helper function that create a header for the dot graph presentation.
   * @return string with the node header.
   */
  string printHeadLeaf();
  /*!
   * Helper function to print one node
   * \return string of the node in the convenation of the node->child
   */
  string printCostLeaf();
  /*!
   * helper function to print children list
   * \return connective list of children
   */
  string printChildren();

private:
  const SCEV *SCEVValue = nullptr; /*!< Contains the SCEV value of the node*/
  const MachineBasicBlock *MBB =
      nullptr; /*!< Contains the Machine Basic Block pointer*/
  const BasicBlock *BB = nullptr; /*!< Contains the Basic Block pointer*/
  bool visit = false;             /*!< Was sign in the current iteration*/
  bool taken = false;             /*!< Was taken by the compute*/
  LoopData *LD = nullptr; /*!< pointer to loop data (IndexSpace mapping)*/
  CostLeaf *ChildToVisit =
      nullptr;   /*!< contains the corrent direction to work when DFS run*/
  int cycle = 0; /*!< Number of VLIW instruction*/
  vector<CostLeaf *> children; /*!< Children list of the node*/
  LeafType type;               /*!< Type of leaf (LOOP|EXIT|LATCH|NODE)*/
};
/*!
 * CostLead compare another CostLeaf element.
 */
struct CostLeafCMP {
  CostLeafCMP() {}
  bool operator()(const CostLeaf *s1, const CostLeaf *s2) const {
    bool val = *s1 < *s2;
    return val;
  }
};

/*!
 * /class LoopCost is a class contains a connective list (as a vector).
 * This list is a circle list(NodeCircle) where the begin is the first node of
 * the circle after the head(LoopHead) and the last variable in most time is the
 * latch. The latch returns the loop to the head(LoopHead)
 */
class LoopCost {
private:
  int length = 0;               /*!< equal to the number of node in the circle*/
  CostLeaf *LoopHead = nullptr; /*!< The first node in the circle (Loop)*/
  vector<CostLeaf *> NodeCircle; /*< the members of the circle*/
  ScalarEvolution &SE;           /*< LLVM Scalar evolution*/

public:
  /*!
   * Constructor of the object
   * \param Head - The head of the list
   * \param SE - LLVM's Scalar Evolution
   */
  LoopCost(CostLeaf *Head, ScalarEvolution &SE) : LoopHead(Head), SE(SE) {}

  vector<CostLeaf *> getNodeCircle() { return NodeCircle; };
  CostLeaf *getLoopHead() { return this->LoopHead; }
  const SCEV *getLoopCostInSCEV(DominatorTree &DT);
  int getLength() { return length; }

  void setLength(int val) { length = val; }

  void pushNodeToLoop(CostLeaf *Node);
  void connectCircuit();

  bool operator<(const LoopCost &CC) const;
};

struct CostLoopCMP {
  CostLoopCMP() {}
  bool operator()(LoopCost *s1, LoopCost *s2) const {
    return s1->getLength() < s2->getLength();
  }
};

enum status { FINISH, LOOPSTATUS };

/*!
 * type present Path in the graph.
 */
typedef struct Path {
  vector<CostLeaf *> path; /*< A list of costLeaf*/
  int cost;                /*< Cost of the path*/
} Path;

/*!
 * @class allPath presents the all passable path between begin node and end
 * node.
 *
 */
class allPath {
private:
  vector<Path *> route; /*< A list of paths*/
  int maxPathCost = 0;  /*< Max path cost over all paths*/
  int indexPath = 0;    /*< The index number of the most expansive path*/
  CostLeaf *begin;      /*< The begin of the list*/
  CostLeaf *end;        /*< The end of the list. Together with the begina
                            and end. Presenting chunk of graph*/

public:
  allPath(CostLeaf *begin, CostLeaf *end) : begin(begin), end(end){};
  void pushToAllPath(vector<CostLeaf *> in_route, int cost);
  void setMaxPath(Path *newPath);
  CostLeaf *getEnd() { return end; }
  CostLeaf *getbegin() { return begin; }
};

/*!
 * /class CostTree: Contains the leaf of the graph.
 *
 */
class CostTree {
public:
  CostTree(MachineFunction *MF, LoopInfo &LI, ScalarEvolution &SE,
           MachineLoopInfo &MLI1, DominatorTree &DT);
  bool getValid() { return valid; }
  CostLeaf *getRoot() { return Root; }
  void setValid(bool val) { valid = val; }
  void setRoot(CostLeaf *val) { Root = val; }

  void consumeCircle();
  void initVisit();
  bool isValid() { return isValidModel; }
  const SCEV *computeCostTree(CostLeaf *CL,
                              DominatorTree &DT, string deepADD = "|");

  string printTree();
  string printTreeVal(CostLeaf *CL = NULL);

private:
  vector<const BasicBlock *> BBlist;
  CostLeaf *Root = nullptr;              /*!<Contains the root of the tree */
  LoopInfo &LI;                          /*!< LLVM Loop info */
  ScalarEvolution &SE;                   /*!< LLVM Scalar Evolution*/
  MachineLoopInfo &MLI;                  /*!< LLVM Machine Loop info*/
  DominatorTree &DT;                     /*!< DominatorTree info*/
  vector<CostLeaf *> VectorNode;         /*!< Vector of cost leaf */
  vector<LoopCost *> Circle;             /*!< Vector of circles */
  vector<SwitchLeafData *> switchBranch; /*!< list of branch BB*/
  bool valid = false;                    /*!< is valid cost tree*/
  bool isValidModel = true;              /*!< is the model is valid*/
  /*!
   * @param MF is a pointer to machine function
   * @return The function returns if it successes create a cost tree.
   */
  bool buildTree(MachineFunction *MF);
  /*!
   * @brief CreateNode classify the type of the node and fill the field
   * according to the type
   * @param VMMB corrent machine block
   * @return return a object costLeaf.
   */
  CostLeaf *createNode(MachineBasicBlock *MBB);
  /*!
   *
   * \param CF - compute the cost from point CL.
   */
  void computeCircle(CostLeaf *CF);

  void nodeContainsSwitchBranch(const BasicBlock *BB);

  bool isSwitchNode(const BasicBlock *BB);

  void sortChildrenByPriority();

  status computeAllPath(allPath *enter, CostLeaf *content,
                        vector<CostLeaf *> list, int costPath = 0);
};
} // namespace TPCCOST

#endif // LLVM_TPCCOSTMODELEMITTER_H
