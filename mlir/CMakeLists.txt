find_package(MLIR REQUIRED CONFIG)

message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/bin)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/lib)
set(MLIR_BINARY_DIR ${CMAKE_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(TableGen)
include(AddLLVM)
include(AddMLIR)
include(HandleLLVMOptions)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/mlir/include)
include_directories(${PROJECT_BINARY_DIR}/mlir/include)

link_directories(${LLVM_BUILD_LIBRARY_DIR})
add_definitions(${LLVM_DEFINITIONS})

# Q Dialect
add_subdirectory(include/Q/IR)
add_subdirectory(lib/Q/IR)

# QZap Dialect 
add_subdirectory(include/QZap/IR)
add_subdirectory(lib/QZap/IR)

# QPin Dialect 
add_subdirectory(include/QPin/IR)
add_subdirectory(lib/QPin/IR)

# Conversions
add_subdirectory(include/Conversion/q-to-qzap)
add_subdirectory(lib/Conversion/q-to-qzap)

add_subdirectory(include/Conversion/qzap-to-qpin)
add_subdirectory(lib/Conversion/qzap-to-qpin)

add_subdirectory(include/Conversion/qpin-to-llvm)
add_subdirectory(lib/Conversion/qpin-to-llvm)

# Executables
add_subdirectory(tools/q-opt)