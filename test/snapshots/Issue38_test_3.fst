module Issue38_test_3

#lang-pulse

open Pulse
open Pulse.Lib.C



noeq
type foo = {
left: ref Int32.t;
}

[@@erasable]
noeq
type foo_spec = {
left : Int32.t

}

let foo_pred (x:ref foo) (s:foo_spec) : slprop =
exists* (y: foo). (x |-> y) **
(y.left |-> s.left)

assume val foo_spec_default : foo_spec

assume val foo_default (foo_var_spec:foo_spec) : foo

ghost
fn foo_pack (foo_var:ref foo) (#foo_var_spec:foo_spec)
requires foo_var|-> foo_default foo_var_spec
ensures exists* v. foo_pred foo_var v ** pure (v == foo_var_spec)
{ admit() }

ghost
fn foo_unpack (foo_var:ref foo)
requires exists* v. foo_pred foo_var v 
ensures exists* u. foo_var |-> u
{ admit() }

fn foo_alloc ()
returns x:ref foo
ensures freeable x
ensures exists* v. foo_pred x v
{ admit () }

fn foo_free (x:ref foo)
requires freeable x
requires exists* v. foo_pred x v
{ admit() }


ghost fn foo_explode (x:ref foo) (#s:foo_spec)
requires foo_pred x s
ensures exists* (v: foo). (x |-> v) ** (v.left |-> s.left)

{unfold foo_pred}


ghost
fn foo_recover (x:ref foo) (#a0 : Int32.t) 
requires exists* (y: foo). (x |-> y) ** 
(y.left |-> a0)
ensures exists* w. foo_pred x w ** pure (w == {left = a0})
{fold foo_pred x ({left = a0}) }
