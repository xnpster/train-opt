add_llvm_library(LLVMLocalOpt
  LVN.cpp

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Analysis
  Core
  InstCombine
  Support
)
