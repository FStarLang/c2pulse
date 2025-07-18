module Nested_struct

#lang-pulse

open Pulse
open Pulse.Lib.C



noeq
type list = {
next: ref (ref list);
}

[@@erasable]
noeq
type list_spec = {
next : ref list

}

let list_pred (x:ref list) (s:list_spec) : slprop =
exists* (y: list). (x |-> y) **
(y.next |-> s.next)

assume val list_spec_default : list_spec

assume val list_default (list_var_spec:list_spec) : list

ghost
fn list_pack (list_var:ref list) (#list_var_spec:list_spec)
requires list_var|-> list_default list_var_spec
ensures exists* v. list_pred list_var v ** pure (v == list_var_spec)
{ admit() }

ghost
fn list_unpack (list_var:ref list)
requires exists* v. list_pred list_var v 
ensures exists* u. list_var |-> u
{ admit() }

fn list_alloc ()
returns x:ref list
ensures freeable x
ensures exists* v. list_pred x v
{ admit () }

fn list_free (x:ref list)
requires freeable x
requires exists* v. list_pred x v
{ admit() }


ghost fn list_explode (x:ref list) (#s:list_spec)
requires list_pred x s
ensures exists* (v: list). (x |-> v) ** (v.next |-> s.next)
{unfold list_pred}


ghost
fn list_recover (x:ref list) (#a0 : (ref list)) 
requires exists* (y: list). (x |-> y) ** (y.next |-> a0)
ensures exists* w. list_pred x w ** pure (w == {next = a0})
{fold list_pred x ({next = a0}) }

fn foo
(lst : ref list)
{
let mut lst : (ref list) = lst;
admit();
Mklist?.next (! (! (! (! lst)).next)) := (! (! (! lst)).next);
}
