#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "../c2pulse.h"

int test_ampamp(){
    int a; 
    int b;
    return a && b;
}

int test_barbar(){
    int a; 
    int b;
    return a || b;

}

_requires((_slprop) _inline_pulse(pure (length a1 == SizeT.v len)))
_requires((_slprop) _inline_pulse(pure (length a2 == SizeT.v len)))
_requires((_slprop) _inline_pulse(exists* v. (a1 |-> v)))
_requires((_slprop) _inline_pulse(exists* v. (a2 |-> v)))
_ensures((_slprop) _inline_pulse(exists* v. (a1 |-> v)))
_ensures((_slprop) _inline_pulse(exists* v. (a2 |-> v)))
bool compare_elt(int *a1, int *a2, size_t len)
{   
    size_t i = 0;
    if (i < len)
    {   
        _assert((_slprop) _inline_pulse(pts_to_len !a1));
        _assert((_slprop) _inline_pulse(pts_to_len !a2));
        return ((i < len) && a1[i] == a2[i]);
    }
    else
    {
        return false;
    }
}
