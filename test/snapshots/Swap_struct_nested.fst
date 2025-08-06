module Swap_struct_nested

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

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

let bar_pred ([@@@mkey]x:ref bar) (s:bar_spec) : slprop =
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
ensures exists* (v: bar). (x |-> v) ** (v.x |-> s.x) ** 
(v.y |-> s.y)

{unfold bar_pred}


ghost
fn bar_recover (x:ref bar) (#a0 : Int32.t)
(#a1 : Int32.t)

requires exists* (y: bar). (x |-> y) ** 
(y.x |-> a0) **
(y.y |-> a1)
ensures exists* w. bar_pred x w ** pure (w == {x = a0;
y = a1})
{fold bar_pred x ({x = a0;
y = a1}) }

noeq
type foo = {
c: ref bar;
}

[@@erasable]
noeq
type foo_spec = {
c : bar_spec

}

let foo_pred ([@@@mkey]x:ref foo) (s:foo_spec) : slprop =
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

requires exists* (y: foo). (x |-> y) ** 
(y.c  `bar_pred` a0)
ensures exists* w. foo_pred x w ** pure (w == {c = a0})
{fold foo_pred x ({c = a0}) }
