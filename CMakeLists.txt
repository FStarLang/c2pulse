cmake_minimum_required(VERSION 3.13)
project(C2Pulse LANGUAGES C CXX)

set(LLVM_LINK_COMPONENTS support)
set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(external/json)

# Define which LLVM components we need
llvm_map_components_to_libnames(LLVM_LIBS
  support
)

# Gather source files
set(SRC_FILES
  src/ExprLocationAnalyzer.cpp
  src/PulseASTGenerator.cpp
  src/PulseIR.cpp
  src/PulseCodeGen.cpp
  src/main.cpp
  src/Globals.cpp
  src/MacroCommentTracker.cpp
  src/MacroFrontendAction.cpp
)


add_clang_tool(c2pulse
  ${SRC_FILES}
  DEPENDS
    intrinsics_gen
)

# Include directories for headers
target_include_directories(c2pulse
  PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link against LLVM and Clang libraries
target_link_libraries(c2pulse
  PRIVATE
    ${LLVM_LIBS}
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangRewrite
    clangTooling
    nlohmann_json::nlohmann_json
)

# Set C++ standard and other properties
set_target_properties(c2pulse PROPERTIES
  CXX_STANDARD 20
  INTERPROCEDURAL_OPTIMIZATION OFF
)

# Export compile commands (e.g., for clangd or clang-tidy)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
