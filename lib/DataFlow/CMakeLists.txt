add_llvm_library(LLVMDataFlowLabs
  SSCP.cpp
  SCCP.cpp
  
  DEPENDS
  intrinsics_gen
  )

target_link_libraries(LLVMDataFlowLabs PUBLIC
  LLVMAnalysis
  LLVMSupport
  LLVMCore
  LLVMScalarOpts
  LLVMTransformUtils
)