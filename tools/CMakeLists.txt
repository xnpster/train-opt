set(LLVM_LINK_COMPONENTS
  Analysis
  Support
  Core
  ScalarOpts
  IRReader
  TransformUtils
  DataFlowLabs
  )

add_llvm_tool(topt
  topt.cpp
  
  DEPENDS
  intrinsics_gen
  )

export_executable_symbols(topt)
