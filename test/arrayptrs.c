#include "c2pulse.h"
#include <stdlib.h>

// Create arrayptr via pointer arithmetic, then write through it
void write_via_ptr(_array int *a)
  _requires(a._length == 10)
  _preserves_value(a._length)
{
  _arrayptr int *p = a + 3;
  *p = 42;
  _ghost_stmt(arrayptr_drop (!var_p));
}

_include_pulse(
  let maybe_arrayptr_pts_to #a (x: array a) (y: array a) : slprop =
    if Pulse.Lib.Array.is_null x then emp else arrayptr_pts_to x y

  [@@pulse_intro]
  ghost fn intro_maybe_arrayptr_pts_to_null (#a: Type0) (y: array a)
    ensures maybe_arrayptr_pts_to Pulse.Lib.Array.null y
  {
    rewrite emp as maybe_arrayptr_pts_to Pulse.Lib.Array.null y
  }

  [@@pulse_intro]
  ghost fn intro_maybe_arrayptr_pts_to_nonnull (#a: Type0) (x y: array a)
    requires arrayptr_pts_to x y
    ensures maybe_arrayptr_pts_to x y
  {
    if Pulse.Lib.Array.is_null x {
      drop_ (arrayptr_pts_to x y);
      assert rewrites_to x Pulse.Lib.Array.null;
    } else {
      rewrite arrayptr_pts_to x y as maybe_arrayptr_pts_to x y;
    }
  }
)

_include_pulse(
  let is_slice_prop #a (lo hi: array a) (x: array a) (v: Seq.seq (option a)) (m: nat->prop) =
    offset_of x <= offset_of lo
      /\ offset_of lo <= offset_of hi
      /\ offset_of hi <= offset_of x + Seq.length v
      /\ (forall i. offset_of lo <= i /\ i < offset_of hi ==>
        (m (i - offset_of x) /\ Some? (Seq.index v (i - offset_of x))))

  [@@pulse_eager_unfold]
  let is_slice #a (lo hi: array a) (x: array a) p v m =
    pts_to_mask x #p v m ** 
    arrayptr_pts_to lo x ** arrayptr_pts_to hi x **
    pure (is_slice_prop lo hi x v m)
)

_arrayptr int *binary_search(_arrayptr int *lo, _arrayptr int *hi, int target)
  _preserves((_slprop) _inline_pulse(is_slice $(lo) $(hi) $`arr $`p_arr $`v_arr $`m_arr))
  _requires((bool) _inline_pulse(offset_of $(hi) - offset_of $(lo) < 100000))
  _ensures((_slprop) _inline_pulse(maybe_arrayptr_pts_to $(return) (arrayptr_parent $(lo))))
{
  while (lo < hi)
    _invariant((_slprop) _inline_pulse(live $&(lo)))
    _invariant((_slprop) _inline_pulse(live $&(hi)))
    _invariant((_slprop) _inline_pulse(arrayptr_pts_to $(lo) (old <| arrayptr_parent $(lo))))
    _invariant((_slprop) _inline_pulse(arrayptr_pts_to $(hi) (old <| arrayptr_parent $(hi))))
    _invariant((bool) _inline_pulse(offset_of $(lo) <= offset_of $(hi)))
    _invariant((bool) _inline_pulse(old (offset_of $(lo)) <= offset_of $(lo) && offset_of $(hi) <= old (offset_of $(hi))))
  {
      int *mid = lo + (hi - lo) / 2;
      if (*mid == target)
        return mid;
      else if (*mid < target)
        lo = mid + 1;
      else
        hi = mid;
  }
  return NULL;
}