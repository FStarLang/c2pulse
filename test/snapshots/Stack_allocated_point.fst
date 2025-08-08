module Stack_allocated_point

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



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

[@@pulse_unfold]
let point_pred ([@@@mkey]x:ref point) (s:point_spec) : slprop =
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

fn test_stack_allocator ()
requires emp
ensures emp
{
let mut p : point = point_default point_spec_default;
point_pack p;
point_explode p;
(! p).px := 1l;
(! p).py := 2l;
point_recover p;
point_unpack p;
}
