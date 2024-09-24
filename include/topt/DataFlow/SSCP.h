#ifndef TOPT_DATAFLOW_SSCP_H
#define TOPT_DATAFLOW_SSCP_H

#include <llvm/IR/PassManager.h>

namespace llvm {
class Function;

namespace trainOpt {
/**
 *  SSCP - Sparse Simple Constant Propagation.
 */
class SSCPPass : public PassInfoMixin<SSCPPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
} // namespace trainOpt
} // namespace llvm

#endif // TOPT_DATAFLOW_SSCP_H
