#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

INCLUDE (
    ghost
    fn freebie ()
    requires emp
    ensures pure False
    {
        admit();
    }    
)

REQUIRES(emp)
RETURNS(i:int32)
ENSURES(emp)
int test_empty(void)
{ return 0; }

RETURNS (i:int32)
ENSURES(pure False)
int test_freebie(void)
{
    LEMMA(freebie());
    return 0;
}

RETURNS(i:ref int32)
ENSURES(i |-> v)
ENSURES(freeable i)
int* new_heap_ref(int v)
{
    int *r = (int*)malloc(sizeof(int));
    *r = v;
    return r;
}


ERASED_ARG(#w:_)
REQUIRES((r |-> w) ** freeable r)
RETURNS(i:int32)
ENSURES(pure (i == w))
int last_value_of(int* r)
{
   int v = *r;
   free(r);
   return v;   
}

ERASED_ARG(#w:_)
REQUIRES(r |-> w)
REQUIRES(freeable r)
RETURNS(s:ref int32)
ENSURES(s |-> w)
ENSURES(freeable s)
int* copy_free_box (int* r)
{
    int v = *r;
    free(r);
    int *s = (int*)malloc(sizeof(int));
    *s = v;
    return s;
}

ERASED_ARG(#w:_)
REQUIRES(r |-> w)
RETURNS(s:ref int32)
ENSURES((r |-> w) ** (s |-> w) ** freeable s)
int* copy_box(int* r)
{
    int v = *r;
    int *s = (int*)malloc(sizeof(int));
    *s = v;
    return s;
}