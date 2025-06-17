cmake_minimum_required(VERSION 3.13)
project(C2Pulse LANGUAGES C CXX)

# LLVM/Clang root (adjust if needed)
set(LLVM_ROOT "${CMAKE_SOURCE_DIR}/../external_tools/llvm-project/build")
set(CMAKE_PREFIX_PATH "${LLVM_ROOT}")

# Force use of shared libraries
set(LLVM_LINK_LLVM_DYLIB ON)
set(LLVM_BUILD_LLVM_DYLIB ON)
set(LLVM_DYLIB_COMPONENTS all)

# Enable compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable RTTI (important for Clang AST)
set(LLVM_ENABLE_RTTI ON CACHE BOOL "Enable RTTI" FORCE)

# Find packages
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Using LLVM from: ${LLVM_DIR}")
message(STATUS "Using Clang from: ${CLANG_DIR}")

# Set Clang as compiler
set(CMAKE_C_COMPILER "${LLVM_ROOT}/bin/clang")
set(CMAKE_CXX_COMPILER "${LLVM_ROOT}/bin/clang++")

# Compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")

# Include directories
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Let LLVM figure out which libs we need
llvm_map_components_to_libnames(LLVM_LIBS
  core
  support
  irreader
  frontendopenmp
  bitreader
  analysis
  transformutils
)

# Clang libs (link dynamically where possible)
set(CLANG_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangRewrite
  clangTooling
)

# Source files
set(SRC_FILES
  src/ExprLocationAnalyzer.cpp
  src/PulseGenerator.cpp
  src/PulseIR.cpp
  src/PulseCodeGen.cpp
  src/main.cpp
  src/Globals.cpp
)

add_executable(c2pulse ${SRC_FILES})

add_subdirectory(test)

# Link dependencies
target_link_libraries(c2pulse
  PRIVATE
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangRewrite
    clangTooling
    LLVMCore
    LLVMSupport
)

# Export compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Limit parallel link jobs since this is a Debug build
set_property(GLOBAL PROPERTY JOB_POOLS link_pool=2)
set_property(TARGET c2pulse PROPERTY JOB_POOL_LINK link_pool)

# Absolutely force RTTI during compilation of this target
target_compile_options(c2pulse PRIVATE -frtti)

target_link_libraries(c2pulse
  PRIVATE
    ${LLVM_LIBS}
    ${CLANG_LIBS}
)
