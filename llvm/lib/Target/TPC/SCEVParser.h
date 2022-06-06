#ifndef SCEVPARSER_CPP_H
#define SCEVPARSER_CPP_H

#include "TPCTargetMachine.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

using namespace llvm;
using namespace std;

class SCEVParser {
  public:

  SCEVParser(const SCEV *scev, ScalarEvolution *sel, Module *M = nullptr,
             LoopInfo *LInfo = nullptr)
          : p_SCEVP(scev), p_SEL(sel), m_SCE((*p_SEL), p_SEL->getDataLayout(), "TPC_LLVM"), p_M(M) {
      computeStride();
  };

  /*!
   *
   * @param input string to convert
   * @param Int8Ty "char" type
   * @return A string to push to section
   */
  vector<Constant *> createAString(std::string input, Type *Int8Ty);

  /*!
   *
   * @param s SCEV
   * @return
   */
  const SCEV *computeInit(const SCEV *s);

  /*!
   *
   * @param s SCEV
   * @return run on the scev and set to zero unknown nodes
   */
  const SCEV *treeRunner(const SCEV *s);

  /*!
   *
   * @param s SCEV
   * @return convert SCEV value to string presentation
   */
  std::string printH(const SCEV *s);

  /*!
   * This is the entry point of section creating. With that the SCEV section will be create.
   * @param index
   * @param LoadStore L-load S-store
   */
  void parseExpr(int index, char LoadStore);

  int get_step() { return m_StepInducation; }

  Instruction *getValueInducation();

  private:
  const SCEV *p_SCEVP;                                      /*!Contains a pointer to SCEV object*/
  int m_StepInducation = 0;                                   /*!Step size*/
  ScalarEvolution *p_SEL;                                     /*!Pointer to ScalarEvolution*/
  SCEVExpander m_SCE;                                         /*!SCEVExpander Class*/
  Module *p_M;                                                /*!Module pointer*/
  vector<Instruction *> m_InsVec;                             /*!instruction container*/
  vector<Instruction *> m_InsVec2;                            /*!instruction container*/
  std::vector<std::pair<const Loop *, Value *>> m_stepValue;  /*!pair of loop ptr and value for the step*/

  /*!
   * Enum describes the index space mac and min of A,B
   */
  typedef enum status {
    max,
    min
  } m_status;

  /*!
   * @brief findCoefficient find the constant value op the loop indeucation.
   * @param Expr SCEV to analysis
   * @param TargetLoop The loop to analysis
   * @return  The coefficient of the loop.
   * @example for(int i=0;i<3;i++) ==> {0,+,1}<%for_bb> --> SCEV = const{1}
   */
  const SCEV *findCoefficient(const SCEV *Expr, const Loop *TargetLoop);

  /*!
   * @param s SCEV to analysis
   * @param InsVecUpdate container to save the instructions (a value node in the SCEV)
   */
  void searchForValue(const SCEV *s, vector<Instruction *> &InsVecUpdate);

  /*!
   * @breaf compute the initialize value of the iterator
   * @param s SCEV to analysis
   * @param range {min|max} ==>
   * @retur the minimum or the maximum of the iterator.
   */
  const SCEV *computeInitIter(const SCEV *s, status range);

  /*!
  *
  * Find the stride element and set the stride member
  */
  void computeStride();

  /*!
   *
   * @param s SCEV pointer to analysis
   */
  void computerStepInstruction(const SCEV *s);
};

#endif // SCEVPARSER_CPP_H