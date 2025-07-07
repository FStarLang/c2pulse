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
    LEMMA(unfold (pts_to_diag r0 r1 v));
    *r0 = *r0 + *r1;
    *r1 = *r0;
    LEMMA(fold (pts_to_diag r0 r1));
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
  LEMMA(unfold(is_point); point_explode p);
  p->px = p->px + dx;
  p->py = p->py + dy;
  LEMMA(point_recover p; fold (is_point p (fst v + as_int dx, snd v + as_int dy)));
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
  LEMMA(unfold(is_point); point_explode p);
  p->px = p->px + dx;
  p->py = p->py + dy;
  LEMMA(point_recover p; fold_is_point p);
}


RETURNS(p:ref point)
ENSURES(is_point p (as_int x, as_int y))
ENSURES(freeable p)
point* create_point(int x, int y)
{
  point* p = (point*)malloc(sizeof(point));
  LEMMA(point_explode p);
  p->px = x;
  p->py = y;
  LEMMA(point_recover p; fold_is_point p);
  return p;
}

// infinite loop
// void create_and_move()
// {
//   point *p = create_point(0, 0);
//   move_alt(p, 1, 1);
//   // ASSERT(is_point p (1, 1));
//   // free(p);
// }

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
  LEMMA(unfold is_point_curry; point_explode p);
  p->px = p->px + dx;
  p->py = p->py + dy;
  LEMMA(point_recover p; fold is_point_curry);
}