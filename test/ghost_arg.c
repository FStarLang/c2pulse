#include "pal.h"

void foo(int *p)
    _ghost_arg(_specint n)
    _requires(*p == n)
    _ensures(*p == n)
{}