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

#include <map>
#include <vector>

using std::map;
using std::vector;
using std::pair;

using namespace llvm;

#define DEBUG_TYPE "lvn"

namespace llvm {

namespace trainOpt {
PreservedAnalyses LVNPass::run(Function &F, FunctionAnalysisManager &AM) {
  
  vector<pair<pair<unsigned, vector<Value*>>,Value*>> table;
  map<Value*, Value*> replace_val;


  for (BasicBlock &BB : F) {
    vector<pair<pair<unsigned, vector<Value*>>,Value*>> table;
    map<Value*, Value*> replace_val;

    for (Instruction &Inst : BB) {
      Inst.print(outs());
      outs() << "\n";
      // continue;

      auto opcode = Inst.getOpcode();

      vector<Value*> vals;
      auto num_args = Inst.getNumOperands();
      for(int i = 0; i < num_args; i++)
      {
        auto* arg = Inst.getOperand(i);
        auto it = replace_val.find(arg);
        if(it != replace_val.end())
        {
          Inst.setOperand(i, it->second);
          arg = it->second;
        }

        vals.push_back(arg);
      }

      auto encoded = std::make_pair(opcode, vals);
      bool found = false;
      for(const auto& e : table)
      {
        if(e.first == encoded)
        {
          replace_val[&Inst] = e.second;
          found = true;
          break;
        }
      }

      if(!found)
        table.push_back({encoded, &Inst});
    }

    outs() << "proc\n";

    for (auto& e : replace_val) {
      ((Instruction*)e.first)->eraseFromParent();
    }
  }
  return PreservedAnalyses::none();
}
} // namespace trainOpt
} // namespace llvm
