set(LLVM_TOPT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LLVM_TOPT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(LLVM_TOPT_INCLUDE_DIRS
    ${LLVM_TOPT_SOURCE_DIR}/include
)

include_directories(
  AFTER
  ${LLVM_TOPT_INCLUDE_DIRS}
)

add_subdirectory(lib)
add_subdirectory(tools)
