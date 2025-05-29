cmake_minimum_required(VERSION 3.13)
project(ANFAndPulseRewriter LANGUAGES C CXX)

# Find LLVM and Clang via the CONFIG mode
find_package(LLVM REQUIRED CONFIG) 
find_package(Clang REQUIRED CONFIG) 

# Add LLVM’s CMake modules (for llvm_map_components_to_libnames)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# Define our tool executable
add_executable(anf_pulse_tool ANFAndPulseRewriter.cpp)

# Include LLVM/Clang headers
target_include_directories(anf_pulse_tool PRIVATE
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

# Link against LLVM Support (InitLLVM) + Clang Tooling libraries
# and map in any additional LLVM components you need
llvm_map_components_to_libnames(LLVM_LIBS
  support                    # for InitLLVM
  # add other LLVM components here if needed
)

target_link_libraries(anf_pulse_tool
  PRIVATE
    ${LLVM_LIBS}
    clangTooling
    clangBasic
    clangASTMatchers
    clangFrontend
    clangRewrite
)

# Export compile commands for LibTooling’s CommonOptionsParser
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
