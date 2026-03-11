// Tests-todo: Unary negation on unsigned types
//
// Missing features: unary minus on unsigned integers (negation)
// Prototype tests: issue51_test_2.c
//
// The C standard defines -x on unsigned types as producing a valid
// unsigned result (2^N - x). c2pulse should support this.

#include "c2pulse.h"
#include <stdint.h>

int neg_u32(uint32_t x) {
    return -x;
}

int64_t neg_u64(uint64_t x) {
    return -x;
}
