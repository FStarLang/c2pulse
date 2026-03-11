// Tests-todo: Increment, decrement, and compound assignment operators
//
// Missing features: ++, --, +=, -=, *=, /=, %=
// Prototype tests: test_ops.c, random.c
//
// These operators are syntactic sugar for existing supported operations.
// c2pulse should desugar them during translation.

#include "c2pulse.h"
#include <stdint.h>

uint32_t test_post_incr(uint32_t a)
    _requires(a < 1000)
{
    a++;
    return a;
}

uint32_t test_pre_incr(uint32_t a)
    _requires(a < 1000)
{
    ++a;
    return a;
}

uint32_t test_post_decr(uint32_t a)
    _requires(a > 0)
{
    a--;
    return a;
}

uint32_t test_pre_decr(uint32_t a)
    _requires(a > 0)
{
    --a;
    return a;
}
