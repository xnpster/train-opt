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

  auto* sieveTy = FunctionType::get(Type::getVoidTy(*TheContext), {Type::getInt32Ty(*TheContext), PointerType::get(Type::getInt8Ty(*TheContext), 0)}, false);
  auto* sieveFn = Function::Create(sieveTy, Function::ExternalLinkage, "sieve", TheModule.get());

  auto *sieveFirstBB = BasicBlock::Create(*TheContext, "entry_sieve", sieveFn);
  Builder->SetInsertPoint(sieveFirstBB);
  // Builder->CreateCall( 
  //     printf, {fmt, ConstantInt::get(Type::getInt32Ty(*TheContext), 128)});
  // Builder->CreateRet(ConstantInt::get(Type::getInt32Ty(*TheContext), 0));

// if n > 0
  auto* cmp_res = Builder->CreateICmpSGT(sieveFn->getArg(0), ConstantInt::get(Type::getInt32Ty(*TheContext), 0));


  auto* n_gt_zero = BasicBlock::Create(*TheContext, "n_gt_zero", sieveFn);
  auto* else0 = BasicBlock::Create(*TheContext, "else0", sieveFn);

  Builder->CreateCondBr(cmp_res, n_gt_zero, else0);

  Builder->SetInsertPoint(n_gt_zero);
  auto* in_array = sieveFn->getArg(1);
  auto* loaded = Builder->CreateGEP(Type::getInt8Ty(*TheContext), in_array, {ConstantInt::get(Type::getInt32Ty(*TheContext), 0)});
  Builder->CreateStore(ConstantInt::get(Type::getInt8Ty(*TheContext), 0), loaded);
  Builder->CreateBr(else0);
  Builder->SetInsertPoint(else0);

//if n > 1
  auto* cmp_res2 = Builder->CreateICmpSGT(sieveFn->getArg(0), ConstantInt::get(Type::getInt32Ty(*TheContext), 1));
  auto* n_gt_one = BasicBlock::Create(*TheContext, "n_gt_one", sieveFn);
  auto* else1 = BasicBlock::Create(*TheContext, "else1", sieveFn);

  Builder->CreateCondBr(cmp_res2, n_gt_one, else1);

  Builder->SetInsertPoint(n_gt_one);
  
  loaded = Builder->CreateGEP(Type::getInt8Ty(*TheContext), in_array, {ConstantInt::get(Type::getInt32Ty(*TheContext), 1)});
  Builder->CreateStore(ConstantInt::get(Type::getInt8Ty(*TheContext), 0), loaded);
  Builder->CreateBr(else1);
  Builder->SetInsertPoint(else1);

// for1
  auto* i1 = Builder->CreateAlloca(Type::getInt32Ty(*TheContext), 0, ConstantInt::get(Type::getInt32Ty(*TheContext), 1));
  Builder->CreateStore(ConstantInt::get(Type::getInt32Ty(*TheContext), 2), i1);

  auto* for_1_cond = BasicBlock::Create(*TheContext, "for_1_cond", sieveFn);
  auto* for_1_body = BasicBlock::Create(*TheContext, "for_1_body", sieveFn);
  auto* for_1_end = BasicBlock::Create(*TheContext, "for_1_end", sieveFn);

  Builder->CreateBr(for_1_cond);
  Builder->SetInsertPoint(for_1_cond);
  auto* i1_val = Builder->CreateLoad(Type::getInt32Ty(*TheContext), i1);
  auto* cmp_res3 = Builder->CreateICmpSLT(i1_val, sieveFn->getArg(0));
  Builder->CreateCondBr(cmp_res3, for_1_body, for_1_end);

  Builder->SetInsertPoint(for_1_body);
  auto* ld2_addr = Builder->CreateGEP(Type::getInt8Ty(*TheContext), in_array, {i1_val});
  Builder->CreateStore(ConstantInt::get(Type::getInt8Ty(*TheContext), 1), ld2_addr);
  auto* i1_added = Builder->CreateAdd(i1_val, ConstantInt::get(Type::getInt32Ty(*TheContext), 1));
  Builder->CreateStore(i1_added, i1);
  Builder->CreateBr(for_1_cond);

  Builder->SetInsertPoint(for_1_end);
  Builder->CreateStore(ConstantInt::get(Type::getInt32Ty(*TheContext), 2), i1);

  auto* for_2_cond = BasicBlock::Create(*TheContext, "for_2_cond", sieveFn);
  auto* for_2_body = BasicBlock::Create(*TheContext, "for_2_body", sieveFn);
  auto* for_2_end = BasicBlock::Create(*TheContext, "for_2_end", sieveFn);

  Builder->CreateBr(for_2_cond);
  Builder->SetInsertPoint(for_2_cond);
  auto* i2_val = Builder->CreateLoad(Type::getInt32Ty(*TheContext), i1);
  auto* cmp_res4 = Builder->CreateICmpSLT(i2_val, sieveFn->getArg(0));
  Builder->CreateCondBr(cmp_res4, for_2_body, for_2_end);
  Builder->SetInsertPoint(for_2_body);

  auto* ld3_addr = Builder->CreateGEP(Type::getInt8Ty(*TheContext), in_array, {i2_val});
  auto* ld3_res = Builder->CreateLoad(Type::getInt8Ty(*TheContext), ld3_addr);
  auto* cmp_res5 = Builder->CreateICmpNE(ConstantInt::get(Type::getInt8Ty(*TheContext), 0), ld3_res);
  auto* if_2_body = BasicBlock::Create(*TheContext, "if_2_body", sieveFn);
  auto* if_2_end = BasicBlock::Create(*TheContext, "if_2_end", sieveFn);
  Builder->CreateCondBr(cmp_res5, if_2_body, if_2_end);

  Builder->SetInsertPoint(if_2_end);  
  auto* i2_added = Builder->CreateAdd(i2_val, ConstantInt::get(Type::getInt32Ty(*TheContext), 1));
  Builder->CreateStore(i2_added, i1);
  Builder->CreateBr(for_2_cond);

  Builder->SetInsertPoint(for_2_end);
  Builder->CreateRetVoid();

  Builder->SetInsertPoint(if_2_body);
  auto* j1 = Builder->CreateAlloca(Type::getInt32Ty(*TheContext), 0, ConstantInt::get(Type::getInt32Ty(*TheContext), 1));
  auto* square_i1 = Builder->CreateMul(i2_val, i2_val);
  Builder->CreateStore(square_i1, j1);

  auto* for_3_cond = BasicBlock::Create(*TheContext, "for_3_cond", sieveFn);
  auto* for_3_body = BasicBlock::Create(*TheContext, "for_3_body", sieveFn);

  Builder->CreateBr(for_3_cond);
  Builder->SetInsertPoint(for_3_cond);

  auto* j1_val = Builder->CreateLoad(Type::getInt32Ty(*TheContext), j1);
  auto* cmp_res6 = Builder->CreateICmpSLT(j1_val, sieveFn->getArg(0));
  Builder->CreateCondBr(cmp_res6, for_3_body, if_2_end);

  Builder->SetInsertPoint(for_3_body);
  auto* ld4_addr = Builder->CreateGEP(Type::getInt8Ty(*TheContext), in_array, {j1_val});
  auto* ld4_res = Builder->CreateStore(ConstantInt::get(Type::getInt8Ty(*TheContext), 0) , ld4_addr);

  auto* j1_added = Builder->CreateAdd(j1_val, i2_val);
  Builder->CreateStore(j1_added, j1);
  Builder->CreateBr(for_3_cond);
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
