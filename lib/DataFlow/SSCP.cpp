//===- SSCP.cpp - Pass that implements Sparse Simple Constant Propagation -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include <llvm/ADT/Statistic.h>
#include <llvm/Analysis/ConstantFolding.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/User.h>
#include <llvm/InitializePasses.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/Local.h>

#include "topt/DataFlow/SSCP.h"

#define DEBUG_TYPE "SimpleConstProp"

/**
 *  @todo: Try to use it.
 *  It is optional.
 *  STATISTIC(SimpleConstPropCount, "TODO!");
 */

namespace llvm {
namespace trainOpt {
static bool runSSCP(Function &F, const DataLayout &DL,
                    const TargetLibraryInfo *TI) {
  SmallVector<Instruction *, 16> workList;

  LLVM_DEBUG(dbgs() << F.getName() << '\n');
  for (Instruction &I : instructions(F)) {
    LLVM_DEBUG(dbgs() << I << "\nUsers:\n");
    for (User *U : I.users()) {
      LLVM_DEBUG(dbgs() << "\t" << *U << "\n");
    }
    /**
     *  @todo: Insert your code here!
     *  Delete debug print if it disturbs.
     *  Fill in the `workList`.
     */
  }

  while (!workList.empty()) {
    Instruction *I = workList.back();
    workList.pop_back();
    /**
     *  @todo: Insert your code here!
     *  Implement a propagation step.
     *  After every iteration try to reduce dead instructions.
     *  It is recommended to use the `ConstantFoldInstruction`,
     *  `isInstructionTriviallyDead` functions.
     *  To delete instruction from BasicBlock use the `eraseFromParent`
     *  method of the class `Instruction`.
     */
  }
  /**
   *  @todo: Insert your code here!
   *  Return true if the pass makes changes, otherwise return false.
   */
  return false;
}

PreservedAnalyses SSCPPass::run(Function &F, FunctionAnalysisManager &AM) {
  const DataLayout &DL = F.getDataLayout();
  TargetLibraryInfo &TLI = AM.getResult<TargetLibraryAnalysis>(F);

  if (!runSSCP(F, DL, &TLI))
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}
} // namespace trainOpt
} // namespace llvm
