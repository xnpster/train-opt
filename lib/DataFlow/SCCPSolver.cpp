#include <llvm/ADT/Statistic.h>
#include <llvm/Analysis/ConstantFolding.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Analysis/ValueLattice.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/User.h>
#include <llvm/InitializePasses.h>
#include <llvm/Pass.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/Local.h>

#include "topt/DataFlow/SCCPSolver.h"

#include <llvm/Analysis/InstructionSimplify.h>

#define DEBUG_TYPE "SCCPSolver"

namespace llvm {
namespace trainOpt {
void Solver::solve() {
  while (!BBWorkList.empty() || !InstWorkList.empty() ||
         !OverdefinedInstWorkList.empty()) {

    while (!OverdefinedInstWorkList.empty()) {
      auto *I = OverdefinedInstWorkList.pop_back_val();
      markUsersAsChanged(I);
    }

    while (!InstWorkList.empty()) {
      auto *I = InstWorkList.pop_back_val();
      if (I->getType()->isStructTy() || !getValueState(I).isOverdefined())
        markUsersAsChanged(I);
    }

    while (!BBWorkList.empty()) {
      auto *BB = BBWorkList.pop_back_val();
      
      for(auto& I : *BB)
      {
        dbgs() << "Visit " << I << "\n";
        visit(I);
      }
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

bool Solver::isBlockExecutable(BasicBlock *BB) {
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
  dbgs() << "Visit bin " << I << '\n';

  LatticeVal V1State = getValueState(I.getOperand(0));
  LatticeVal V2State = getValueState(I.getOperand(1));

  LatticeVal &IV = ValueState[&I];
  if (IV.isOverdefined()) 
    return;

  if (V1State.isConstant() && V2State.isConstant()) {
    Value *V1 = V1State.isConstant() ? V1State.getConstant() : I.getOperand(0);
    Value *V2 = V2State.isConstant() ? V2State.getConstant() : I.getOperand(1);

    Value *R = simplifyBinOp(I.getOpcode(), V1, V2, SimplifyQuery(DL, &I));
    auto *C = dyn_cast_or_null<Constant>(R);
    if (C) {
      markConstant(&I, C);
      return;
    }
    else
    {
      markOverdefined(&I);
      return;
    }
  }

  if (V1State.isOverdefined() || V2State.isOverdefined()) {
    markOverdefined(&I);
  }
}

void Solver::visitCmpInst(CmpInst &I) {
  dbgs() << "Visit cmp " << I << '\n';

  if (getValueState(&I).isOverdefined())
    return;
 
  Value *Op1 = I.getOperand(0);
  Value *Op2 = I.getOperand(1);
 
  auto V1State = getValueState(Op1);
  auto V2State = getValueState(Op2);
  Value *R = simplifyCmpInst(I.getPredicate(), 
    V1State.isConstant() ? V1State.getConstant() : Op1, 
    V2State.isConstant() ? V2State.getConstant() : Op2, SimplifyQuery(DL, &I));
  auto *C = dyn_cast_or_null<Constant>(R);
  if (C) {
    markConstant(&I, C);
    return;
  }
 
  // If operands are still unknown, wait for it to resolve.
  if ((V1State.isUnknown() || V2State.isUnknown()) && !getValueState(&I).isConstant())
    return;
 
  markOverdefined(&I);
}

void Solver::visitTerminator(Instruction &I) {
  dbgs() << "Visit term " << I << '\n';
  SmallVector<bool, 16> SuccFeasible;
  getFeasibleSuccessors(I, SuccFeasible);
 
  BasicBlock *BB = I.getParent();
 
  for (unsigned i = 0, e = SuccFeasible.size(); i != e; ++i)
    if (SuccFeasible[i])
      markEdgeExecutable(BB, I.getSuccessor(i));
}

void Solver::visitPHINode(PHINode &PN) {
  dbgs() << "Visit phi " << PN << '\n';
  if (PN.getType()->isStructTy())
  {
    markOverdefined(&PN);
    return;
  }

  if (getValueState(&PN).isOverdefined())
    return;

  if (PN.getNumIncomingValues() > 64)
  {
    markOverdefined(&PN);
    return;
  }
 
  unsigned NumActiveIncoming = 0;
 
  auto PhiState = getValueState(&PN);

  for (unsigned i = 0; i < PN.getNumIncomingValues(); i++) {
    if (!isEdgeFeasible(PN.getIncomingBlock(i), PN.getParent()))
      continue;

    LatticeVal &IV = getValueState(PN.getIncomingValue(i));
    if (IV.isUnknown()) {
      continue;
    }
    
    if(IV.isOverdefined())
    {
      markOverdefined(&PN);
      break;
    }

    if(getValueState(&PN).isConstant())
    {
      if(getValueState(&PN).getConstantInt()->getValue() != IV.getConstantInt()->getValue())
      {
        markOverdefined(&PN);
        break;
      }
      else
      {
        continue;
      }
    }
    else if(getValueState(&PN).isUnknown())
    {
      markConstant(&PN, IV.getConstant());
      continue;
    }
  }
}

void Solver::visitReturnInst(ReturnInst &I) {}

bool Solver::isEdgeFeasible(BasicBlock *From, BasicBlock *To) {
  if (KnownFeasibleEdges.count(Edge(From, To))) {
    return true;
  }
  return false;
}

void Solver::getFeasibleSuccessors(Instruction &I,
                                   SmallVector<bool, 16> &Succs) {
  Succs.resize(I.getNumSuccessors());
  if (auto *BI = dyn_cast<BranchInst>(&I)) {
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
    return LV; // Common case, already in the map.
  }

  if (auto *C = dyn_cast<Constant>(V)) {
    // Undef values remain unknown.
    if (!isa<UndefValue>(V)) {
      LV.markConstant(C);
    }
  }

  // All others are underdefined by default.
  return LV;
}
} // namespace trainOpt
} // namespace llvm
