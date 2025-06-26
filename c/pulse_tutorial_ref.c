#include <stdint.h>
#include <stdlib.h>
#include "pulse_macros.h"

REQUIRES("r |-> 'v")
RETURNS(v)
ENSURES("r |-> 'v")
ENSURES("pure (v == 'v)")
int value_of(int *r)
{
    return *r;
}

ERASED_ARG(w)
REQUIRES(r |-> w)
RETURNS(v)
ENSURES(r |-> w)
ENSURES(pure (v == w))
int value_of_explicit(int *r)
{
    return *r;
}

REQUIRES("r |-> 'v")
ENSURES(r |-> v)
void assign(int *r, int v)
{
    *r = v;
}

REQUIRES(exists* w. r |-> w)
ENSURES(r |-> v)
void assign_alt(int *r, int v)
{
    *r = v;
}

ERASED_ARG(w)
REQUIRES(r |-> w)
REQUIRES(pure FStar.Int32.(fits (v w + n)))
ENSURES(exists* ww. r |-> ww ** pure FStar.Int32.(v ww == v w + n))
void add(int *r, int n)
{
    *r = *r + n;
}

ERASED_ARG(w : FStar.Int32.t { FStar.Int32.(fits (v w + n)) })
REQUIRES(r |-> w)
ENSURES(r |-> FStar.Int32.(w +^ n))
void add_alt(int *r, int n)
{
    *r = *r + n;
}

ERASED_ARG(w : FStar.Int32.t { FStar.Int32.fits(4 * v w) })
REQUIRES(r |-> w)
ENSURES(exists* ww. r |-> ww ** pure FStar.Int32.(v ww == 4 * v w))
void quadruple(int *r)
{
    add(r, *r);
    add(r, *r);
}

ERASED_ARG(w:FStar.Int32.t)
ERASED_ARG(p:perm)
REQUIRES(x |-> Frac p w)
RETURNS(i)
ENSURES(x |-> Frac p w)
ENSURES(pure (i == w))
int value_of_perm(int *x)
{
    return *x;
}

ERASED_ARG(v)
ERASED_ARG(p:perm)
REQUIRES(x |-> Frac p v)
ENSURES(x |-> Frac (p /. 2.0R) v)
ENSURES(x |-> Frac (p /. 2.0R) v)
void share_ref(int *x)
{
    LEMMA(share(x));
}

ERASED_ARG(v0)
ERASED_ARG(v1)
ERASED_ARG(p:perm)
REQUIRES(x |-> Frac (p /. 2.0R) v0)
REQUIRES(x |-> Frac (p /. 2.0R) v1)
ENSURES(x |-> Frac p v0)
ENSURES(pure (v0 == v1))
void gather_ref(int *x)
{
    LEMMA(gather(x));
}

ERASED_ARG(v)
ERASED_ARG(p:perm)
REQUIRES(x |-> Frac p v)
REQUIRES(pure (~(p <=. 1.0R)))
ENSURES(pure False)
void max_perm (int *x)
{
    LEMMA(pts_to_perm_bound x);
    LEMMA(unreachable());
}

ERASED_ARG(v)
ERASED_ARG(p:perm)
REQUIRES(r |-> Frac p v)
RETURNS(s)
ENSURES(s |-> Frac (p /. 2.0R) v)
ENSURES(s |-> Frac (p /. 2.0R) v)
ENSURES(pure (s == r))
int* alias_ref(int *r)
{
    LEMMA(share r);
    return r;
}

ERASED_ARG(vr)
REQUIRES(r |-> vr)
ENSURES(exists* w. r |-> w ** pure FStar.Int32.(v w == v vr + 1))
int incr (int *r)
{
    *r = *r + 1;
}


RETURNS(i)
ENSURES(pure (i == 1l))
int one()
{
    int i = 0;
    incr(&i);
    return i;
}

EXPECT_FAILURE(19)
RETURNS(s)
ENSURES(s |-> 0l)
int* refs_are_scoped()
{
    int s = 0;
    return &s;
}