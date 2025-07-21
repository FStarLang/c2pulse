
#include "../include/PulseMacros.h"

INCLUDE(let max_spec x y = if x < y then y else x)

ERASED_ARG(#vx #vy : _)
ERASED_ARG(#px #py : _)
REQUIRES(x |->Frac px vx)
REQUIRES(y |->Frac py vy)
RETURNS(n : int32)
ENSURES(x |->Frac px vx)
ENSURES(y |->Frac py vy)
ENSURES(pure(as_int n == max_spec(as_int vx)(as_int vy)))
EXPECT_FAILURE() // see comments below
int max_alt(int *x, int *y) {
  LEMMA(let orig_x = x); // doesn't help, x is the local shadowed version
  int result = 0;
  int xx = *x;
  int yy = *y;
  LEMMA(with vx. assert x |-> vx);
  LEMMA(with vy. assert y |-> vy);
  LEMMA(with vxx. assert xx |-> vxx);
  LEMMA(with vyy. assert yy |-> vyy);
  if (xx > yy)
    // Note: if does not frame anything, so the ensures must mention the full
    // postcondition. But we need to mention the **original** x and y, not the
    // locally shadowed versions, so we're out of luck here.
    ENSURES(exists* r. (x |-> Frac px vx) ** (y |-> Frac py vy) ** (xx |-> vxx) ** 
                       (yy |-> vyy) **
                       (result |-> r) **
                       pure (as_int r == max_spec (as_int vxx) (as_int vyy))
           ) 
    {
      result = xx;
    }
    else {
        result = yy;
    }
  return result;
}

// However, this now works without an ENSURES clause
ERASED_ARG(#vx #vy : _)
ERASED_ARG(#px #py : _)
REQUIRES(x |->Frac px vx)
REQUIRES(y |->Frac py vy)
RETURNS(n : int32)
ENSURES(x |->Frac px vx)
ENSURES(y |->Frac py vy)
ENSURES(pure(as_int n == max_spec(as_int vx)(as_int vy)))
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
ERASED_ARG(#vx #vy : _)
ERASED_ARG(#px #py : _)
REQUIRES(x |->Frac px vx)
REQUIRES(y |->Frac py vy)
RETURNS(n : int32)
ENSURES(x |->Frac px vx)
ENSURES(y |->Frac py vy)
ENSURES(pure(as_int n == max_spec(as_int vx)(as_int vy)))
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
ERASED_ARG(#vx #vy : _)
ERASED_ARG(#px #py : _)
REQUIRES(x |->Frac px vx)
REQUIRES(y |->Frac py vy)
RETURNS(n : int32)
ENSURES(x |->Frac px vx)
ENSURES(y |->Frac py vy)
ENSURES(pure(as_int n == max_spec(as_int vx)(as_int vy)))
int max_alt4(int *x, int *y) {
  if (*x > *y)
    return *x;
  else
    return *y;
}
