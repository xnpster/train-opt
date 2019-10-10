set(TOPT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TOPT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(TOPT_LIB_DIRS ${TOPT_SOURCE_DIR}/lib)
set(TOPT_INCLUDE_DIRS ${TOPT_SOURCE_DIR}/include)

include_directories(
  AFTER
  ${TOPT_INCLUDE_DIRS}
)

add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(test)
