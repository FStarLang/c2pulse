module Random

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



noeq
type s1 = {
bytes: (array bool);
charbytes: (array Int8.t);
matrix: (array (array Int32.t));
length: ref Int32.t;
arr: (array bool);
}

[@@erasable]
noeq
type s1_spec = {
bytes : Seq.seq (bool);
charbytes : Seq.seq (Int8.t);
matrix : Seq.seq (array Int32.t);
length : Int32.t;
arr : Seq.seq (bool)

}

[@@pulse_unfold]
let s1_pred ([@@@mkey]x:ref s1) (s:s1_spec) : slprop =
exists* (y: s1). (x |-> y) **
(y.bytes |-> s.bytes) **
(y.charbytes |-> s.charbytes) **
(y.matrix |-> s.matrix) **
(y.length |-> s.length) **
(y.arr |-> s.arr)

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
ensures exists* (v: s1). (x |-> v) ** (v.bytes |-> s.bytes) ** 
(v.charbytes |-> s.charbytes) ** 
(v.matrix |-> s.matrix) ** 
(v.length |-> s.length) ** 
(v.arr |-> s.arr)

{unfold s1_pred}


ghost
fn s1_recover (x:ref s1) (#a0 : (Seq.seq bool))
(#a1 : (Seq.seq Int8.t))
(#a2 : (Seq.seq (array Int32.t)))
(#a3 : Int32.t)
(#a4 : (Seq.seq bool))

requires exists* (y: s1). (x |-> y) ** 
(y.bytes |-> a0) **
(y.charbytes |-> a1) **
(y.matrix |-> a2) **
(y.length |-> a3) **
(y.arr |-> a4)
ensures exists* w. s1_pred x w ** pure (w == {bytes = a0;
charbytes = a1;
matrix = a2;
length = a3;
arr = a4})
{fold s1_pred x ({bytes = a0;
charbytes = a1;
matrix = a2;
length = a3;
arr = a4}) }

fn foo
(x1 : ( ref s1) )
preserves 
exists* s1_s. s1_pred x1 s1_s
{
let mut x1 : (ref s1) = x1;
s1_explode (!x1);
pts_to_len ((!(!x1)).bytes);
let mut tempArr : (array bool) = (! (! x1)).bytes;
let mut tempArr1 : (array Int8.t) = (! (! x1)).charbytes;
let mut x: Int32.t = witness #_ #_;
let mut matrix : (array (array Int32.t)) = [| witness #_ #_; 10sz |];
pluspluspost_int32 x;
x := (Int32.add (! x) 2l);
x := Int32.add (! x) 1l;
x := Int32.div (! x) 2l;
x := Int32.mul (! x) 2l;
x := (Int32.rem (! x) 2l);
admit();
();
}
