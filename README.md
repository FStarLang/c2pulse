# C2Pulse: Clang-based C to Pulse transpiler

C2Pulse is a Clang-based source-to-source transformation plugin that rewrites C functions into equivalent Pulse programs. It uses macros to allow the user to add **Pulse specifications**. For instance, `ENSURES(s)`, `REQUIRES(s)` etc. For a list of macros we support please check `CodeShield/test-transpiler/c/pulse_macros.h`.

## Project Structure
```
c2pulse/
├── CMakeLists.txt                      # Build configuration for the transpiler
├── README.md
├── build.sh                            # Build c2pulse and create a symlink in llvm-project
├── run.sh                              # Run the transpiler C2Pulse and checks the output with Fstar
├── run_c2pulse.sh                      # Runs only the transpiler and generates the Pulse files from the input C     
├── run_fstar.sh                        # Runs Fstart on top of the generated files to check them   
├── run-lit.sh                          # Run the lit test-suite
├── test/                               # Lit-based test suite
│   ├── CMakeLists.txt                  # Includes test files in the build
│   ├── lit.cfg.py                      # LIT configuration for test execution
│   └── complex_test.c
│   └── reverse_test.c
│   └── swap_test.c
│   └── ...
├── include/                            # Header files for Pulse logic           
|   ├── ExprLocationAnalyzer.h          
|   ├── Globals.h                       
│   ├── PulseGenerator.h                
│   ├── PulseCodeGen.h                  
│   └── PulseIR.h                       
└── src/                              
    ├── ExprLocationAnalyzer.cpp        # Utility to locate expressions in source
    ├── Globals.cpp                     # Global utilities and shared state
    ├── PulseGenerator.cpp              # Pulse IR builder from standard C or C in ANF-like form
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

## Run the C2Pulse Lit Test Suite
To verify the correctness of the tool, you can run the included semantic test cases using LLVM's lit infrastructure. From the root of the project, run:

```bash
./run-lit.sh ./test/
```

This will run all test cases located in the `test/` directory using the LLVM testing infrastructure and check expected outputs and diagnostics.

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

