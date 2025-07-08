module Simple_struct_test

#lang-pulse

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

let u32_pair_struct_pred (x:ref u32_pair_struct) (s:u32_pair_struct_spec) : slprop =
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
ensures exists* (v: u32_pair_struct). (x |-> v) ** (v.first |-> s.first) ** (v.second |-> s.second)
{unfold u32_pair_struct_pred}


ghost
fn u32_pair_struct_recover (x:ref u32_pair_struct) (#a0 : UInt32.t) (#a1 : UInt32.t) 
requires exists* (y: u32_pair_struct). (x |-> y) ** (y.first |-> a0) ** (y.second |-> a1)
ensures exists* w. u32_pair_struct_pred x w ** pure (w == {first = a0; second = a1})
{fold u32_pair_struct_pred x ({first = a0; second = a1}) }
