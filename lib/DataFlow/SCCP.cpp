//===- SCCP.cpp - Pass that implements ------------------------------------===//
//              Sparse Conditional Constant Propagation 
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// @todo Description
//
//===----------------------------------------------------------------------===//

#include "topt/DataFlow/SCCP.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueLattice.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/User.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;

#define DEBUG_TYPE "SparseCondConstProp"

/**
 * @todo: Try to use it.
 * It is optional.
 * STATISTIC(sccpCount, "TODO!");
 */

namespace trainOpt {

static bool tryToReplaceWithConstant(Solver &Solver, Value *V) {
  assert(!V->getType()->isStructTy() && "Structure type is not supported!");
  const LatticeVal &val = Solver.getLatticeValueFor(V);
  if (val.isOverdefined()) {
    return false;
  }
  Constant *Const = val.isConstant() ?
    val.getConstant() : UndefValue::get(V->getType());

  assert(!isa<CallInst>(V) && "CallInst is not supported!");

  V->replaceAllUsesWith(Const);
  return true;
}

/**
 *  Public methods!
 */

void Solver::solve() {
  while (!BBWorkList.empty() || !InstWorkList.empty() ||
         !OverdefinedInstWorkList.empty()) {

    while (!OverdefinedInstWorkList.empty()) {
      auto *I = OverdefinedInstWorkList.pop_back_val();
      /**
       *  @todo: Insert your code here!
       */
    }

    while (!InstWorkList.empty()) {
      auto *I = InstWorkList.pop_back_val();
      /**
       *  @todo: Insert your code here!
       */
    }

    while (!BBWorkList.empty()) {
      auto *BB = BBWorkList.pop_back_val();
      /**
       *  @todo: Insert your code here!
       */
    }
  }
}

bool Solver::markBlockExecutable(BasicBlock *BB) {
  if (!BBExecutable.insert(BB).second) {
    return false;
  }
  BBWorkList.push_back(BB); // Add the block to the worklist!
  return true;
}

void Solver::markOverdefined(Value *V) {
  if (auto *STy = dyn_cast<StructType>(V->getType())) {
    assert(false && "StructType is unsupported!");
  } else {
    markOverdefined(ValueState[V], V);
  }
}

bool Solver::isBlockExecutable(BasicBlock *BB)
{
  if (BBExecutable.count(BB)) {
    return true;
  }
  return false;
}

const LatticeVal &Solver::getLatticeValueFor(Value *V) const {
  const auto I = ValueState.find(V);
  if (I == ValueState.end()) {
    V->dump();
  }
  assert(I != ValueState.end() && "V is not found in ValueState");
  return I->second;
}

/**
 *  Private methods!
 */

void Solver::visitBinaryOperator(Instruction &I) {
  LatticeVal V1State = getValueState(I.getOperand(0));
  LatticeVal V2State = getValueState(I.getOperand(1));

  LatticeVal &IV = ValueState[&I];
  if (IV.isOverdefined()) {
    /**
     *  @todo: Insert your code here!
     */
  }

  if (V1State.isConstant() && V2State.isConstant()) {
    /**
     *  @todo: Insert your code here!
     */
  }
  
  if (!V1State.isOverdefined() && !V2State.isOverdefined()) {
    /**
     *  @todo: Insert your code here!
     */
  }
  /**
   *  @todo: Insert your code here!
   */
}

void Solver::visitCmpInst(CmpInst &I) {
  /**
   *  @todo: Insert your code here!
   */
}

void Solver::visitTerminator(Instruction &I) {
  /**
   *  @todo: Insert your code here!
   */
}

void Solver::visitPHINode(PHINode &PN) {
  /**
   *  @todo: Insert your code here!
   */
  for (unsigned i = 0; i < PN.getNumIncomingValues(); i++) {
    LatticeVal &IV = getValueState(PN.getIncomingValue(i));
    if (IV.isUnknown()) {
      continue;
    }
    /**
     *  @todo: Insert your code here!
     */
  }
}

void Solver::visitReturnInst(ReturnInst &I) {
}

bool Solver::isEdgeFeasible(BasicBlock *From, BasicBlock *To) {
  if (KnownFeasibleEdges.count(Edge(From, To))) {
    return true;
  }
  return false;
}

void Solver::getFeasibleSuccessors(Instruction &I, SmallVector<bool, 16> & Succs) {
  Succs.resize(I.getNumSuccessors());
  if (auto * BI = dyn_cast<BranchInst>(&I)) {
    if (BI->isUnconditional()) {
      Succs[0] = true;
      return;
    }

    LatticeVal BCValue = getValueState(BI->getCondition());
    ConstantInt *CI = BCValue.getConstantInt();
    if (!CI) {
      if (!BCValue.isUnknown()) {
        Succs[0] = Succs[1] = true;
      }
      return;
    }
    Succs[CI->isZero()] = true;
    return;
  }
  assert(false && "Unsupported Terminator!");
}

bool Solver::markEdgeExecutable(BasicBlock *Source, BasicBlock *Dest) {
  if (!KnownFeasibleEdges.insert(Edge(Source, Dest)).second) {
    return false;
  }
  if (!markBlockExecutable(Dest)) {
    for (PHINode &PN : Dest->phis()) {
      visitPHINode(PN);
    }
  }
  return true;
}

void Solver::pushToWorkList(LatticeVal &IV, Value *V) {
  if (IV.isOverdefined()) {
    OverdefinedInstWorkList.push_back(V);
    return;
  }
  InstWorkList.push_back(V);
}

bool Solver::markConstant(LatticeVal &IV, Value *V, Constant *C) {
  if (!IV.markConstant(C)) {
    return false;
  }
  pushToWorkList(IV, V);
  return true;
}

bool Solver::markConstant(Value *V, Constant *C) {
  return markConstant(ValueState[V], V, C);
}

bool Solver::markOverdefined(LatticeVal &IV, Value *V) {
  if (!IV.markOverdefined()) {
    return false;
  }
  // Only instructions get into the work list
  pushToWorkList(IV, V);
  return true;
}

void Solver::markUsersAsChanged(Value *I) {
  for (User *U : I->users()) {
    if (auto *UI = dyn_cast<Instruction>(U)) {
      if (BBExecutable.count(UI->getParent())) {
        visit(*UI);
      }
    }
  }
}

LatticeVal &Solver::getValueState(Value *V) {
  assert(!V->getType()->isStructTy() && "StructType is unsupported");

  std::pair<DenseMap<Value *, LatticeVal>::iterator, bool> I =
    ValueState.insert(std::make_pair(V, LatticeVal()));
  LatticeVal &LV = I.first->second;

  if (!I.second) {
    return LV;  // Common case, already in the map.
  }

  if (auto * C = dyn_cast<Constant>(V)) {
    // Undef values remain unknown.
    if (!isa<UndefValue>(V)) {
      LV.markConstant(C);
    }
  }

  // All others are underdefined by default.
  return LV;
}

/**
 *  SCCP - Sparse Conditional Constant Propagation pass.
 */

struct SCCP : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  SCCP() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
  }
};

