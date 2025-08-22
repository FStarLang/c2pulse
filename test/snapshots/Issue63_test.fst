module Issue63_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



noeq
type s1 = {
bytes: ref (array UInt8.t);
}

[@@erasable]
noeq
type s1_spec = {
bytes : array UInt8.t

}

let s1_relations (s:s1_spec) : prop = 
Pulse.Lib.Array.length s.bytes == 32

[@@pulse_unfold]
let s1_pred ([@@@mkey]x:ref s1) (s:s1_spec) : slprop =
exists* (y: s1). (x |-> y) **
(y.bytes |-> s.bytes)
** pure (s1_relations s)

assume val s1_spec_default : s1_spec

assume val s1_default (s1_var_spec:s1_spec) : s1

ghost
fn s1_pack (s1_var:ref s1) (#s1_var_spec:s1_spec)
requires s1_var|-> s1_default s1_var_spec
ensures exists* v. s1_pred s1_var v ** pure (v == s1_var_spec)
{ admit() }

ghost
fn s1_unpack (s1_var:ref s1)
requires exists* v. s1_pred s1_var v 
ensures exists* u. s1_var |-> u
{ admit() }

fn s1_alloc ()
returns x:ref s1
ensures freeable x
ensures exists* v. s1_pred x v
{ admit () }

fn s1_free (x:ref s1)
requires freeable x
requires exists* v. s1_pred x v
{ admit() }


ghost fn s1_explode (x:ref s1) (#s:s1_spec)
requires s1_pred x s
ensures exists* (v: s1). (x |-> v) ** (v.bytes |-> s.bytes)

** pure (s1_relations s)
{unfold s1_pred}


ghost
fn s1_recover (x:ref s1) (#a0 : (array UInt8.t))

requires exists* (y: s1). (x |-> y) ** 
(y.bytes |-> a0)** pure (s1_relations {
bytes = a0})

ensures exists* w. s1_pred x w ** pure (w == {bytes = a0})
{fold s1_pred x ({bytes = a0}) }

fn read_s10
(x1 : ( ref s1) )
preserves 
exists* s1_s y. s1_pred x1 s1_s ** (s1_s.bytes |-> y)
returns UInt8.t
{
let mut x1 : (ref s1) = x1;
s1_explode (!x1);
pts_to_len (!(!(!x1)).bytes);
let mut res : UInt8.t = (op_Array_Access (! (! (! x1)).bytes) (int32_to_sizet 0l));
admit();
(int32_to_uint8 0l);
}
