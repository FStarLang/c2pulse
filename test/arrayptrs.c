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

typedef int *int_arrptr _arrayptr;

int_arrptr binary_search(int_arrptr lo, int_arrptr hi, int target)
  _preserves((_slprop) _inline_pulse(pts_to_mask 'arr #'p_arr 'v_arr 'm_arr))
  _preserves((_slprop) _inline_pulse(
    let arr = 'arr in let v_arr = 'v_arr in let m_arr = 'm_arr in // '
    arrayptr_pts_to $(lo) arr ** arrayptr_pts_to $(hi) arr ** pure (
      offset_of arr <= offset_of $(lo) /\ offset_of $(lo) <= offset_of $(hi) /\ offset_of $(hi) <= offset_of arr + Seq.length v_arr /\ (
      forall i. ~(offset_of $(lo) <= i /\ i < offset_of $(hi)) \/
      (reveal m_arr (i - offset_of arr) /\ Some? (Seq.index v_arr (i - offset_of arr)))))))
{
  _ghost_stmt(with arr p_arr v_arr m_arr. assert pts_to_mask arr #p_arr v_arr m_arr);
  while (lo < hi)
    _invariant(_live(lo))
    _invariant(_live(hi))
    _invariant((_slprop) _inline_pulse(arrayptr_pts_to $(lo) arr))
    _invariant((_slprop) _inline_pulse(arrayptr_pts_to $(hi) arr))
    _invariant((bool) _inline_pulse(offset_of $(lo) <= offset_of $(hi)))
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