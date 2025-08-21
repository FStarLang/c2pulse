module Nested_struct_with_arrays

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

noeq
type s2 = {
field_s1: ref (ref s1);
}

[@@erasable]
noeq
type s2_spec = {
field_s1 : ( ref s1) 

}

[@@pulse_unfold]
let s2_pred ([@@@mkey]x:ref s2) (s:s2_spec) : slprop =
exists* (y: s2). (x |-> y) **
(y.field_s1 |-> s.field_s1)

assume val s2_spec_default : s2_spec

assume val s2_default (s2_var_spec:s2_spec) : s2

ghost
fn s2_pack (s2_var:ref s2) (#s2_var_spec:s2_spec)
requires s2_var|-> s2_default s2_var_spec
ensures exists* v. s2_pred s2_var v ** pure (v == s2_var_spec)
{ admit() }

ghost
fn s2_unpack (s2_var:ref s2)
requires exists* v. s2_pred s2_var v 
ensures exists* u. s2_var |-> u
{ admit() }

fn s2_alloc ()
returns x:ref s2
ensures freeable x
ensures exists* v. s2_pred x v
{ admit () }

fn s2_free (x:ref s2)
requires freeable x
requires exists* v. s2_pred x v
{ admit() }


ghost fn s2_explode (x:ref s2) (#s:s2_spec)
requires s2_pred x s
ensures exists* (v: s2). (x |-> v) ** (v.field_s1 |-> s.field_s1)

{unfold s2_pred}


ghost
fn s2_recover (x:ref s2) (#a0 : (ref s1))

requires exists* (y: s2). (x |-> y) ** 
(y.field_s1 |-> a0)
ensures exists* w. s2_pred x w ** pure (w == {field_s1 = a0})
{fold s2_pred x ({field_s1 = a0}) }


[@@pulse_unfold] let owns_s2 (x:ref s2) (y:Seq.seq UInt8.t) =
exists* s2_spec s1_spec.
s2_pred x s2_spec **
s1_pred s2_spec.field_s1 s1_spec **
(s1_spec.bytes |-> y) **
freeable x **
freeable s2_spec.field_s1 ** freeable_array s1_spec.bytes
ghost fn intro_owns_s2 (#x:ref s2) (#y:ref s1) (#z:array UInt8.t) (#bytes:Seq.seq UInt8.t) ()
requires
s2_pred x ({field_s1=y}) **
s1_pred y ({bytes=z}) **
(z |-> bytes) **
freeable x **
freeable y **
freeable_array z
ensures owns_s2 x bytes { rewrite each z as ({bytes=z}).bytes }
fn test ()
{
let mut size_expr0 : SizeT.t = (uint64_to_sizet (int32_to_uint64 32l));
let mut bytes : (array UInt8.t) = alloc_array #UInt8.t !size_expr0;
let mut x1 : s1 = s1_default s1_spec_default;
s1_pack x1;
(! x1).bytes := (! bytes);
(! x1).bytes := (! bytes);
admit();
}

fn mk_s2 ()
returns x: _
ensures exists* y. owns_s2 x y
{
let mut size_expr1 : SizeT.t = (uint64_to_sizet (int32_to_uint64 32l));
let mut bytes : (array UInt8.t) = alloc_array #UInt8.t !size_expr1;
let mut x1 : (ref s1) = s1_alloc ();
Mks1?.bytes (! (! x1)) := (! bytes);
let mut x2 : (ref s2) = s2_alloc ();
Mks2?.field_s1 (! (! x2)) := (! x1);
intro_owns_s2 ();
(! x2);
}

fn read_byte0
(x2 : ( ref s2) )
preserves exists* y. owns_s2 x2 y
returns UInt8.t
{
let mut x2 : (ref s2) = x2;
let mut x1 : (ref s1) = (! (! (! x2)).field_s1);
pts_to_len (!(!(!x1)).bytes);
let mut res : UInt8.t = (op_Array_Access (! (! (! x1)).bytes) (int32_to_sizet 0l));
(! res);
}
