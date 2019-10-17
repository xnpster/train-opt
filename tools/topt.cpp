#include "llvm/ADT/Triple.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LegacyPassNameParser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include <algorithm>
#include <memory>

using namespace llvm;

#define DEBUG_TYPE "main"

// The OptimizationList is automatically populated with registered Passes by the
// PassNameParser.
//
static cl::list<const PassInfo *, bool, PassNameParser>
PassList(cl::desc("Optimizations available:"));

static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input bitcode file>"),
  cl::init("-"), cl::value_desc("filename"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Override output filename"),
  cl::value_desc("filename"));

/**
 *  @todo: Implement TrainOpt pipeline on the new pass manager!
 */


namespace llvm {
  void initializeSSCPPass(PassRegistry &);
  void initializeSCCPPass(PassRegistry &);
} // namespace llvm

int main(int argc, char **argv) {
  InitLLVM X(argc, argv);
  LLVMContext Context;

  // Initialize passes
  PassRegistry &Registry = *PassRegistry::getPassRegistry();
  initializeSSCPPass(Registry);
  initializeSCCPPass(Registry);

  cl::ParseCommandLineOptions(argc, argv,
    "llvm .ll -> .ll trainig optimizer and analysis printer\n");

  SMDiagnostic Err;
  if (OutputFilename.empty()) {
    OutputFilename = "-";
  }

  std::error_code EC;
  std::unique_ptr<ToolOutputFile> Out =
    std::make_unique<ToolOutputFile>(OutputFilename, EC, sys::fs::OF_None);

  // Load the input module...
  std::unique_ptr<Module> M = parseIRFile(InputFilename, Err, Context, true);
  if (!M) {
    Err.print(argv[0], dbgs());
    return 1;
  }
  Triple ModuleTriple(M->getTargetTriple());
  legacy::PassManager PM;
  for (unsigned i = 0; i < PassList.size(); ++i) {
    const PassInfo *PassInf = PassList[i];
    Pass *P = nullptr;
    if (PassInf->getNormalCtor())
      P = PassInf->getNormalCtor()();
    else
      errs() << argv[0] << ": cannot create pass: "
             << PassInf->getPassName() << "\n";
    if (P) {
      PM.add(P);
    }
  }
  PM.add(createPrintModulePass(Out->os(), "", true));
  PM.run(*M);
  Out->keep();
  LLVM_DEBUG(dbgs() << "Hello train-opt! Training Optimizer!\n");
  return 0;
}
