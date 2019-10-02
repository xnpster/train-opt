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

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/User.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;

#define DEBUG_TYPE "SimpleConstProp"

/**
 *  @todo: Try to use it.
 *  It is optional.
 *  STATISTIC(SimpleConstPropCount, "TODO!");
*/

namespace llvm {
  void initializeSSCPPass(PassRegistry &);
}

namespace trainOpt {

/**
 *  SSCP - Sparse Simple Constant Propagation.
 */
struct SSCP : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  SSCP() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    DataLayout DL = F.getParent()->getDataLayout();
    TargetLibraryInfo *TLI =
        &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
    SmallVector<Instruction *, 16> workList;

    errs() << F.getName() << '\n';
    for (Instruction &I : instructions(F)) {
      I.print(errs());
      errs() << "\nUsers:\n";
      for (User *U : I.users()) {
        errs() << "\t";
        U->print(errs());
        errs() << "\n";
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

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
  }
};
} // namespace trainOpt

char trainOpt::SSCP::ID = 0;

using namespace trainOpt;
INITIALIZE_PASS_BEGIN(SSCP, "SSCP",
                      "Lab1. Sparse Simple Constant Propagation", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(SSCP, "SSCP",
                    "Lab1. Sparse Simple Constant Propagation", false, false)
