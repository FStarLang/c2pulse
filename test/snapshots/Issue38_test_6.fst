module Issue38_test_6

#lang-pulse

open Pulse
open Pulse.Lib.C



noeq
type foo = {
left: ref Int32.t;
next: ref (ref foo);
}

[@@erasable]
noeq
type foo_spec = {
left : Int32.t;
next : ref foo

}

let foo_pred (x:ref foo) (s:foo_spec) : slprop =
exists* (y: foo). (x |-> y) **
(y.left |-> s.left) **
(y.next |-> s.next)

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
ensures exists* (v: foo). (x |-> v) ** (v.left |-> s.left) ** (v.next |-> s.next)
{unfold foo_pred}


ghost
fn foo_recover (x:ref foo) (#a0 : Int32.t) (#a1 : (ref foo)) 
requires exists* (y: foo). (x |-> y) ** (y.left |-> a0) ** (y.next |-> a1)
ensures exists* w. foo_pred x w ** pure (w == {left = a0; next = a1})
{fold foo_pred x ({left = a0; next = a1}) }

//Dumping the Clang AST.
// RecordDecl 0x5d0a51efdc40 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue38_test_6.c:1:1, col:8> col:8 struct foo
// TypedefDecl 0x5d0a51efdd98 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue38_test_6.c:2:1, col:21> col:21 referenced foo_ptr 'struct foo *'
// `-PointerType 0x5d0a51efdd40 'struct foo *'
//   `-ElaboratedType 0x5d0a51efdce0 'struct foo' sugar
//     `-RecordType 0x5d0a51efdcc0 'struct foo'
//       `-Record 0x5d0a51efddf0 'foo'
// RecordDecl 0x5d0a51efddf0 prev 0x5d0a51efdc40 </home/t-visinghal/Applications/src/c2pulse/test/issue-related/issue38_test_6.c:3:1, line:6:1> line:3:8 struct foo definition
// |-FieldDecl 0x5d0a51efde88 <line:4:9, col:13> col:13 left 'int'
// `-FieldDecl 0x5d0a51efdf50 <line:5:9, col:17> col:17 next 'foo_ptr':'struct foo *'
