#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

typedef struct _point {
  int px;
  int py;
} point;

_include_pulse(
  let is_point (p:ref point) (xy : (int & int))
  : slprop
  = exists* v. point_pred p v ** pure (as_int v.px == fst xy) ** pure (as_int v.py == snd xy)
)

_include_pulse(
  ghost
  fn fold_is_point (p:ref point) (#s:point_spec)
  requires point_pred p s
  ensures exists* v. is_point p v ** pure (v == (as_int s.px, as_int s.py))
  {
    fold (is_point p (as_int s.px, as_int s.py));
  }
)

_requires((_slprop) _inline_pulse(is_point p v))
_requires((_slprop) _inline_pulse(pure <| fits (+) (fst v) (as_int dx)))
_requires((_slprop) _inline_pulse(pure <| fits (+) (snd v) (as_int dy)))
_ensures((_slprop) _inline_pulse(is_point p (fst v + as_int dx, snd v + as_int dy)))
void move_alt(point *p, int dx, int dy)
{
  _assert((_slprop) _inline_pulse(unfold(is_point); point_explode !p));
  p->px = p->px + dx;
  p->py = p->py + dy;
  _assert((_slprop) _inline_pulse(point_recover !p; fold_is_point !p));
}

_ensures((_slprop) _inline_pulse(is_point p (as_int x, as_int y)))
_ensures((_slprop) _inline_pulse(freeable p))
point* create_point(int x, int y)
{
  point* p = (point*)malloc(sizeof(point));
  _assert((_slprop) _inline_pulse(point_explode !p));
  p->px = x;
  p->py = y;
  _assert((_slprop) _inline_pulse(point_recover !p; fold_is_point !p));
  return p;
}

void create_and_move()
{
  point *p = create_point(0, 0);
  move_alt(p, 1, 1);
  _assert((_slprop) _inline_pulse(unfold(is_point); point_explode !p; point_recover !p));
  free(p);
}