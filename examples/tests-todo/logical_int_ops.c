// Tests-todo: Logical operators on integer types (non-bool)
//
// Missing features: &&, ||, ! on int operands (C coerces 0 → false, nonzero → true)
// Also: double-negation !!x as integer-to-bool idiom
// Prototype tests: test_logicals_int.c, issue54_test_3.c, issue_anon_1.c
//
// In C, && and || are defined to work on any scalar type, converting
// to bool implicitly. c2pulse currently only supports these on bool/slprop.
// Proposed: insert implicit int-to-bool coercion (x != 0) before logical ops.

#include "c2pulse.h"
#include <stdint.h>

int test_logical_and() {
    int x = 11;
    int y = 10;
    int z = x && y;
    return z;
}

int test_logical_or() {
    int x = 10;
    int y = 11;
    int z = x || y;
    return z;
}

int test_double_negation() {
    int x = 10;
    int y = 20;
    return !!x + !!y;
}

int test_negated_and() {
    int x = -10;
    int y = 10;
    int z = !(x && y);
    return z;
}
