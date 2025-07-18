module Issue38_test_5

#lang-pulse

open Pulse
open Pulse.Lib.C



noeq
type foo = {
v: ref Int32.t;
foo0: ref (ref foo);
}

[@@erasable]
noeq
type foo_spec = {
v : Int32.t;
foo0 : ref foo

}

let foo_pred (x:ref foo) (s:foo_spec) : slprop =
exists* (y: foo). (x |-> y) **
(y.v |-> s.v) **
(y.foo0 |-> s.foo0)

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
ensures exists* (v: foo). (x |-> v) ** (v.v |-> s.v) ** (v.foo0 |-> s.foo0)
{unfold foo_pred}


ghost
fn foo_recover (x:ref foo) (#a0 : Int32.t) (#a1 : (ref foo)) 
requires exists* (y: foo). (x |-> y) ** (y.v |-> a0) ** (y.foo0 |-> a1)
ensures exists* w. foo_pred x w ** pure (w == {v = a0; foo0 = a1})
{fold foo_pred x ({v = a0; foo0 = a1}) }

//Dumping the Clang AST.
// RecordDecl 0x5fdf32b29730 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue38_test_5.c:1:1, line:4:1> line:1:8 struct foo definition
// |-FieldDecl 0x5fdf32b297e8 <line:2:3, col:7> col:7 v 'int'
// `-FieldDecl 0x5fdf32b298f8 <line:3:3, col:15> col:15 foo 'struct foo *'
