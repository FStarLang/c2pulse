// Tests-todo: Negative integer literals
//
// Missing features: negative integer literals (e.g., -10, -100)
// Prototype tests: test_logicals_int.c (f4, f5)
//
// The C parser sees `-10` as UnaryOperator(Minus, 10). c2pulse currently
// doesn't support the unary minus operator on signed integer literals.
// This should be straightforward to add as a constant folding or
// by emitting Int32.int_to_t (-10).

#include "c2pulse.h"

int test_negative_literal() {
    int x = -10;
    int y = -100;
    return x + y;
}
