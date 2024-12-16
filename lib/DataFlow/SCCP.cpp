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

#include <llvm/ADT/STLExtras.h>

#include "topt/DataFlow/SCCP.h"
#include "topt/DataFlow/SCCPSolver.h"

using namespace llvm;

#define DEBUG_TYPE "SparseCondConstProp"

/**
 * @todo: Try to use it.
 * It is optional.
 * STATISTIC(sccpCount, "TODO!");
 */

namespace llvm::trainOpt {
static bool tryToReplaceWithConstant(Solver &Solver, Value *V) {
  assert(!V->getType()->isStructTy() && "Structure type is not supported!");
  const LatticeVal &val = Solver.getLatticeValueFor(V);
  if (val.isOverdefined()) {
    return false;
  }
  Constant *Const =
      val.isConstant() ? val.getConstant() : UndefValue::get(V->getType());

  assert(!isa<CallInst>(V) && "CallInst is not supported!");

  V->replaceAllUsesWith(Const);
  return true;
}

static bool runSCCP(Function &F, const DataLayout &DL,
                    const TargetLibraryInfo *TLI) {
  Solver Solver(DL, TLI);

  for (Argument &AI : F.args()) {
    Solver.markOverdefined(&AI);
  }
  Solver.markBlockExecutable(&F.front());

  Solver.solve();
 
  // If we decided that there are basic blocks that are dead in this function,
  // delete their contents now.  Note that we cannot actually delete the blocks,
  // as we cannot modify the CFG of the function.
 
  SmallPtrSet<Value *, 32> InsertedValues;
  SmallVector<Instruction *, 8> InsToErase;

  for (auto &BB : F) {

    if (!Solver.isBlockExecutable(&BB)) {
      for (Instruction &Inst : BB)
        InsToErase.push_back(&Inst);
    }
    else
    {
      for (Instruction &Inst : BB) {
        if (Inst.getType()->isVoidTy())
          continue;
        if (tryToReplaceWithConstant(Solver, &Inst)) {
          // if (isInstructionTriviallyDead(&Inst), TLI)
          //   InsToErase.push_back(&Inst);
        }
      }
    }
  }

  for(auto* I : InsToErase)
    I->eraseFromParent();

  return InsToErase.size();
}

PreservedAnalyses SCCPPass::run(Function &F, FunctionAnalysisManager &AM) {
  const DataLayout &DL = F.getFunction().getParent()->getDataLayout();
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);

  if (!runSCCP(F, DL, &TLI))
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}
} // namespace llvm::trainOpt
