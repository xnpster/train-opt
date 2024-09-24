
add_llvm_library(LLVMConstProp
  SSCP.cpp
  SCCP.cpp
  SCCPSolver.cpp
  
  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Analysis
  Core
  InstCombine
  Support
)
