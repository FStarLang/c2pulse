cmake_minimum_required(VERSION 3.13)
project(C2Pulse LANGUAGES C CXX)

# Hard code LLVM path
set(LLVM_ROOT "${CMAKE_SOURCE_DIR}/../external_tools/llvm-project/build")

set(CMAKE_PREFIX_PATH "${LLVM_ROOT}")

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# debug messages
message(STATUS "Using custom LLVM from: ${LLVM_DIR}")
message(STATUS "Using custom Clang from: ${CLANG_DIR}")

# Use Clang to compile C2Pulse
set(CMAKE_C_COMPILER "${LLVM_ROOT}/bin/clang")
set(CMAKE_CXX_COMPILER "${LLVM_ROOT}/bin/clang++")

# Compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(${LLVM_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")


# Header Files
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Source files
set(SRC_FILES
  src/ANFVisitor.cpp
  src/ANFConsumer.cpp
  src/ANFTransformer.cpp
  src/ExprLocationAnalyzer.cpp
  src/PulseGenerator.cpp
  src/PulseIR.cpp
  src/PulseCodeGen.cpp
  src/main.cpp
  src/Globals.cpp
)

# Executable Name
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

# Also ensure any LLVM headers behave consistently
set(LLVM_ENABLE_RTTI ON CACHE BOOL "Enable RTTI" FORCE)