bool SCCP::runOnFunction(Function &F) {
  DataLayout DL = F.getParent()->getDataLayout();
  TargetLibraryInfo *TLI =
    &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  Solver Solver(DL, TLI);

  for (Argument &AI : F.args()) {
    Solver.markOverdefined(&AI);
  }
  Solver.markBlockExecutable(&F.front());

  Solver.solve();

  for (auto &BB : F) {
    /**
     *  @todo: Insert your code here!
     *  1. We need to check whether the BasicBlock is executable.
     *     If the BasicBlock is not executable, we need
     *     to remove its instructions.
     *
     *  2. If the BasicBlock is executable, we need to check whether
     *     it is possible to replace every instruction from
     *     the BasicBlock with constant. If it is possible, replace it.
     *     It is recommended to use the `tryToReplaceWithConstant`,
     *     `isInstructionTriviallyDead` methods.
     *     To delete Instruction from the BasicBlock use the
     *     @code eraseFromParent @endcode method of the class
     *     @code Instruction @endcode.
     */
  }
  /**
   *  @todo: Insert your code here!
   *  Return true, if the pass makes changes, otherwise return false.
   */
  return false;
}
} // namespace trainOpt

char trainOpt::SCCP::ID = 0;

using namespace trainOpt;
INITIALIZE_PASS_BEGIN(SCCP, "SCCP",
                      "Lab2. Sparse Conditional Constant Propagation", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(SCCP, "SCCP",
                    "Lab2. Sparse Conditional Constant Propagation", false,
                    false)
