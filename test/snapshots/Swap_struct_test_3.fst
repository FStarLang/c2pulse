module Swap_struct_test_3

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



noeq
type u32_pair = {
first: ref UInt32.t;
second: ref UInt32.t;
}

[@@erasable]
noeq
type u32_pair_spec = {
first : UInt32.t;
second : UInt32.t

}

[@@pulse_unfold]
let u32_pair_pred ([@@@mkey]x:ref u32_pair) (s:u32_pair_spec) : slprop =
exists* (y: u32_pair). (x |-> y) **
(y.first |-> s.first) **
(y.second |-> s.second)

assume val u32_pair_spec_default : u32_pair_spec

assume val u32_pair_default (u32_pair_var_spec:u32_pair_spec) : u32_pair

ghost
fn u32_pair_pack (u32_pair_var:ref u32_pair) (#u32_pair_var_spec:u32_pair_spec)
requires u32_pair_var|-> u32_pair_default u32_pair_var_spec
ensures exists* v. u32_pair_pred u32_pair_var v ** pure (v == u32_pair_var_spec)
{ admit() }

ghost
fn u32_pair_unpack (u32_pair_var:ref u32_pair)
requires exists* v. u32_pair_pred u32_pair_var v 
ensures exists* u. u32_pair_var |-> u
{ admit() }

fn u32_pair_alloc ()
returns x:ref u32_pair
ensures freeable x
ensures exists* v. u32_pair_pred x v
{ admit () }

fn u32_pair_free (x:ref u32_pair)
requires freeable x
requires exists* v. u32_pair_pred x v
{ admit() }


ghost fn u32_pair_explode (x:ref u32_pair) (#s:u32_pair_spec)
requires u32_pair_pred x s
ensures exists* (v: u32_pair). (x |-> v) ** (v.first |-> s.first) ** 
(v.second |-> s.second)

{unfold u32_pair_pred}


ghost
fn u32_pair_recover (x:ref u32_pair) (#a0 : UInt32.t)
(#a1 : UInt32.t)

requires exists* (y: u32_pair). (x |-> y) ** 
(y.first |-> a0) **
(y.second |-> a1)
ensures exists* w. u32_pair_pred x w ** pure (w == {first = a0;
second = a1})
{fold u32_pair_pred x ({first = a0;
second = a1}) }

fn new_u32_pair ()
returns x:_
ensures freeable x
ensures u32_pair_pred x ({ first = 0ul; second = 1ul })
{
let mut x : (ref u32_pair) = u32_pair_alloc ();
Mku32_pair?.first (! (! x)) := (uint64_to_uint32 0UL);
Mku32_pair?.second (! (! x)) := (uint64_to_uint32 1UL);
(! x);
}

fn swap_fields
(x : ( ref u32_pair) )
(#s : _)
requires u32_pair_pred x s
ensures u32_pair_pred x ({first = s.second; second = s.first})
{
let mut x : (ref u32_pair) = x;
let mut f1 : UInt32.t = (! (! (! x)).first);
Mku32_pair?.first (! (! x)) := (! (! (! x)).second);
Mku32_pair?.second (! (! x)) := (! f1);
}

fn swap_refs
(x : ( ref UInt32.t) )
(y : ( ref UInt32.t) )
(#vx #vy:erased _)
requires x |-> vx
requires y |-> vy
ensures x |-> vy
ensures y |-> vx
{
let mut x : (ref UInt32.t) = x;
let mut y : (ref UInt32.t) = y;
let mut tmp : UInt32.t = (! (! x));
(! x) := (! (! y));
(! y) := (! tmp);
}

fn swap_fields_alt
(x : ( ref u32_pair) )
(#s : erased _)
requires u32_pair_pred x s
ensures u32_pair_pred x ({first = s.second; second = s.first})
{
let mut x : (ref u32_pair) = x;
(swap_refs (! (! x)).first (! (! x)).second);
}

fn main ()
returns Int32.t
{
let mut x : (ref u32_pair) = (new_u32_pair ());
(swap_fields (! x));
(swap_fields_alt (! x));
with vx. assert ((x |-> vx) ** u32_pair_pred vx {first = 0ul; second = 1ul});
(u32_pair_free (! x));
0l;
}
