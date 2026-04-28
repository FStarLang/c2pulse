// Tests-todo: Increment, decrement, and compound assignment operators
//
// Missing features: ++, --, +=, -=, *=, /=, %=
// Prototype tests: test_ops.c, random.c
//
// These operators are syntactic sugar for existing supported operations.
// We cannot desugar them directly because the lhs might be effectful.

#include "pal.h"
#include <stddef.h>
#include <stdint.h>

int test_incr_decr(int a) {
    a++;
    ++a;
    a--;
    --a;
    return a;
}

int test_compound_assign(int a) {
    a += 1;
    a -= 1;
    a *= 2;
    a /= 1;
    return a;
}

size_t test_sizet_incr(size_t a) {
    a++;
    a--;
    return a;
}

uint32_t test_u32_incr(uint32_t a) {
    a++;
    a--;
    return a;
}
