#ifndef TOPT_LOCAL_OPT_LVN_H
#define TOPT_LOCAL_OPT_LVN_H

#include <llvm/IR/PassManager.h>

namespace llvm {
class Function;

namespace trainOpt {
/**
 *  SSCP - Sparse Simple Constant Propagation.
 */
class LVNPass : public PassInfoMixin<LVNPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
} // namespace trainOpt
} // namespace llvm

#endif // TOPT_LOCAL_OPT_LVN_H
