#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

ERASED_ARG(#w:erased _)
REQUIRES(r |-> w)
RETURNS(v:int32)
ENSURES(r |-> w)
ENSURES(pure (v == w))
int value_of(int *r)
{
    return *r;
}

REQUIRES(exists* w. r |-> w)
ENSURES(r |-> v)
void assign(int *r, int v)
{
    *r = v;
}

ERASED_ARG(#w:erased _)
REQUIRES(r |-> w)
REQUIRES(pure (fits (+) (as_int w) (as_int n)))
ENSURES(exists* ww. (r |-> ww) ** pure (as_int ww == as_int w + as_int n))
void add(int *r, int n)
{
    *r = *r + n;
}

ERASED_ARG(#w : erased _ { (fits (+) (as_int w) (as_int n)) })
REQUIRES(r |-> w)
ENSURES(r |-> (w +^ n))
void add_alt(int *r, int n)
{
    *r = *r + n;
}

ERASED_ARG(#w : erased _ { (fits ( * ) 4 (as_int w)) })
REQUIRES(r |-> w)
ENSURES(exists* ww. (r |-> ww) ** pure (as_int ww == 4 * as_int w))
void quadruple(int *r)
{
    add(r, *r);
    add(r, *r);
}


ERASED_ARG(#w:erased _)
ERASED_ARG(p:perm)
REQUIRES(x |-> Frac p w)
RETURNS(i:int32)
ENSURES(x |-> Frac p w)
ENSURES(pure (i == w))
int value_of_perm(int *x)
{
    return *x;
}

ERASED_ARG(#v:erased _)
ERASED_ARG(p:perm)
REQUIRES(x |-> Frac p v)
ENSURES(x |-> Frac (p /. 2.0R) v)
ENSURES(x |-> Frac (p /. 2.0R) v)
void share_ref(int *x)
{
    LEMMA(with vx. assert (x |-> vx));
    LEMMA(share(vx));
}


ERASED_ARG(#v0:erased _)
ERASED_ARG(#v1:erased _)
ERASED_ARG(p:perm)
REQUIRES(x |-> Frac (p /. 2.0R) v0)
REQUIRES(x |-> Frac (p /. 2.0R) v1)
ENSURES(x |-> Frac p v0)
ENSURES(pure (v0 == v1))
void gather_ref(int *x)
{
    LEMMA(with vx. assert (x |-> vx));
    LEMMA(gather(vx));
}


ERASED_ARG(#v:erased _)
ERASED_ARG(p:perm)
REQUIRES(x |-> Frac p v)
REQUIRES(pure (~(p <=. 1.0R)))
ENSURES(pure False)
void max_perm (int *x)
{
    LEMMA(with vx. assert (x |-> vx));
    LEMMA(pts_to_perm_bound vx);
    LEMMA(unreachable());
}

ERASED_ARG(#v:erased _)
ERASED_ARG(p:perm)
REQUIRES(r |-> Frac p v)
RETURNS(s: ref int32)
ENSURES(s |-> Frac (p /. 2.0R) v)
ENSURES(s |-> Frac (p /. 2.0R) v)
ENSURES(pure (s == r))
int* alias_ref(int *r)
{
    LEMMA(with vr. assert (r |-> vr));
    LEMMA(share vr);
    return r;
}


ERASED_ARG(#vr:erased _)
REQUIRES(r |-> vr)
REQUIRES(pure (fits (+) (as_int vr) 1))
ENSURES(exists* w. (r |-> w) ** pure (as_int w == as_int vr + 1))
void incr (int *r)
{
    *r = *r + 1;
}



REQUIRES(emp)
RETURNS(i:int32)
ENSURES(pure (as_int i == 1))
int one()
{
    int i = 0;
    incr(&i);
    return i;
}

EXPECT_FAILURE(228)
RETURNS(s:ref int32)
ENSURES(s |-> 0l)
int* refs_are_scoped()
{
    int s = 0;
    return &s;
}
