module Heap_allocated_array

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



noeq
type foo_struct = {
a: ref Int32.t;
b: ref Int32.t;
}

[@@erasable]
noeq
type foo_struct_spec = {
a : Int32.t;
b : Int32.t

}

[@@pulse_unfold]
let foo_struct_pred ([@@@mkey]x:ref foo_struct) (s:foo_struct_spec) : slprop =
exists* (y: foo_struct). (x |-> y) **
(y.a |-> s.a) **
(y.b |-> s.b)

assume val foo_struct_spec_default : foo_struct_spec

assume val foo_struct_default (foo_struct_var_spec:foo_struct_spec) : foo_struct

ghost
fn foo_struct_pack (foo_struct_var:ref foo_struct) (#foo_struct_var_spec:foo_struct_spec)
requires foo_struct_var|-> foo_struct_default foo_struct_var_spec
ensures exists* v. foo_struct_pred foo_struct_var v ** pure (v == foo_struct_var_spec)
{ admit() }

ghost
fn foo_struct_unpack (foo_struct_var:ref foo_struct)
requires exists* v. foo_struct_pred foo_struct_var v 
ensures exists* u. foo_struct_var |-> u
{ admit() }

fn foo_struct_alloc ()
returns x:ref foo_struct
ensures freeable x
ensures exists* v. foo_struct_pred x v
{ admit () }

fn foo_struct_free (x:ref foo_struct)
requires freeable x
requires exists* v. foo_struct_pred x v
{ admit() }


ghost fn foo_struct_explode (x:ref foo_struct) (#s:foo_struct_spec)
requires foo_struct_pred x s
ensures exists* (v: foo_struct). (x |-> v) ** (v.a |-> s.a) ** 
(v.b |-> s.b)

{unfold foo_struct_pred}


ghost
fn foo_struct_recover (x:ref foo_struct) (#a0 : Int32.t)
(#a1 : Int32.t)

requires exists* (y: foo_struct). (x |-> y) ** 
(y.a |-> a0) **
(y.b |-> a1)
ensures exists* w. foo_struct_pred x w ** pure (w == {a = a0;
b = a1})
{fold foo_struct_pred x ({a = a0;
b = a1}) }

fn foo
(length : SizeT.t)
requires 
pure (SizeT.v length < 100)
returns Int32.t
{
let mut length : SizeT.t = length;
let mut b0 : (ref Int32.t) = alloc_ref #Int32.t ();
let mut b1 : (ref Int32.t) = alloc_ref #Int32.t ();
let mut size_expr0 : SizeT.t = (! length);
let mut b : (array Int32.t) = alloc_array #Int32.t !size_expr0;
let mut length2 : Int32.t = 100l;
let mut size_expr1 : SizeT.t = (uint64_to_sizet (int32_to_uint64 (! length2)));
let mut bb : (array Int32.t) = alloc_array #Int32.t !size_expr1;
(free_ref (! b0));
(free_ref (! b1));
(free_array #Int32.t (! b));
(free_array #Int32.t (! bb));
(sizet_to_int32 (! length));
}
