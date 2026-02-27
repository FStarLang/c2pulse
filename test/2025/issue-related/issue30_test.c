
#include "../c2pulse.h"

_include_pulse(let max_spec x y = if x < y then y else x)

_requires((_slprop) _inline_pulse(x |->Frac px vx))
_requires((_slprop) _inline_pulse(y |->Frac py vy))
_ensures((_slprop) _inline_pulse(x |->Frac px vx))
_ensures((_slprop) _inline_pulse(y |->Frac py vy))
_ensures((_slprop) _inline_pulse(pure(as_int n == max_spec(as_int vx)(as_int vy))))
// see comments below
int max_alt(int *x, int *y) {
  _assert((_slprop) _inline_pulse(let orig_x = x)); // doesn't help, x is the local shadowed version
  int result = 0;
  int xx = *x;
  int yy = *y;
  _assert((_slprop) _inline_pulse(with vx. assert x |-> vx));
  _assert((_slprop) _inline_pulse(with vy. assert y |-> vy));
  _assert((_slprop) _inline_pulse(with vxx. assert xx |-> vxx));
  _assert((_slprop) _inline_pulse(with vyy. assert yy |-> vyy));
  if (xx > yy)
    // Note: if does not frame anything, so the ensures must mention the full
    // postcondition. But we need to mention the **original** x and y, not the
    // locally shadowed versions, so we're out of luck here.
    _ensures((_slprop) _inline_pulse(exists* r. (x |-> Frac px vx) ** (y |-> Frac py vy) ** (xx |-> vxx) ** 
                       (yy |-> vyy) **
                       (result |-> r) **
                       pure (as_int r == max_spec (as_int vxx) (as_int vyy)))) 
    {
      result = xx;
    }
    else {
        result = yy;
    }
  return result;
}

// However, this now works without an ENSURES clause
_requires((_slprop) _inline_pulse(x |->Frac px vx))
_requires((_slprop) _inline_pulse(y |->Frac py vy))
_ensures((_slprop) _inline_pulse(x |->Frac px vx))
_ensures((_slprop) _inline_pulse(y |->Frac py vy))
_ensures((_slprop) _inline_pulse(pure(as_int n == max_spec(as_int vx)(as_int vy))))
int max_alt2(int *x, int *y) {
  int result = 0;
  int vx = *x;
  int vy = *y;
  if (vx > vy)
    {
      result = vx;
    }
    else {
        result = vy;
    }
  return result;
}

// Or more naturally:
_requires((_slprop) _inline_pulse(x |->Frac px vx))
_requires((_slprop) _inline_pulse(y |->Frac py vy))
_ensures((_slprop) _inline_pulse(x |->Frac px vx))
_ensures((_slprop) _inline_pulse(y |->Frac py vy))
_ensures((_slprop) _inline_pulse(pure(as_int n == max_spec(as_int vx)(as_int vy))))
int max_alt3(int *x, int *y) {
  int result = 0;
  if (*x > *y)
    {
      result = *x;
    }
    else {
        result = *y;
    }
  return result;
}

// Or more naturally:
_requires((_slprop) _inline_pulse(x |->Frac px vx))
_requires((_slprop) _inline_pulse(y |->Frac py vy))
_ensures((_slprop) _inline_pulse(x |->Frac px vx))
_ensures((_slprop) _inline_pulse(y |->Frac py vy))
_ensures((_slprop) _inline_pulse(pure(as_int n == max_spec(as_int vx)(as_int vy))))
int max_alt4(int *x, int *y) {
  if (*x > *y)
    return *x;
  else
    return *y;
}
