module Pulse_user_defined_predicates1

#lang-pulse

open Pulse
open Pulse.Lib.C



let pts_to_diag (#a:_) (r0 r1:ref a) (v:a) : slprop = (r0 |-> v) ** (r1 |-> v)
fn double_int
(r0 : ref Int32.t)
(r1 : ref Int32.t)
(#v:erased _ { fits ( * ) 2 (as_int v) })
requires pts_to_diag r0 r1 v
ensures exists* w. pts_to_diag r0 r1 w ** pure (as_int w = 2 * as_int v)
{
let mut r0 : (ref Int32.t) = r0;
let mut r1 : (ref Int32.t) = r1;
with vr0. assert (r0 |-> vr0);
with vr1. assert (r1 |-> vr1);
unfold (pts_to_diag vr0 vr1 v);
(! r0) := (Int32.add (! (! r0)) (! (! r1)));
(! r1) := (! (! r0));
fold (pts_to_diag vr0 vr1);
}

noeq
type point = {
px: ref Int32.t;
py: ref Int32.t;
}

[@@erasable]
noeq
type point_spec = {
px : Int32.t;
py : Int32.t

}

let point_pred (x:ref point) (s:point_spec) : slprop =
exists* (y: point). (x |-> y) **
(y.px |-> s.px) **
(y.py |-> s.py)

assume val point_spec_default : point_spec

assume val point_default (point_var_spec:point_spec) : point

ghost
fn point_pack (point_var:ref point) (#point_var_spec:point_spec)
requires point_var|-> point_default point_var_spec
ensures exists* v. point_pred point_var v ** pure (v == point_var_spec)
{ admit() }

ghost
fn point_unpack (point_var:ref point)
requires exists* v. point_pred point_var v 
ensures exists* u. point_var |-> u
{ admit() }

fn point_alloc ()
returns x:ref point
ensures freeable x
ensures exists* v. point_pred x v
{ admit () }

fn point_free (x:ref point)
requires freeable x
requires exists* v. point_pred x v
{ admit() }


ghost fn point_explode (x:ref point) (#s:point_spec)
requires point_pred x s
ensures exists* (v: point). (x |-> v) ** (v.px |-> s.px) ** 
(v.py |-> s.py)

{unfold point_pred}


ghost
fn point_recover (x:ref point) (#a0 : Int32.t)
(#a1 : Int32.t)

requires exists* (y: point). (x |-> y) ** 
(y.px |-> a0) **
(y.py |-> a1)
ensures exists* w. point_pred x w ** pure (w == {px = a0;
py = a1})
{fold point_pred x ({px = a0;
py = a1}) }

let is_point (p:ref point) (xy : (int & int)) : slprop = exists* v. point_pred p v ** pure (as_int v.px == fst xy) ** pure (as_int v.py == snd xy)
fn move
(p : ref point)
(dx : Int32.t)
(dy : Int32.t)
(#v:erased _)
requires is_point p v
requires pure <| fits (+) (fst v) (as_int dx)
requires pure <| fits (+) (snd v) (as_int dy)
ensures is_point p (fst v + as_int dx, snd v + as_int dy)
{
let mut p : (ref point) = p;
let mut dx : Int32.t = dx;
let mut dy : Int32.t = dy;
with vp. assert (p |-> vp);
with vdx. assert (dx |-> vdx);
with vdy. assert (dy |-> vdy);
unfold(is_point); point_explode vp;
Mkpoint?.px (! (! p)) := (Int32.add (! (! (! p)).px) (! dx));
Mkpoint?.py (! (! p)) := (Int32.add (! (! (! p)).py) (! dy));
point_recover vp; fold (is_point vp (fst v + as_int vdx, snd v + as_int vdy));
}

ghost fn fold_is_point (p:ref point) (#s:point_spec) requires point_pred p s ensures exists* v. is_point p v ** pure (v == (as_int s.px, as_int s.py)) { fold (is_point p (as_int s.px, as_int s.py)); }
fn move_alt
(p : ref point)
(dx : Int32.t)
(dy : Int32.t)
(#v:erased _)
requires is_point p v
requires pure <| fits (+) (fst v) (as_int dx)
requires pure <| fits (+) (snd v) (as_int dy)
ensures is_point p (fst v + as_int dx, snd v + as_int dy)
{
let mut p : (ref point) = p;
let mut dx : Int32.t = dx;
let mut dy : Int32.t = dy;
with vp. assert (p |-> vp);
unfold(is_point); point_explode vp;
Mkpoint?.px (! (! p)) := (Int32.add (! (! (! p)).px) (! dx));
Mkpoint?.py (! (! p)) := (Int32.add (! (! (! p)).py) (! dy));
point_recover vp; fold_is_point vp;
}

fn create_point
(x : Int32.t)
(y : Int32.t)
returns p:ref point
ensures is_point p (as_int x, as_int y)
ensures freeable p
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
let mut p : (ref point) = point_alloc ();
with vp. assert (p |-> vp);
point_explode vp;
Mkpoint?.px (! (! p)) := (! x);
Mkpoint?.py (! (! p)) := (! y);
point_recover vp; fold_is_point vp;
(! p);
}

fn create_and_move ()
{
let mut p : (ref point) = (create_point 0l 0l);
(move_alt (! p) 1l 1l);
with vp. assert (p |-> vp);
assert(is_point vp (1, 1));
unfold is_point;
(point_free (! p));
}

let is_point_curry (p:ref point) (x y : int) : slprop = exists* v. point_pred p v ** pure (as_int v.px == x) ** pure (as_int v.py == y)
fn move_curry
(p : ref point)
(dx : Int32.t)
(dy : Int32.t)
(#x #y:erased _)
requires is_point_curry p x y
requires pure <| fits (+) x (as_int dx)
requires pure <| fits (+) y (as_int dy)
ensures is_point_curry p (x + as_int dx) (y + as_int dy)
{
let mut p : (ref point) = p;
let mut dx : Int32.t = dx;
let mut dy : Int32.t = dy;
with vp. assert (p |-> vp);
unfold is_point_curry; point_explode vp;
Mkpoint?.px (! (! p)) := (Int32.add (! (! (! p)).px) (! dx));
Mkpoint?.py (! (! p)) := (Int32.add (! (! (! p)).py) (! dy));
point_recover vp; fold is_point_curry;
}
