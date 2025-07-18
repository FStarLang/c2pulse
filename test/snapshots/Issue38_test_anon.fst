module Issue38_test_anon

#lang-pulse

open Pulse
open Pulse.Lib.C



noeq
type anon_name0 = {
x: ref Int32.t;
y: ref Int32.t;
}

[@@erasable]
noeq
type anon_name0_spec = {
x : Int32.t;
y : Int32.t

}

let anon_name0_pred (x:ref anon_name0) (s:anon_name0_spec) : slprop =
exists* (y: anon_name0). (x |-> y) **
(y.x |-> s.x) **
(y.y |-> s.y)

assume val anon_name0_spec_default : anon_name0_spec

assume val anon_name0_default (anon_name0_var_spec:anon_name0_spec) : anon_name0

ghost
fn anon_name0_pack (anon_name0_var:ref anon_name0) (#anon_name0_var_spec:anon_name0_spec)
requires anon_name0_var|-> anon_name0_default anon_name0_var_spec
ensures exists* v. anon_name0_pred anon_name0_var v ** pure (v == anon_name0_var_spec)
{ admit() }

ghost
fn anon_name0_unpack (anon_name0_var:ref anon_name0)
requires exists* v. anon_name0_pred anon_name0_var v 
ensures exists* u. anon_name0_var |-> u
{ admit() }

fn anon_name0_alloc ()
returns x:ref anon_name0
ensures freeable x
ensures exists* v. anon_name0_pred x v
{ admit () }

fn anon_name0_free (x:ref anon_name0)
requires freeable x
requires exists* v. anon_name0_pred x v
{ admit() }


ghost fn anon_name0_explode (x:ref anon_name0) (#s:anon_name0_spec)
requires anon_name0_pred x s
ensures exists* (v: anon_name0). (x |-> v) ** (v.x |-> s.x) ** (v.y |-> s.y)
{unfold anon_name0_pred}


ghost
fn anon_name0_recover (x:ref anon_name0) (#a0 : Int32.t) (#a1 : Int32.t) 
requires exists* (y: anon_name0). (x |-> y) ** (y.x |-> a0) ** (y.y |-> a1)
ensures exists* w. anon_name0_pred x w ** pure (w == {x = a0; y = a1})
{fold anon_name0_pred x ({x = a0; y = a1}) }

noeq
type foo = {
s: ref anon_name0;
}

[@@erasable]
noeq
type foo_spec = {
s : anon_name0_spec

}

let foo_pred (x:ref foo) (s:foo_spec) : slprop =
exists* (y: foo). (x |-> y) **
anon_name0_pred y.s s.s

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
ensures exists* (v: foo). (x |-> v) ** (v.s `anon_name0_pred` s.s)
{unfold foo_pred}


ghost
fn foo_recover (x:ref foo) (#a0 : anon_name0_spec) 
requires exists* (y: foo). (x |-> y) ** (y.s  `anon_name0_pred` a0)
ensures exists* w. foo_pred x w ** pure (w == {s = a0})
{fold foo_pred x ({s = a0}) }

//Dumping the Clang AST.
// RecordDecl 0x5a02a23c4b30 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue38_test_anon.c:1:1, line:6:1> line:1:8 struct foo definition
// |-RecordDecl 0x5a02a23c4bd0 <line:2:3, line:5:3> line:2:3 struct definition
// | |-FieldDecl 0x5a02a23c4c88 <line:3:5, col:9> col:9 x 'int'
// | `-FieldDecl 0x5a02a23c4cf0 <line:4:5, col:9> col:9 y 'int'
// `-FieldDecl 0x5a02a23c4d98 <line:2:3, line:5:5> col:5 s 'struct (unnamed struct at /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue38_test_anon.c:2:3)':'struct foo::(unnamed at /home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue38_test_anon.c:2:3)'
