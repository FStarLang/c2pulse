// Tests-todo: Expect-failure (negative) tests
//
// Missing features: an annotation to mark tests that should fail verification
// Prototype tests: many tests used EXPECT_FAILURE() to verify that
//   incorrect specs are properly rejected
//
// Proposed: _expect_failure annotation on functions

#include "pal.h"

// Proposed syntax:
// _expect_failure
// int bad_spec(int x)
//     _ensures(return == 0)
// {
//     return x;  // wrong: doesn't always return 0
// }

// _expect_failure
// int *dangling_ref()
//     _ensures(*return == 0)
// {
//     int s = 0;
//     return &s;  // dangling pointer
// }
