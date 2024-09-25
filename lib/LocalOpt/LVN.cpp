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

#include "topt/LocalOpt/LVN.h"

using namespace llvm;

#define DEBUG_TYPE "lvn"

namespace llvm {

namespace trainOpt {
PreservedAnalyses LVNPass::run(Function &F, FunctionAnalysisManager &AM) {
  for (BasicBlock &BB : F) {
    for (Instruction &Inst : BB)
      Inst.print(outs());
    /**
     * @todo Insert your code here:
     *
     */
  }
  return PreservedAnalyses::none();
}
} // namespace trainOpt
} // namespace llvm
