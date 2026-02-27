#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

_include_pulse(
    ghost
    fn freebie ()
    requires emp
    ensures pure False
    {
        admit();
    }    
)

int test_empty(void)
{ return 0; }

_ensures((_slprop) _inline_pulse(pure False))
int test_freebie(void)
{
    _assert((_slprop) _inline_pulse(freebie()));
    return 0;
}

_ensures((_slprop) _inline_pulse(i |-> v))
_ensures((_slprop) _inline_pulse(freeable i))
int* new_heap_ref(int v)
{
    int *r = (int*)malloc(sizeof(int));
    *r = v;
    return r;
}

_requires((_slprop) _inline_pulse((r |-> w) ** freeable r))
_ensures(i == w)
int last_value_of(int* r)
{
   int v = *r;
   free(r);
   return v;   
}

_requires((_slprop) _inline_pulse(r |-> w))
_requires((_slprop) _inline_pulse(freeable r))
_ensures((_slprop) _inline_pulse(s |-> w))
_ensures((_slprop) _inline_pulse(freeable s))
int* copy_free_box (int* r)
{
    int v = *r;
    free(r);
    int *s = (int*)malloc(sizeof(int));
    *s = v;
    return s;
}

_requires((_slprop) _inline_pulse(r |-> w))
_ensures((_slprop) _inline_pulse((r |-> w) ** (s |-> w) ** freeable s))
int* copy_box(int* r)
{
    int v = *r;
    int *s = (int*)malloc(sizeof(int));
    *s = v;
    return s;
}