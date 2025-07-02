#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

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


INCLUDE ("
  let is_diag_point (p:ref point) (v:int32)
  : slprop
  = point_pred p {px=v; py=v}

  let is_point (p:ref point) (xy : (int & int))
  : slprop
  = exists* v. point_pred p v ** pure (as_int v.px == fst xy) ** pure (as_int v.py == snd xy)
"
)

ERASED_ARG(#v:erased _)
REQUIRES(is_point p v)
REQUIRES(fits (+) (fst v) (as_int dx))
REQUIRES(fits (+) (snd v) (as_int dy))
ENSURES(is_point p (fst v + as_int dx, snd v + as_int dy))
void move(point *p, int dx, int dy)
{
  LEMMA(unfold(is_point));
  p->px = p->px + dx;
  p->py = p->py + dy;
  LEMMA(fold(is_point));
}