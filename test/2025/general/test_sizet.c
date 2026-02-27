#include <stdio.h>
#include <stdint.h>
#include "../c2pulse.h"

unsigned long long test2(size_t l, size_t k, unsigned long long j);

_requires((_slprop) _inline_pulse(pure (SizeT.v l < 100)))
unsigned long foo(size_t l);

unsigned long foo(size_t l){

    unsigned long y = l;
    int yy = l;
    long long jj = yy + 100;
    y = y + 1;
    y = y + 100; 
    y = y * 10;
    return y + yy + jj;
}

_requires((_slprop) _inline_pulse(pure (SizeT.v l < 100)))
unsigned long long test(size_t l, size_t k, unsigned long long j){

    k = l;
    k = k + 1;
    k = k - 1;
    j = foo(k);
    _assert((_slprop) _inline_pulse(assume (pure (UInt64.v !j < 1000))));
    k = k + j;
    
    return k;
}