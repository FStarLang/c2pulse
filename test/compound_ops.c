// Tests-todo: Increment, decrement, and compound assignment operators
//
// Missing features: ++, --, +=, -=, *=, /=, %=
// Prototype tests: test_ops.c, random.c
//
// These operators are syntactic sugar for existing supported operations.
// c2pulse should desugar them during translation.

#include "c2pulse.h"
#include <stdint.h>

uint32_t test_incr(uint32_t a)
    _requires(a < 1000)
{
    a++;
    return a;
}

uint32_t test_decr(uint32_t a)
    _requires(a > 0)
{
    a--;
    return a;
}

uint32_t test_compound_add(uint32_t a, uint32_t b)
    _requires(a < 1000 && b < 1000)
{
    a += b;
    return a;
}

uint32_t test_compound_sub(uint32_t a, uint32_t b)
    _requires(a >= b)
{
    a -= b;
    return a;
}
