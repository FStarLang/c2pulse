module Issue18_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn test ()
requires emp
returns i:int32
ensures emp
{
0l;
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
fn point_recover (x:ref point) (#a0 : Int32.t) (#a1 : Int32.t) 
requires exists* (y: point). (x |-> y) ** 
(y.px |-> a0) **
(y.py |-> a1)
ensures exists* w. point_pred x w ** pure (w == {px = a0;
py = a1})
{fold point_pred x ({px = a0;
py = a1}) }

let is_diag_point (p:ref point) (v:int32) : slprop = point_pred p {px=v; py=v} let is_point (p:ref point) (xy : (int & int)) : slprop = exists* v. point_pred p v ** pure (as_int v.px == fst xy) ** pure (as_int v.py == snd xy) ghost fn fold_is_point (p:ref point) (#s:point_spec) requires point_pred p s ensures exists* v. is_point p v ** pure (v == (as_int s.px, as_int s.py)) { fold (is_point p (as_int s.px, as_int s.py)); }
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
unfold(is_point); point_explode !p;
Mkpoint?.px (! (! p)) := (Int32.add (! (! (! p)).px) (! dx));
Mkpoint?.py (! (! p)) := (Int32.add (! (! (! p)).py) (! dy));
point_recover !p; fold_is_point !p;
}
