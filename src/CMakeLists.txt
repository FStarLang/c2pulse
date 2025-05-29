cmake_minimum_required(VERSION 3.13)
project(ANFAndPulseRewriter LANGUAGES C CXX)

# Find LLVM and Clang via the CONFIG mode
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Found Clang in: ${Clang_DIR}")

# Add LLVM’s CMake modules (for llvm_map_components_to_libnames)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# We build this as a plugin shared library, not an executable
add_library(anf_pulse_plugin SHARED ANFAndPulseRewriter.cpp)

# Expose LLVM and Clang headers to our target
target_include_directories(anf_pulse_plugin
  PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

# Define which LLVM components we need
llvm_map_components_to_libnames(LLVM_LIBS
  support       # for InitLLVM
)

# Link against LLVM support + Clang libraries
target_link_libraries(anf_pulse_plugin
  PRIVATE
    ${LLVM_LIBS}
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangRewrite
    clangTooling
)

# Make sure we export compile flags for tools like clang-tidy
set_target_properties(anf_pulse_plugin PROPERTIES
  CXX_STANDARD 17
  INTERPROCEDURAL_OPTIMIZATION OFF
)

# Export compile commands (for LibTooling’s CommonOptionsParser, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
