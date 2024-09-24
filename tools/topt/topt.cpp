#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/CodeGen/MachinePassManager.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalObject.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassNameParser.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/IRPrinter/IRPrintingPasses.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/InitializePasses.h>
#include <llvm/PassInfo.h>
#include <llvm/PassRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Triple.h>

#include "topt/DataFlow/SCCP.h"
#include "topt/DataFlow/SSCP.h"

#define DEBUG_TYPE "main"

using namespace llvm;

static cl::opt<std::string> PassPipeline(
    "passes",
    cl::desc(
        "A textual description of the pass pipeline. To have analysis passes "
        "available before a certain pass, add 'require<foo-analysis>'."));

static cl::alias PassPipeline2("p", cl::aliasopt(PassPipeline),
                               cl::desc("Alias for -passes"));

static cl::opt<std::string> InputFilename(cl::Positional,
                                          cl::desc("<input bitcode file>"),
                                          cl::init("-"),
                                          cl::value_desc("filename"));

static cl::opt<std::string> OutputFilename("o",
                                           cl::desc("Override output filename"),
                                           cl::value_desc("filename"));

static void registerPassBuilderCallbacks(PassBuilder &PB) {
  PB.registerPipelineParsingCallback(
      [](StringRef Name, FunctionPassManager &PM,
         ArrayRef<PassBuilder::PipelineElement>) {
        if (Name == "topt-sccp") {
          PM.addPass(trainOpt::SCCPPass{});
          return true;
        }
        if (Name == "topt-sscp") {
          PM.addPass(trainOpt::SSCPPass{});
          return true;
        }
        return false;
      });
}

int main(int argc, char **argv) {
  InitLLVM X(argc, argv);
  LLVMContext Context;

  cl::ParseCommandLineOptions(
      argc, argv, "llvm .ll -> .ll trainig optimizer and analysis printer\n");

  SMDiagnostic Err;
  if (OutputFilename.empty()) {
    OutputFilename = "-";
  }

  std::error_code EC;
  std::unique_ptr<ToolOutputFile> Out = std::make_unique<ToolOutputFile>(
      OutputFilename, EC, sys::fs::OF_TextWithCRLF);
  if (EC) {
    errs() << EC.message() << '\n';
    return 1;
  }

  // Load the input module...
  std::unique_ptr<Module> M = parseIRFile(InputFilename, Err, Context);
  if (!M) {
    Err.print(argv[0], dbgs());
    return 1;
  }

  FunctionPassManager FPM;
  ModulePassManager MPM;

  LoopAnalysisManager LAM;
  MachineFunctionAnalysisManager MFAM;
  FunctionAnalysisManager FAM;
  CGSCCAnalysisManager CGAM;
  ModuleAnalysisManager MAM;

  PassBuilder PB;
  registerPassBuilderCallbacks(PB);

  PB.registerFunctionAnalyses(FAM);
  PB.registerModuleAnalyses(MAM);
  PB.registerLoopAnalyses(LAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerMachineFunctionAnalyses(MFAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  if (Error Err = PB.parsePassPipeline(MPM, PassPipeline)) {
    errs() << "topt: " << toString(std::move(Err)) << "\n";
    return 1;
  }

  MPM.addPass(PrintModulePass(Out->os()));
  MPM.run(*M, MAM);

  Out->keep();
  LLVM_DEBUG(dbgs() << "Hello train-opt! Training Optimizer!\n");
  return 0;
}
