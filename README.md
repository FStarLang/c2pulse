# C2Pulse: Clang-based C to Pulse transpiler

C2Pulse is a Clang-based source-to-source transformation plugin that rewrites C functions into equivalent Pulse programs. It uses macros to allow the user to add **Pulse specifications**. For instance, `ENSURES(s)`, `REQUIRES(s)` etc. For a list of macros we support please check `CodeShield/test-transpiler/c/pulse_macros.h`.

## Project Structure
```
c2pulse/
├── CMakeLists.txt                      # Build configuration for the transpiler
├── README.md
├── build.sh                            # Build c2pulse and create a symlink in llvm-project
├── run.sh                              # Run the transpiler C2Pulse and checks the output with Fstar
├── test_runner.sh
├── scripts/ 
│   ├── run_c2pulse.sh                  # Runs only the transpiler and generates the Pulse files from the input C     
│   ├── run_fstar.sh                    # Runs Fstart on top of the generated files to check them   
│   ├── run_test.sh                     # Run the test-suite
│   ├── build_snapdb.sh                 # Build the snapshot database
│   └── check_file.sh                   # Helper for testing against snapshop
├── test/                               # C2Pulse test suite
│   ├── Cfg.fst.config.json              
│   ├── lit.cfg.py                      
│   ├── general
|   |   ├── swap_test.c
|   |   ├── ...
│   ├── include
|   |   └── PulseMacros.h
│   ├── snapshots
│   └──  ...
├── include/                            # Header files for Pulse logic           
|   ├── ExprLocationAnalyzer.h          
|   ├── Globals.h  
│   ├── MacroCommentTracker.h
│   ├── MacroFrontendAction.h                      
│   ├── PulseGenerator.h                
│   ├── PulseCodeGen.h                  
│   └── PulseIR.h                      
└── src/                              
    ├── ExprLocationAnalyzer.cpp        # Utility to locate expressions in source
    ├── Globals.cpp                     # Global utilities and shared state
    ├── MacroCommentTracker.cpp
    ├── MacroFrontendAction.cpp  
    ├── PulseASTGenerator.cpp           # Pulse AST builder from standard C
    ├── PulseCodeGen.cpp                # Emits Pulse code from Pulse IR
    ├── PulseIR.cpp                     # Pulse IR utility methods and definitions
    └── main.cpp                        # Tool entry point and Clang integration

```
---

## Project Dependencies 

### Ubuntu
```
sudo apt install opam
```

## Cloning the repo
Ensure that you clone all submodules:
```bash
git clone --recurse-submodules git@github.com:FStarLang/c2pulse.git
````

## Building C2Pulse 

```bash
./build.sh
```

Set the environment variable `C2PULSE_BUILD_TYPE=Release` if you are not a developer for faster build and faster
binary executable. This will produce a binary executable in the build/bin directory of the LLVM build. 
The binary will be called `c2pulse`. You can run the tool as follows:

```bash
./external/llvm-project/build/bin/c2pulse /path/to/test.c
```

Alternatively, please find a `run_c2pulse.sh` scipt in the root directory.

---

## Run the C2Pulse Test Suite

To verify the correctness of the tool, run the semantic test suite from the root of the project:

```bash
./scripts/test_runner.sh 
```

This will execute all test cases under the `test/` directory, checking both the generated outputs and expected diagnostics.

You can also run the test suite on a specific directory:

```bash
./scripts/run_test.sh ./test/general/
```

Or file:

```bash
./scripts/run_test.sh ./test/general/swap_test.c
```

---

## What It Does

Given a simple C function annotated with correct Pulse specifications in filename swap.c.
For available user defined specfications, kindly look at the file `pulse_macros.h` in 
the directory `include/`.

```c
REQUIRES((r1 `pts_to` 'w1) ** (r2 `pts_to` 'w2))
ENSURES((r1 `pts_to` 'w2) ** (r2 `pts_to` 'w1))
void ref_swap(int* r1, int* r2) 
{
  int tmp = *r1;
  *r1 = *r2;
  *r2 = tmp;
}
```

The plugin rewrites it to a Pulse program stored in file Swap.fst.
In general, it emits pulse code for the file in a file called
Filename.fst (capitalizes the first letter)

```fstar
module Swap

#lang-pulse

open Pulse

fn ref_swap
(r1 : ref Int32.t)
(r2 : ref Int32.t)
requires (r1 `pts_to` 'w1) ** (r2 `pts_to` 'w2)
ensures (r1 `pts_to` 'w2) ** (r2 `pts_to` 'w1)
{
  let tmp = (! r1);
  r1 := (! r2);
  r2 := tmp;
}
```

* Emits specifications for use with formal tools like Pulse
* Generates Pulse code that can be compiled with Fstar.
---

## Debug Modes (Developer Feature)

C2Pulse leverages LLVM’s internal debugging infrastructure. To help developers trace internal transformations and behavior, several components include debug output guarded by `DEBUG_TYPE`.

> Note: This feature is intended for developers and contributors who want to inspect or debug the internal behavior of the tool.

### Enabling Debug Output for Specific Components

Each major part of C2Pulse defines its own `DEBUG_TYPE`. You can enable debug output for a specific component using:

```bash
./run.sh ./test/general/swap_test.c -debug --debug-only=ast-loc-info
```

Available `DEBUG_TYPE`'s include:

| `DEBUG_TYPE`            | Component                  |
| ----------------------- | -------------------------- |
| `ast-loc-info`          | `ExprLocationAnalyzer.cpp` |
| `macro-comment-tracker` | `MacroCommentTracker.cpp`  |
| `macro-frontend-action` | `MacroFrontendAction.cpp`  |
| `pulse-ast-gen`         | `PulseASTGenerator.cpp`    |
| `pulse-code-gen`        | `PulseCodeGen.cpp`         |

To add debug output in code, use:

```cpp
#define DEBUG_TYPE "ast-loc-info"
DEBUG_WITH_TYPE("ast-loc-info", {
  llvm::dbgs() << "Debug message here\n";
});
```

### Enabling All Debug Output

To see all debug output, from every `DEBUG_TYPE`, pass the `-debug` flag alone:

```bash
./run.sh ./test/general/swap_test.c  -debug
```

Or set the environment variable:

```bash
export LLVM_DEBUG=1
./run.sh ./test/general/swap_test.c 
```

> Note: The environment variable disables all filtering, and outputs everything inside `LLVM_DEBUG(...)` and `DEBUG_WITH_TYPE(...)` blocks. 

---
