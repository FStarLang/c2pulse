#include <stdio.h>
#include <stdbool.h>
#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(pure (Int32.v a < 100)))
int foo(int a, bool b) {
    a = a + b;
    int *p; 
    p = &a;
    *p = *p + 123;
    return *p;
}
