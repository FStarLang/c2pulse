#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../c2pulse.h"

_include_pulse( 
  module SZ = Pulse.Lib.C.SizeT
  module U64 = FStar.UInt64
 )

_requires((_slprop) _inline_pulse(arr |-> Frac p s))
_ensures((_slprop) _inline_pulse(arr |-> Frac p s))
_ensures((_slprop) _inline_pulse(pure (v == Seq.index s (SizeT.v i) )))
int read_i(int *arr, size_t i)
{
    return arr[i];
}

_requires((_slprop) _inline_pulse(arr |-> s))
_ensures((_slprop) _inline_pulse(exists* s1. (arr |-> s1) ** pure (s1 == Seq.upd s (SizeT.v i) v)))
void write_i(int *arr, size_t i, int v)
{
    arr[i] = v;
}

_preserves((_slprop) _inline_pulse(a1 |-> Frac p s1))
_preserves((_slprop) _inline_pulse(a2 |-> Frac p s2))
_ensures((_slprop) _inline_pulse(pure (res==1l <==> (SizeT.v i < SizeT.v l && Seq.index s1 (SizeT.v i) = Seq.index s2 (SizeT.v i)))))
int compare_elements(int *a1, int *a2, size_t l, size_t i)
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

_include_pulse(
    module SizeT = Pulse.Lib.C.SizeT
)

_preserves((_slprop) _inline_pulse(a1 |-> Frac p s1))
_preserves((_slprop) _inline_pulse(a2 |-> Frac p s2))
_ensures((_slprop) _inline_pulse(pure (res <==> Seq.equal s1 s2)))
bool compare(int *a1, int *a2, size_t l)
{
    size_t i = 0;
    _assert((_slprop) _inline_pulse(with va1. assert (a1 |-> va1)));
    _assert((_slprop) _inline_pulse(with va2. assert (a2 |-> va2)));
    while( compare_elements(a1, a2, l, i) == 1 )
        _invariant((_slprop) _inline_pulse(exists* vi vl.
        (a1 |-> va1) ** (a2 |-> va2) ** (* tedious *)
        (i |-> vi) ** (va1 |-> Frac p s1) ** (va2 |-> Frac p s2) **
        (l |-> vl) **
        pure (
        Seq.length s1 = SizeT.v vl
        /\ Seq.length s2 = SizeT.v vl
        /\ SizeT.v vi <= SizeT.v vl
        /\ (b == (SizeT.v vi < SizeT.v vl && Seq.index s1 (SizeT.v vi) = Seq.index s2 (SizeT.v vi)))
        /\ (forall (i:nat). i < SizeT.v vi ==> Seq.index s1 i == Seq.index s2 i))))
    {
        i = i + 1;
    }
    return (i == l);
}
