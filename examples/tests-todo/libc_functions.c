// Tests-todo: Library function specs (abs, etc.)
//
// Missing features: specifications for C standard library functions
//   like abs(), along with general support for extern function declarations
//   with c2pulse specifications
// Prototype tests: test_funs.c, test_funs2.c, test_funs3.c, test_funs4.c
//
// The prototype had a LibcFunctions.h that provided Pulse specs for
// standard C library functions. The new c2pulse needs a way to declare
// external function specs.
//
// Proposed: use _include_pulse to declare external function specs,
// or have a c2pulse-standard-library Pulse module.

#include "c2pulse.h"
#include <stdlib.h>

int use_abs(int a) {
    int abs_a = abs(a);
    return abs_a;
}

int call_chain(int a, int b) {
    int abs_a = abs(a);
    int abs_b = abs(b);
    return abs_a + abs_b;
}
