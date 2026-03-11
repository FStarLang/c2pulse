// Tests-todo: Mixed integer type comparisons
//
// Missing features: implicit integer conversions in binary operations
//   (e.g., comparing uint32_t to uint64_t)
// Prototype tests: simple_compare_sizet.c (baz, boo functions)
//
// C performs "usual arithmetic conversions" when operands have different
// integer types. c2pulse should insert explicit widening casts.

#include "c2pulse.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool compare_u32_u64(uint32_t i, uint64_t j) {
    if (i < j) {
        return true;
    } else {
        return false;
    }
}

bool compare_u64_u32(uint64_t i, uint32_t j) {
    if (i < j) {
        return true;
    } else {
        return false;
    }
}
