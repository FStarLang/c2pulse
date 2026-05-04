#include "pal.h"
#include <stdlib.h>

// Create arrayptr via pointer arithmetic, then write through it
void write_via_ptr(_array int *a)
  _requires(a._length == 10)
  _preserves_value(a._length)
{
  _arrayptr int *p = a + 3;
  *p = 42;
  _ghost_stmt(arrayptr_drop $(p));
}

_include_pulse(
  let unless_null #a (x: array a) (p: slprop) : slprop =
    if Pulse.Lib.Array.is_null x then emp else p

  [@@pulse_intro]
  ghost fn intro_unless_null_null (#a: Type0) p
    ensures unless_null #a Pulse.Lib.Array.null p
  {
    rewrite emp as unless_null #a Pulse.Lib.Array.null p
  }

  [@@pulse_intro]
  ghost fn intro_unless_null_nonnull (#a: Type0) (x: array a) p
    requires p
    ensures unless_null x p
  {
    if Pulse.Lib.Array.is_null x {
      drop_ p;
      assert rewrites_to x Pulse.Lib.Array.null;
    } else {
      rewrite p as unless_null x p;
    }
  }

  ghost fn elim_unless_null_null (#a: Type0) (x: array a) p
    requires unless_null x p
    requires pure (Pulse.Lib.Array.is_null x)
  {
    rewrite unless_null x p as emp
  }
  ghost fn elim_unless_null_nonnull (#a: Type0) (x: array a) p
    requires unless_null x p
    requires pure (not (Pulse.Lib.Array.is_null x))
    ensures p
  {
    rewrite unless_null x p as p
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
  _ensures((_slprop) _inline_pulse(unless_null $(return)
    (arrayptr_pts_to $(return) (arrayptr_parent $(lo)) **
      pure (offset_of $(lo) <= offset_of $(return) /\ offset_of $(return) < offset_of $(hi)))))
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

void use_binary_search(_array int *arr, int target, size_t length)
  _requires(length == arr._length && length <= 10000)
{
  _arrayptr int *lo = arr;
  _arrayptr int *hi = arr + length;
  _arrayptr int *result = binary_search(lo, hi, target);
  if (result == NULL) {
    _ghost_stmt(elim_unless_null_null _ _);
  } else {
    _ghost_stmt(elim_unless_null_nonnull _ _);
    int val = *result;
  }
}
