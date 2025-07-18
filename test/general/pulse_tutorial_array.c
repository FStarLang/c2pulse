#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"


INCLUDE ( 
  module SZ = Pulse.Lib.C.SizeT
 )

ERASED_ARG(#s:erased (Seq.seq int32))
ERASED_ARG(#p:perm  { SZ.as_int i < Seq.length s })
REQUIRES(arr |-> Frac p s)
RETURNS(v:int32)
ENSURES(arr |-> Frac p s)
ENSURES(pure (v == Seq.index s (SZ.as_int i) ))
int read_i(ISARRAY() int *arr, size_t i)
{
    return arr[i];
}

ERASED_ARG(#s:erased (Seq.seq int32) { SZ.as_int i < Seq.length s })
REQUIRES(arr |-> s)
ENSURES(exists* s1. (arr |-> s1) ** pure (s1 == Seq.upd s (SZ.as_int i) v))
void write_i(ISARRAY() int *arr, size_t i, int v)
{
    arr[i] = v;
}

ERASED_ARG(#s1 #s2 : erased (Seq.seq int32))
ERASED_ARG(#p:perm { Seq.length s1 = Seq.length s2 && Seq.length s2 = SZ.as_int l })
REQUIRES(a1 |-> Frac p s1)
REQUIRES(a2 |-> Frac p s2)
RETURNS(res:int32)
ENSURES(a1 |-> Frac p s1)
ENSURES(a2 |-> Frac p s2)
ENSURES(pure (res==1l <==> (SZ.as_int i < SZ.as_int l && Seq.index s1 (SZ.as_int i) = Seq.index s2 (SZ.as_int i))))
int compare_elements(ISARRAY() int *a1, ISARRAY() int *a2, size_t l, size_t i)
{
    if (i < l)
    {
        if (a1[i] == a2[i])
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

INCLUDE (
    module SizeT = Pulse.Lib.C.SizeT
)

ERASED_ARG(#s1 #s2 : erased (Seq.seq int32))
ERASED_ARG(#p:perm { Seq.length s1 = Seq.length s2 && Seq.length s2 = SZ.as_int l })
REQUIRES(a1 |-> Frac p s1)
REQUIRES(a2 |-> Frac p s2)
RETURNS(res:bool)
ENSURES(a1 |-> Frac p s1)
ENSURES(a2 |-> Frac p s2)
ENSURES(pure (res <==> Seq.equal s1 s2))
bool compare(ISARRAY() int *a1, ISARRAY() int *a2, size_t l)
{
    size_t i = 0;
    while( compare_elements(a1, a2, l, i) == 1 )
    INVARIANTS("invariant b.\
        exists* vi vl.\
            (i |-> vi) ** (a1 |-> Frac p s1) ** (a2 |-> Frac p s2) **\
            (l |-> vl) **\
            pure (\
                SZ.as_int vi <= SZ.as_int vl /\ \
                (b == (SZ.as_int vi < SZ.as_int vl && Seq.index s1 (SZ.as_int vi) = Seq.index s2 (SZ.as_int vi))) /\ \
                (forall (i:nat). i < SZ.as_int vi ==> Seq.index s1 i == Seq.index s2 i)            \
            )"
    )
    {
        i = i + 1;
    }
    return (i == l);
}
