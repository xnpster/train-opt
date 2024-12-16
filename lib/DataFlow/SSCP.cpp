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
#include <llvm/Analysis/InstructionSimplify.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/User.h>
#include <llvm/InitializePasses.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/Local.h>

#include <set>
#include <map>

using std::set;
using std::map;

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

  set<Value*> overdef;
  map<Value*, Value*> consts;

  LLVM_DEBUG(dbgs() << F.getName() << '\n');
  for (Instruction &I : instructions(F)) {
    // dbgs() << I << "\nUsers:\n";
    

    if(BinaryOperator::classof(&I))
    {
      Value *R = simplifyBinOp(I.getOpcode(), I.getOperand(0), I.getOperand(1), SimplifyQuery(DL, &I));
      if(R)
      {
        consts[&I] = R;
        workList.push_back(&I);
      }
    }
    else if (CmpInst::classof(&I))
    {
      Value *R = simplifyCmpInst(((CmpInst&)I).getPredicate(), I.getOperand(0), I.getOperand(1), SimplifyQuery(DL, &I));
      if(R)
      {
        consts[&I] = R;
        workList.push_back(&I);
      }
    }
    else
    {
      overdef.insert(&I);
      workList.push_back(&I);
    }
    /**
     *  @todo: Insert your code here!
     *  Delete debug print if it disturbs.
     *  Fill in the `workList`.
     */
  }

  bool changed = false;

  while (!workList.empty()) {
    Instruction *Inst = workList.back();
    workList.pop_back();

    dbgs() << *Inst << " Users:\n";

    for (User *U : Inst->users()) {
      Instruction* I = cast<Instruction>(U);

      dbgs() << "+++  " << *I << "\n";

      if(consts.find(I) == consts.end())
      {

        dbgs() << "Process " << "\n";
        auto num_args = I->getNumOperands();
        bool replaced = false;
        for(int i = 0; i < num_args; i++)
        {
          auto* arg = I->getOperand(i);
          dbgs() << "arg: " << *arg << "\n";
          auto it = consts.find(arg);
          if(it != consts.end())
          {
            dbgs() << "Replace " << *arg << " to " << *it->second << "\n";
            I->setOperand(i, it->second);
            replaced = true;
          }
        }

        if(replaced && overdef.find(I) == overdef.end())
        {
          if(BinaryOperator::classof(I))
          {
            Value *R = simplifyBinOp(I->getOpcode(), I->getOperand(0), I->getOperand(1), SimplifyQuery(DL, I));
            if(R)
            {
              consts[I] = R;
              workList.push_back(I);
            }
          }
          else if (CmpInst::classof(I))
          {
            Value *R = simplifyCmpInst(((CmpInst*)I)->getPredicate(), I->getOperand(0), I->getOperand(1), SimplifyQuery(DL, I));
            if(R)
            {
              consts[I] = R;
              workList.push_back(I);
            }
          }
        }

        changed |= replaced;
        
      }
    }
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
  return changed;
}

PreservedAnalyses SSCPPass::run(Function &F, FunctionAnalysisManager &AM) {
  const DataLayout &DL = F.getFunction().getParent()->getDataLayout();
  TargetLibraryInfo &TLI = AM.getResult<TargetLibraryAnalysis>(F);

  if (!runSSCP(F, DL, &TLI))
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}
} // namespace trainOpt
} // namespace llvm
