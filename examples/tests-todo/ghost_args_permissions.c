// Tests-todo: Fractional permissions and explicit Pulse-level ghost arguments
//
// Missing features:
//   - ERASED_ARG / ghost arguments (specification-level variables)
//   - Fractional permissions (Frac p v)
//   - share/gather permission splitting
//   - Nullable pointer specs (|->?)
// Prototype tests: pulse_tutorial_ref.c, pulse_tutorial_conditionals.c,
//   issue1_test.c, issue2_test.c, issue16_test.c, issue29_test.c
//
// The new c2pulse uses _old(), _live(), and direct C expressions for specs
// instead of explicit Pulse-level ghost arguments. Some features may not
// have a direct equivalent yet.
//
// Proposed: for read-only parameters, use `const` + automatic treatment.
// For explicit ghost args, consider a _ghost() annotation:

#include "c2pulse.h"

// Read a value through a const pointer (already works via const_read.c):
int value_of(const int *r)
    _ensures(return == *r)
{
    return *r;
}

// Assign through a pointer (already works):
void assign(int *r, int v) {
    *r = v;
}

// Proposed syntax for ghost arguments when needed:
// _ghost(int old_val)
// void incr(int *r)
//     _requires(*r == old_val)
//     _requires((_specint) old_val + 1 <= INT32_MAX)
//     _ensures(*r == old_val + 1)
// {
//     *r = *r + 1;
// }

// For now, use _old():
void incr(int *r)
    _requires((_specint) *r + 1 <= INT32_MAX)
    _ensures(*r == _old(*r) + 1)
{
    *r = *r + 1;
}

// Proposed syntax for nullable pointer annotations:
// _nullable int *
// Or: _refine(this == NULL || ...) int *

// Proposed syntax for fractional permissions (read-only sharing):
// void read_shared(_shared int *x, _shared int *y)
//     _ensures(return == *x + *y)
// { ... }
