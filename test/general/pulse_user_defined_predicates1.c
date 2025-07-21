#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

INCLUDE (
  let pts_to_diag 
        (#a:_)
        (r0 r1:ref a)
        (v:a)
  : slprop
  = (r0 |-> v) ** (r1 |-> v)
)

ERASED_ARG(#v:erased _ { fits ( * ) 2 (as_int v) })
REQUIRES(pts_to_diag r0 r1 v)
ENSURES(exists* w. pts_to_diag r0 r1 w ** pure (as_int w = 2 * as_int v))
void double_int(int *r0, int *r1)
{
    LEMMA(with vr0. assert (r0 |-> vr0));
    LEMMA(with vr1. assert (r1 |-> vr1));
    LEMMA(unfold (pts_to_diag vr0 vr1 v));
    *r0 = *r0 + *r1;
    *r1 = *r0;
    LEMMA(fold (pts_to_diag vr0 vr1));
}


typedef struct _point {
  int px;
  int py;
} point;


INCLUDE (
  let is_point (p:ref point) (xy : (int & int))
  : slprop
  = exists* v. point_pred p v ** pure (as_int v.px == fst xy) ** pure (as_int v.py == snd xy)
)

ERASED_ARG(#v:erased _)
REQUIRES(is_point p v)
REQUIRES(pure <| fits (+) (fst v) (as_int dx))
REQUIRES(pure <| fits (+) (snd v) (as_int dy))
ENSURES(is_point p (fst v + as_int dx, snd v + as_int dy))
void move(point *p, int dx, int dy)
{
  LEMMA(with vp. assert (p |-> vp));
  LEMMA(with vdx. assert (dx |-> vdx));
  LEMMA(with vdy. assert (dy |-> vdy));
  LEMMA(unfold(is_point); point_explode vp);
  p->px = p->px + dx;
  p->py = p->py + dy;
  LEMMA(point_recover vp; fold (is_point vp (fst v + as_int vdx, snd v + as_int vdy)));
}

INCLUDE (
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
void move_alt(point *p, int dx, int dy)
{
  LEMMA(with vp. assert (p |-> vp));
  LEMMA(unfold(is_point); point_explode vp);
  p->px = p->px + dx;
  p->py = p->py + dy;
  LEMMA(point_recover vp; fold_is_point vp);
}


RETURNS(p:ref point)
ENSURES(is_point p (as_int x, as_int y))
ENSURES(freeable p)
point* create_point(int x, int y)
{
  point* p = (point*)malloc(sizeof(point));
  LEMMA(with vp. assert (p |-> vp));
  LEMMA(point_explode vp);
  p->px = x;
  p->py = y;
  LEMMA(point_recover vp; fold_is_point vp);
  return p;
}

void create_and_move()
{
  point *p = create_point(0, 0);
  move_alt(p, 1, 1);
  LEMMA(with vp. assert (p |-> vp));
  ASSERT(is_point vp (1, 1));
  LEMMA(unfold is_point);
  free(p);
}

INCLUDE( 
  let is_point_curry (p:ref point) (x y : int)
  : slprop
  = exists* v. point_pred p v ** pure (as_int v.px == x) ** pure (as_int v.py == y)
)

ERASED_ARG(#x #y:erased _)
REQUIRES(is_point_curry p x y)
REQUIRES(pure <| fits (+) x (as_int dx))
REQUIRES(pure <| fits (+) y (as_int dy))
ENSURES(is_point_curry p (x + as_int dx) (y + as_int dy))
void move_curry(point *p, int dx, int dy)
{
  LEMMA(with vp. assert (p |-> vp));
  LEMMA(unfold is_point_curry; point_explode vp);
  p->px = p->px + dx;
  p->py = p->py + dy;
  LEMMA(point_recover vp; fold is_point_curry);
}
