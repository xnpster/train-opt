set(LLVM_LINK_COMPONENTS
  AllTargetsAsmParsers
  AllTargetsCodeGens
  AllTargetsInfos
  Analysis
  Support
  Core
  CodeGen
  ScalarOpts
  IRReader
  IRPrinter
  Target
  TargetParser
  TransformUtils
  Passes
  ConstProp
)

add_llvm_tool(topt
  topt.cpp
  
  DEPENDS
  intrinsics_gen
  )

export_executable_symbols(topt)
