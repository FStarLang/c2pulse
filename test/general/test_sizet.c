#include <stdio.h>
#include <stdint.h>
#include "../include/PulseMacros.h"


unsigned long long test2(size_t l, size_t k, unsigned long long j);
unsigned long foo(size_t l);

REQUIRES(pure (SizeT.v l < 100))
RETURNS(UInt64.t)
unsigned long foo(size_t l){

    unsigned long y = l;
    int yy = l;
    long long jj = yy + 100;
    y = y + 1;
    y = y + 100; 
    y = y * 10;
    return y + yy + jj;
}

REQUIRES(pure (SizeT.v l < 100))
RETURNS(UInt64.t)
unsigned long long test(size_t l, size_t k, unsigned long long j){

    k = l;
    k = k + 1;
    k = k - 1;

    j = foo(k);
    
    LEMMA(assume (pure (UInt64.v !j < 1000)));
    k = k + j;
    
    return k;
}