// Tests-todo: Recursive functions
//
// Missing features: recursive function calls with verified termination
// Prototype tests: complex_test.c (recursiveFunction),
//   pulse_tutorial_loops.c (fib_rec, call_fib_rec)
//
// The prototype supported recursive functions with explicit Pulse specs.
// Proposed: recursive functions should work with standard _requires/_ensures;
// termination metrics can be specified with a new annotation.

#include "c2pulse.h"
#include <stdint.h>

// Proposed: simple recursion with _decreases annotation
// _decreases(limit - x)
// int recursive_sum(int x, int limit)
//     _requires(x <= limit)
//     _requires((_specint) limit * 12 <= INT32_MAX)
//     _ensures(return == (limit - x) * 12)
// {
//     if (x >= limit) return 0;
//     else return 12 + recursive_sum(x + 1, limit);
// }

// Proposed: fibonacci with _include_pulse for spec function
// _include_pulse(
//     let rec fib (n: nat) : nat =
//         if n <= 1 then 1
//         else fib (n - 1) + fib (n - 2)
// )
//
// _decreases(n)
// void fib_rec(int n, int *cur, int *prev)
//     _requires(n > 0)
//     _requires((_specint) _inline_pulse(fib (Int32.v $(n))) <= INT32_MAX)
//     _ensures(n > 0 ==> *cur == (_specint) _inline_pulse(fib (Int32.v $(_old(n)))))
// {
//     if (n == 1) {
//         *cur = 1;
//         *prev = 1;
//     } else {
//         fib_rec(n - 1, cur, prev);
//         int tmp = *cur;
//         *cur = *cur + *prev;
//         *prev = tmp;
//     }
// }
