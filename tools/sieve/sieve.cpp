#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalObject.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

using namespace llvm;

static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<IRBuilder<>> Builder;

/**
 * @todo Emit the sieve llvm code!
 * void sieve(int n, char *prime) {
 *   if (n > 0) {
 *     prime[0] = 0;
 *   }
 *
 *   if (n > 1) {
 *     prime[1] = 0;
 *   }
 *
 *   for (int i = 2; i < n; ++i) {
 *     prime[i] = 1;
 *   }
 *
 *   for (int i = 2; i < n; ++i) {
 *     if (prime[i]) {
 *       for (int j = i * i; j < n; j += i) {
 *         prime[j] = 0;
 *       }
 *     }
 *   }
 * }
 */
void emitModule() {
  auto *printfTy = FunctionType::get(Type::getInt32Ty(*TheContext),
                                     {PointerType::get(*TheContext, 0)}, true);
  auto *printf = Function::Create(printfTy, Function::ExternalLinkage, "printf",
                                  TheModule.get());

  auto *fmt = Builder->CreateGlobalString("%d\n", ".str", 0, TheModule.get());

  SmallVector<Type *> argTypes{Type::getInt32Ty(*TheContext),
                               PointerType::get(*TheContext, 0)};
  auto *fnTy =
      FunctionType::get(Type::getInt32Ty(*TheContext), argTypes, false);
  auto *fn = Function::Create(fnTy, Function::ExternalLinkage, "main",
                              TheModule.get());

  auto *entryBB = BasicBlock::Create(*TheContext, "entry", fn);
  Builder->SetInsertPoint(entryBB);
  Builder->CreateCall(
      printf, {fmt, ConstantInt::get(Type::getInt32Ty(*TheContext), 128)});
  Builder->CreateRet(ConstantInt::get(Type::getInt32Ty(*TheContext), 0));
}

int main() {
  TheContext = std::make_unique<LLVMContext>();
  TheModule = std::make_unique<Module>("main", *TheContext);

  // Create a new builder for the module.
  Builder = std::make_unique<IRBuilder<>>(*TheContext);

  emitModule();

  llvm::outs() << *TheModule;

  return 0;
}
