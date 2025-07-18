module Swap_struct_nested

#lang-pulse

open Pulse
open Pulse.Lib.C



noeq
type bar = {
x: ref Int32.t;
y: ref Int32.t;
}

[@@erasable]
noeq
type bar_spec = {
x : Int32.t;
y : Int32.t

}

let bar_pred (x:ref bar) (s:bar_spec) : slprop =
exists* (y: bar). (x |-> y) **
(y.x |-> s.x) **
(y.y |-> s.y)

assume val bar_spec_default : bar_spec

assume val bar_default (bar_var_spec:bar_spec) : bar

ghost
fn bar_pack (bar_var:ref bar) (#bar_var_spec:bar_spec)
requires bar_var|-> bar_default bar_var_spec
ensures exists* v. bar_pred bar_var v ** pure (v == bar_var_spec)
{ admit() }

ghost
fn bar_unpack (bar_var:ref bar)
requires exists* v. bar_pred bar_var v 
ensures exists* u. bar_var |-> u
{ admit() }

fn bar_alloc ()
returns x:ref bar
ensures freeable x
ensures exists* v. bar_pred x v
{ admit () }

fn bar_free (x:ref bar)
requires freeable x
requires exists* v. bar_pred x v
{ admit() }


ghost fn bar_explode (x:ref bar) (#s:bar_spec)
requires bar_pred x s
ensures exists* (v: bar). (x |-> v) ** (v.x |-> s.x) ** (v.y |-> s.y)
{unfold bar_pred}


ghost
fn bar_recover (x:ref bar) (#a0 : Int32.t) (#a1 : Int32.t) 
requires exists* (y: bar). (x |-> y) ** (y.x |-> a0) ** (y.y |-> a1)
ensures exists* w. bar_pred x w ** pure (w == {x = a0; y = a1})
{fold bar_pred x ({x = a0; y = a1}) }

noeq
type foo = {
c: ref bar;
}

[@@erasable]
noeq
type foo_spec = {
c : bar_spec

}

let foo_pred (x:ref foo) (s:foo_spec) : slprop =
exists* (y: foo). (x |-> y) **
bar_pred y.c s.c

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
ensures exists* (v: foo). (x |-> v) ** (v.c `bar_pred` s.c)
{unfold foo_pred}


ghost
fn foo_recover (x:ref foo) (#a0 : bar_spec) 
requires exists* (y: foo). (x |-> y) ** (y.c  `bar_pred` a0)
ensures exists* w. foo_pred x w ** pure (w == {c = a0})
{fold foo_pred x ({c = a0}) }

//Dumping the Clang AST.
// RecordDecl 0x58b06eabb510 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_nested.c:1:9, line:4:1> line:1:16 struct _bar definition
// |-FieldDecl 0x58b06eabb5c8 <line:2:9, col:13> col:13 x 'int'
// `-FieldDecl 0x58b06eabb630 <line:3:9, col:13> col:13 y 'int'
// TypedefDecl 0x58b06eabb6d8 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_nested.c:1:1, line:4:3> col:3 referenced bar 'struct _bar'
// `-ElaboratedType 0x58b06eabb680 'struct _bar' sugar
//   `-RecordType 0x58b06eabb590 'struct _bar'
//     `-Record 0x58b06eabb510 '_bar'
// RecordDecl 0x58b06eabb748 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_nested.c:7:9, line:9:1> line:7:16 struct _foo definition
// `-FieldDecl 0x58b06eabb860 <line:8:5, col:9> col:9 c 'bar':'struct _bar'
// TypedefDecl 0x58b06eabb968 </home/t-visinghal/Applications/src/c2pulse/test/general/swap_struct_nested.c:7:1, line:9:3> col:3 foo 'struct _foo'
// `-ElaboratedType 0x58b06eabb910 'struct _foo' sugar
//   `-RecordType 0x58b06eabb7d0 'struct _foo'
//     `-Record 0x58b06eabb748 '_foo'
