#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

_ensures(return == _old(*r))
int value_of(int *r)
{
    return *r;
}

_ensures(*r == v)
void assign(int *r, int v)
{
    *r = v;
}

_requires((_slprop) _inline_pulse(r |-> w))
_requires((_slprop) _inline_pulse(pure (fits (+) (as_int w) (as_int n))))
_ensures((_slprop) _inline_pulse(exists* ww. (r |-> ww) ** pure (as_int ww == as_int w + as_int n)))
void add(int *r, int n)
{
    *r = *r + n;
}

_requires((_slprop) _inline_pulse(r |-> w))
_ensures((_slprop) _inline_pulse(r |-> (w +^ n)))
void add_alt(int *r, int n)
{
    *r = *r + n;
}

_requires((_slprop) _inline_pulse(r |-> w))
_ensures((_slprop) _inline_pulse(exists* ww. (r |-> ww) ** pure (as_int ww == 4 * as_int w)))
void quadruple(int *r)
{
    add(r, *r);
    add(r, *r);
}

_requires((_slprop) _inline_pulse(x |-> Frac p w))
_ensures((_slprop) _inline_pulse(x |-> Frac p w))
_ensures(return == (_specint) w)
int value_of_perm(int *x)
{
    return *x;
}

_requires((_slprop) _inline_pulse(x |-> Frac p v))
_ensures((_slprop) _inline_pulse(x |-> Frac (p /. 2.0R) v))
_ensures((_slprop) _inline_pulse(x |-> Frac (p /. 2.0R) v))
void share_ref(int *x)
{
    _assert((_slprop) _inline_pulse(with vx. assert (x |-> vx)));
    _assert((_slprop) _inline_pulse(share(vx)));
}

_requires((_slprop) _inline_pulse(x |-> Frac (p /. 2.0R) v0))
_requires((_slprop) _inline_pulse(x |-> Frac (p /. 2.0R) v1))
_ensures((_slprop) _inline_pulse(x |-> Frac p v0))
_ensures(v0 == v1)
void gather_ref(int *x)
{
    _assert((_slprop) _inline_pulse(with vx. assert (x |-> vx)));
    _assert((_slprop) _inline_pulse(gather(vx)));
}

_requires((_slprop) _inline_pulse(x |-> Frac p v))
_requires((_slprop) _inline_pulse(pure (~(p <=. 1.0R))))
_ensures((_slprop) _inline_pulse(pure False))
void max_perm (int *x)
{
    _assert((_slprop) _inline_pulse(with vx. assert (x |-> vx)));
    _assert((_slprop) _inline_pulse(pts_to_perm_bound vx));
    _assert((_slprop) _inline_pulse(unreachable()));
}

_requires((_slprop) _inline_pulse(r |-> Frac p v))
_ensures((_slprop) _inline_pulse(s |-> Frac (p /. 2.0R) v))
_ensures((_slprop) _inline_pulse(s |-> Frac (p /. 2.0R) v))
_ensures(return == r)
int* alias_ref(int *r)
{
    _assert((_slprop) _inline_pulse(with vr. assert (r |-> vr)));
    _assert((_slprop) _inline_pulse(share vr));
    return r;
}

_requires((_slprop) _inline_pulse(r |-> vr ** pure (fits (+) (as_int vr) 1)))
_ensures((_slprop) _inline_pulse(exists* w. (r |-> w) ** pure (as_int w == as_int vr + 1)))
void incr (int *r)
{
    *r = *r + 1;
}

_ensures(return == (_specint) 1)
int one()
{
    int i = 0;
    incr(&i);
    return i;
}

_ensures((_slprop) _inline_pulse(s |-> 0l))
int* refs_are_scoped()
{
    int s = 0;
    return &s;
}
