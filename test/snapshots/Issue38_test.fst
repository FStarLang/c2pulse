module Issue38_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

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

[@@pulse_unfold]
let foo_pred ([@@@mkey]x:ref foo) (s:foo_spec) : slprop =
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

noeq
type foo2 = {
left: ref Int32.t;
}

[@@erasable]
noeq
type foo2_spec = {
left : Int32.t

}

[@@pulse_unfold]
let foo2_pred ([@@@mkey]x:ref foo2) (s:foo2_spec) : slprop =
exists* (y: foo2). (x |-> y) **
(y.left |-> s.left)

assume val foo2_spec_default : foo2_spec

assume val foo2_default (foo2_var_spec:foo2_spec) : foo2

ghost
fn foo2_pack (foo2_var:ref foo2) (#foo2_var_spec:foo2_spec)
requires foo2_var|-> foo2_default foo2_var_spec
ensures exists* v. foo2_pred foo2_var v ** pure (v == foo2_var_spec)
{ admit() }

ghost
fn foo2_unpack (foo2_var:ref foo2)
requires exists* v. foo2_pred foo2_var v 
ensures exists* u. foo2_var |-> u
{ admit() }

fn foo2_alloc ()
returns x:ref foo2
ensures freeable x
ensures exists* v. foo2_pred x v
{ admit () }

fn foo2_free (x:ref foo2)
requires freeable x
requires exists* v. foo2_pred x v
{ admit() }


ghost fn foo2_explode (x:ref foo2) (#s:foo2_spec)
requires foo2_pred x s
ensures exists* (v: foo2). (x |-> v) ** (v.left |-> s.left)

{unfold foo2_pred}


ghost
fn foo2_recover (x:ref foo2) (#a0 : Int32.t)

requires exists* (y: foo2). (x |-> y) ** 
(y.left |-> a0)
ensures exists* w. foo2_pred x w ** pure (w == {left = a0})
{fold foo2_pred x ({left = a0}) }

noeq
type xy = {
x: ref Int32.t;
y: ref Int32.t;
}

[@@erasable]
noeq
type xy_spec = {
x : Int32.t;
y : Int32.t

}

[@@pulse_unfold]
let xy_pred ([@@@mkey]x:ref xy) (s:xy_spec) : slprop =
exists* (y: xy). (x |-> y) **
(y.x |-> s.x) **
(y.y |-> s.y)

assume val xy_spec_default : xy_spec

assume val xy_default (xy_var_spec:xy_spec) : xy

ghost
fn xy_pack (xy_var:ref xy) (#xy_var_spec:xy_spec)
requires xy_var|-> xy_default xy_var_spec
ensures exists* v. xy_pred xy_var v ** pure (v == xy_var_spec)
{ admit() }

ghost
fn xy_unpack (xy_var:ref xy)
requires exists* v. xy_pred xy_var v 
ensures exists* u. xy_var |-> u
{ admit() }

fn xy_alloc ()
returns x:ref xy
ensures freeable x
ensures exists* v. xy_pred x v
{ admit () }

fn xy_free (x:ref xy)
requires freeable x
requires exists* v. xy_pred x v
{ admit() }


ghost fn xy_explode (x:ref xy) (#s:xy_spec)
requires xy_pred x s
ensures exists* (v: xy). (x |-> v) ** (v.x |-> s.x) ** 
(v.y |-> s.y)

{unfold xy_pred}


ghost
fn xy_recover (x:ref xy) (#a0 : Int32.t)
(#a1 : Int32.t)

requires exists* (y: xy). (x |-> y) ** 
(y.x |-> a0) **
(y.y |-> a1)
ensures exists* w. xy_pred x w ** pure (w == {x = a0;
y = a1})
{fold xy_pred x ({x = a0;
y = a1}) }

fn bar_decl ()
returns xy
{
admit();
}
