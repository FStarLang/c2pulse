cmake_minimum_required(VERSION 3.13)
project(C2Pulse LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

set(LLVM_LINK_COMPONENTS support)
set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(external/json)

add_executable(c2pulse
  src/ExprLocationAnalyzer.cpp
  src/PulseASTGenerator.cpp
  src/PulseIR.cpp
  src/PulseCodeGen.cpp
  src/main.cpp
  src/Globals.cpp
  src/MacroCommentTracker.cpp
  src/MacroFrontendAction.cpp
)

# Include directories for headers
target_include_directories(c2pulse
  PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(c2pulse
  PRIVATE
    ${LLVM_DEFINITIONS}
    ${CLANG_DEFINITIONS}
)

target_link_libraries(c2pulse PRIVATE
  nlohmann_json::nlohmann_json
  LLVM
  clang-cpp
)

# Set C++ standard and other properties
set_target_properties(c2pulse PROPERTIES
  CXX_STANDARD 20
  INTERPROCEDURAL_OPTIMIZATION OFF
)

# Export compile commands (e.g., for clangd or clang-tidy)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
