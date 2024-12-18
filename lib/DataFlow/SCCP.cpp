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
  SmallVector<BasicBlock *, 8> BlocksToErase;

  for (auto &BB : F) {

    if (!Solver.isBlockExecutable(&BB)) {
      BlocksToErase.push_back(&BB);
      for (Instruction &Inst : BB)
      {
        Solver.tryToReplaceWithConstant(&Inst);
        InsToErase.push_back(&Inst);
      }
    }
    else
    {
      for (Instruction &Inst : BB) {
        if (Inst.getType()->isVoidTy())
          continue;
        if (Solver.tryToReplaceWithConstant(&Inst)) {
          // if (isInstructionTriviallyDead(&Inst), TLI)
            InsToErase.push_back(&Inst);
        }
      }
    }
  }

  // for(auto* BB : BlocksToErase)
  //   BB->eraseFromParent();

  for(auto* I : InsToErase)
  {
    // I->eraseFromParent();
    dbgs() << "Remove:" << *I << "\n";
    outs() << "Remove" << '\n';
    if(I->users().empty() && !isa<ReturnInst>(I))
      I->removeFromParent();
  }

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
