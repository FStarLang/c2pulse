#include <stdio.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"

REQUIRES(pure (Int32.v a < 100))
RETURNS(Int32.t)
int foo(int a, bool b) {
    a = a + b;
    int *p; 
    p = &a;
    *p = *p + 123;
    return *p;
}
