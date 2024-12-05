#ifndef TOPT_DATAFLOW_SCCP_H
#define TOPT_DATAFLOW_SCCP_H

#include <llvm/IR/PassManager.h>
#include <llvm/IR/Function.h>

namespace llvm {
// class Function;

namespace trainOpt {
/**
 *  SCCP - Sparse Conditional Constant Propagation pass.
 */
class SCCPPass : public PassInfoMixin<SCCPPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
} // namespace trainOpt
} // namespace llvm

#endif // TOPT_DATAFLOW_SCCP_H
