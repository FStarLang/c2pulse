#include <stdint.h>
#include <stdlib.h>
#include "../c2pulse.h"

_include_pulse(
  let pts_to_diag 
        (#a:_)
        (r0 r1:ref a)
        (v:a)
  : slprop
  = (r0 |-> v) ** (r1 |-> v)
)

_requires((_slprop) _inline_pulse(pts_to_diag r0 r1 v))
_ensures((_slprop) _inline_pulse(exists* w. pts_to_diag r0 r1 w ** pure (as_int w = 2 * as_int v)))
void double_int(int *r0, int *r1)
{
    _assert((_slprop) _inline_pulse(with vr0. assert (r0 |-> vr0)));
    _assert((_slprop) _inline_pulse(with vr1. assert (r1 |-> vr1)));
    _assert((_slprop) _inline_pulse(unfold (pts_to_diag vr0 vr1 v)));
    *r0 = *r0 + *r1;
    *r1 = *r0;
    _assert((_slprop) _inline_pulse(fold (pts_to_diag vr0 vr1)));
}

typedef struct _point {
  int px;
  int py;
} point;

_include_pulse(
  let is_point (p:ref point) (xy : (int & int))
  : slprop
  = exists* v. point_pred p v ** pure (as_int v.px == fst xy) ** pure (as_int v.py == snd xy)
)

_requires((_slprop) _inline_pulse(is_point p v))
_requires((_slprop) _inline_pulse(pure <| fits (+) (fst v) (as_int dx)))
_requires((_slprop) _inline_pulse(pure <| fits (+) (snd v) (as_int dy)))
_ensures((_slprop) _inline_pulse(is_point p (fst v + as_int dx, snd v + as_int dy)))
void move(point *p, int dx, int dy)
{
  _assert((_slprop) _inline_pulse(with vp. assert (p |-> vp)));
  _assert((_slprop) _inline_pulse(with vdx. assert (dx |-> vdx)));
  _assert((_slprop) _inline_pulse(with vdy. assert (dy |-> vdy)));
  _assert((_slprop) _inline_pulse(unfold(is_point); point_explode vp));
  p->px = p->px + dx;
  p->py = p->py + dy;
  _assert((_slprop) _inline_pulse(point_recover vp; fold (is_point vp (fst v + as_int vdx, snd v + as_int vdy))));
}

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
  _assert((_slprop) _inline_pulse(with vp. assert (p |-> vp)));
  _assert((_slprop) _inline_pulse(unfold(is_point); point_explode vp));
  p->px = p->px + dx;
  p->py = p->py + dy;
  _assert((_slprop) _inline_pulse(point_recover vp; fold_is_point vp));
}

_ensures((_slprop) _inline_pulse(is_point p (as_int x, as_int y)))
_ensures((_slprop) _inline_pulse(freeable p))
point* create_point(int x, int y)
{
  point* p = (point*)malloc(sizeof(point));
  _assert((_slprop) _inline_pulse(with vp. assert (p |-> vp)));
  _assert((_slprop) _inline_pulse(point_explode vp));
  p->px = x;
  p->py = y;
  _assert((_slprop) _inline_pulse(point_recover vp; fold_is_point vp));
  return p;
}

void create_and_move()
{
  point *p = create_point(0, 0);
  move_alt(p, 1, 1);
  _assert((_slprop) _inline_pulse(with vp. assert (p |-> vp)));
  _assert((_slprop) _inline_pulse(is_point vp (1, 1)));
  _assert((_slprop) _inline_pulse(unfold is_point));
  free(p);
}

_include_pulse( 
  let is_point_curry (p:ref point) (x y : int)
  : slprop
  = exists* v. point_pred p v ** pure (as_int v.px == x) ** pure (as_int v.py == y)
)

_requires((_slprop) _inline_pulse(is_point_curry p x y))
_requires((_slprop) _inline_pulse(pure <| fits (+) x (as_int dx)))
_requires((_slprop) _inline_pulse(pure <| fits (+) y (as_int dy)))
_ensures((_slprop) _inline_pulse(is_point_curry p (x + as_int dx) (y + as_int dy)))
void move_curry(point *p, int dx, int dy)
{
  _assert((_slprop) _inline_pulse(with vp. assert (p |-> vp)));
  _assert((_slprop) _inline_pulse(unfold is_point_curry; point_explode vp));
  p->px = p->px + dx;
  p->py = p->py + dy;
  _assert((_slprop) _inline_pulse(point_recover vp; fold is_point_curry));
}
