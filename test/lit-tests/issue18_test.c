// RUN: %c2pulse %s
// RUN: cat %p/Issue18_test.fst
// RUN: diff %p/Issue18_test.fst %p/../snapshots/Issue18_test.fst
// RUN: %run_fstar.sh %p/Issue18_test.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE
#include <stdint.h>
#include <stdlib.h>
#include "../../include/PulseMacros.h"

REQUIRES(emp)
RETURNS(i:int32)
ENSURES(emp)
int test(void)
{
  return 0;
}

typedef struct _point {
  int px;
  int py;
} point;


INCLUDE (
  let is_diag_point (p:ref point) (v:int32)
  : slprop
  = point_pred p {px=v; py=v}

  let is_point (p:ref point) (xy : (int & int))
  : slprop
  = exists* v. point_pred p v ** pure (as_int v.px == fst xy) ** pure (as_int v.py == snd xy)

  ghost
  fn fold_is_point (p:ref point) (#s:point_spec)
  requires point_pred p s
  ensures exists* v. is_point p v ** pure (v == (as_int s.px, as_int s.py))
  {
    fold (is_point p (as_int s.px, as_int s.py));
  }

)

ERASED_ARG(#v:erased _)
REQUIRES(is_point p v)
REQUIRES(pure <| fits (+) (fst v) (as_int dx))
REQUIRES(pure <| fits (+) (snd v) (as_int dy))
ENSURES(is_point p (fst v + as_int dx, snd v + as_int dy))
void move(point *p, int dx, int dy)
{
  LEMMA(unfold(is_point); point_explode p);
  p->px = p->px + dx;
  p->py = p->py + dy;
  LEMMA(point_recover p; fold_is_point p);
}


// PULSE: All verification conditions discharged successfully
