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

# Define which LLVM components we need
llvm_map_components_to_libnames(LLVM_LIBS
  support
)

# Gather source files
set(SRC_FILES
  src/ANFAndPulseRewriter.cpp
  src/PulseEmitter.cpp
)

# Create shared plugin library
add_library(anf_pulse_plugin SHARED ${SRC_FILES})

# Include directories for headers
target_include_directories(anf_pulse_plugin
  PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link against LLVM and Clang libraries
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

# Set C++ standard and other properties
set_target_properties(anf_pulse_plugin PROPERTIES
  CXX_STANDARD 17
  INTERPROCEDURAL_OPTIMIZATION OFF
)

# Export compile commands (e.g., for clangd or clang-tidy)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
