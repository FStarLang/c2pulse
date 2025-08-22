module Swap_struct_test_4

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



noeq
type u32_pair_struct = {
first: ref UInt32.t;
second: ref UInt32.t;
}

[@@erasable]
noeq
type u32_pair_struct_spec = {
first : UInt32.t;
second : UInt32.t

}

[@@pulse_unfold]
let u32_pair_struct_pred ([@@@mkey]x:ref u32_pair_struct) (s:u32_pair_struct_spec) : slprop =
exists* (y: u32_pair_struct). (x |-> y) **
(y.first |-> s.first) **
(y.second |-> s.second)

assume val u32_pair_struct_spec_default : u32_pair_struct_spec

assume val u32_pair_struct_default (u32_pair_struct_var_spec:u32_pair_struct_spec) : u32_pair_struct

ghost
fn u32_pair_struct_pack (u32_pair_struct_var:ref u32_pair_struct) (#u32_pair_struct_var_spec:u32_pair_struct_spec)
requires u32_pair_struct_var|-> u32_pair_struct_default u32_pair_struct_var_spec
ensures exists* v. u32_pair_struct_pred u32_pair_struct_var v ** pure (v == u32_pair_struct_var_spec)
{ admit() }

ghost
fn u32_pair_struct_unpack (u32_pair_struct_var:ref u32_pair_struct)
requires exists* v. u32_pair_struct_pred u32_pair_struct_var v 
ensures exists* u. u32_pair_struct_var |-> u
{ admit() }

fn u32_pair_struct_alloc ()
returns x:ref u32_pair_struct
ensures freeable x
ensures exists* v. u32_pair_struct_pred x v
{ admit () }

fn u32_pair_struct_free (x:ref u32_pair_struct)
requires freeable x
requires exists* v. u32_pair_struct_pred x v
{ admit() }


ghost fn u32_pair_struct_explode (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
requires u32_pair_struct_pred x s
ensures exists* (v: u32_pair_struct). (x |-> v) ** (v.first |-> s.first) ** 
(v.second |-> s.second)

{unfold u32_pair_struct_pred}


ghost
fn u32_pair_struct_recover (x:ref u32_pair_struct) (#a0 : UInt32.t)
(#a1 : UInt32.t)

requires exists* (y: u32_pair_struct). (x |-> y) ** 
(y.first |-> a0) **
(y.second |-> a1)
ensures exists* w. u32_pair_struct_pred x w ** pure (w == {first = a0;
second = a1})
{fold u32_pair_struct_pred x ({first = a0;
second = a1}) }

fn new_u32_pair_struct ()
requires 
emp
returns x:ref u32_pair_struct
ensures 
freeable x
ensures 
(u32_pair_struct_pred x { first = 0ul; second = 1ul })
{
let mut x : (ref u32_pair_struct) = u32_pair_struct_alloc ();
u32_pair_struct_explode !x;
Mku32_pair_struct?.first (! (! x)) := (uint64_to_uint32 0UL);
Mku32_pair_struct?.second (! (! x)) := (uint64_to_uint32 1UL);
u32_pair_struct_recover !x;
(! x);
}

fn swap_fields
(x : ( ref u32_pair_struct) )
(#s : u32_pair_struct_spec)
requires 
u32_pair_struct_pred x s
ensures 
exists* (s2:u32_pair_struct_spec). u32_pair_struct_pred x s2 ** pure (s2 == ({first = s.second; second = s.first}))
{
let mut x : (ref u32_pair_struct) = x;
u32_pair_struct_explode !x;
let mut f1 : UInt32.t = (! (! (! x)).first);
Mku32_pair_struct?.first (! (! x)) := (! (! (! x)).second);
Mku32_pair_struct?.second (! (! x)) := (! f1);
u32_pair_struct_recover !x;
}

fn swap_refs
(x : ( ref UInt32.t) )
(y : ( ref UInt32.t) )
preserves 
live x
preserves 
live y
ensures 
rewrites_to (!x) (old(!y))
ensures 
rewrites_to (!y) (old(!x))
{
let mut x : (ref UInt32.t) = x;
let mut y : (ref UInt32.t) = y;
let mut tmp : UInt32.t = (! (! x));
(! x) := (! (! y));
(! y) := (! tmp);
}

fn swap_fields_alt
(x : ( ref u32_pair_struct) )
(#s : u32_pair_struct_spec)
requires 
u32_pair_struct_pred x s
ensures 
exists* (s2:u32_pair_struct_spec). u32_pair_struct_pred x s2 ** pure (s2 == {first = s.second; second = s.first})
{
let mut x : (ref u32_pair_struct) = x;
u32_pair_struct_explode !x;
(swap_refs (! (! x)).first (! (! x)).second);
u32_pair_struct_recover !x;
}

fn main ()
returns Int32.t
{
let mut x : (ref u32_pair_struct) = (new_u32_pair_struct ());
(swap_fields (! x));
(swap_fields_alt (! x));
with vx. assert ((x |-> vx) ** u32_pair_struct_pred vx {first = 0ul; second = 1ul});
(u32_pair_struct_free (! x));
0l;
}
