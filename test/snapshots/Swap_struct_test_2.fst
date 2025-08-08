module Swap_struct_test_2

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



noeq
type _u32_pair_struct = {
first: ref UInt32.t;
second: ref UInt32.t;
}

[@@erasable]
noeq
type _u32_pair_struct_spec = {
first : UInt32.t;
second : UInt32.t

}

[@@pulse_unfold]
let _u32_pair_struct_pred ([@@@mkey]x:ref _u32_pair_struct) (s:_u32_pair_struct_spec) : slprop =
exists* (y: _u32_pair_struct). (x |-> y) **
(y.first |-> s.first) **
(y.second |-> s.second)

assume val _u32_pair_struct_spec_default : _u32_pair_struct_spec

assume val _u32_pair_struct_default (_u32_pair_struct_var_spec:_u32_pair_struct_spec) : _u32_pair_struct

ghost
fn _u32_pair_struct_pack (_u32_pair_struct_var:ref _u32_pair_struct) (#_u32_pair_struct_var_spec:_u32_pair_struct_spec)
requires _u32_pair_struct_var|-> _u32_pair_struct_default _u32_pair_struct_var_spec
ensures exists* v. _u32_pair_struct_pred _u32_pair_struct_var v ** pure (v == _u32_pair_struct_var_spec)
{ admit() }

ghost
fn _u32_pair_struct_unpack (_u32_pair_struct_var:ref _u32_pair_struct)
requires exists* v. _u32_pair_struct_pred _u32_pair_struct_var v 
ensures exists* u. _u32_pair_struct_var |-> u
{ admit() }

fn _u32_pair_struct_alloc ()
returns x:ref _u32_pair_struct
ensures freeable x
ensures exists* v. _u32_pair_struct_pred x v
{ admit () }

fn _u32_pair_struct_free (x:ref _u32_pair_struct)
requires freeable x
requires exists* v. _u32_pair_struct_pred x v
{ admit() }


ghost fn _u32_pair_struct_explode (x:ref _u32_pair_struct) (#s:_u32_pair_struct_spec)
requires _u32_pair_struct_pred x s
ensures exists* (v: _u32_pair_struct). (x |-> v) ** (v.first |-> s.first) ** 
(v.second |-> s.second)

{unfold _u32_pair_struct_pred}


ghost
fn _u32_pair_struct_recover (x:ref _u32_pair_struct) (#a0 : UInt32.t)
(#a1 : UInt32.t)

requires exists* (y: _u32_pair_struct). (x |-> y) ** 
(y.first |-> a0) **
(y.second |-> a1)
ensures exists* w. _u32_pair_struct_pred x w ** pure (w == {first = a0;
second = a1})
{fold _u32_pair_struct_pred x ({first = a0;
second = a1}) }

fn new_u32_pair_struct ()
requires emp
returns x:ref _u32_pair_struct
ensures freeable x
ensures (_u32_pair_struct_pred x { first = 0ul; second = 1ul })
{
let mut x : (ref _u32_pair_struct) = _u32_pair_struct_alloc ();
Mk_u32_pair_struct?.first (! (! x)) := (uint64_to_uint32 0UL);
Mk_u32_pair_struct?.second (! (! x)) := (uint64_to_uint32 1UL);
(! x);
}

fn swap_fields
(x : ( ref _u32_pair_struct) )
(#s : _u32_pair_struct_spec)
requires _u32_pair_struct_pred x s
ensures exists* (s':_u32_pair_struct_spec). _u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))
{
let mut x : (ref _u32_pair_struct) = x;
_u32_pair_struct_explode !x;
let mut f1 : UInt32.t = (! (! (! x)).first);
Mk_u32_pair_struct?.first (! (! x)) := (! (! (! x)).second);
Mk_u32_pair_struct?.second (! (! x)) := (! f1);
_u32_pair_struct_recover !x;
}

fn swap_refs
(x : ( ref UInt32.t) )
(y : ( ref UInt32.t) )
requires x |-> 'x
requires y |-> 'y
ensures x |-> 'y
ensures y |-> 'x
{
let mut x : (ref UInt32.t) = x;
let mut y : (ref UInt32.t) = y;
let mut tmp : UInt32.t = (! (! x));
(! x) := (! (! y));
(! y) := (! tmp);
}

fn swap_fields_alt
(x : ( ref _u32_pair_struct) )
(#s : _u32_pair_struct_spec)
requires _u32_pair_struct_pred x s
ensures exists* (s':_u32_pair_struct_spec). _u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first})
{
let mut x : (ref _u32_pair_struct) = x;
_u32_pair_struct_explode !x;
(swap_refs (! (! x)).first (! (! x)).second);
_u32_pair_struct_recover !x;
}

fn main ()
returns Int32.t
{
let mut x : (ref _u32_pair_struct) = (new_u32_pair_struct ());
(swap_fields (! x));
(swap_fields_alt (! x));
with vx. assert ((x |-> vx) ** _u32_pair_struct_pred vx {first = 0ul; second = 1ul});
(_u32_pair_struct_free (! x));
0l;
}
