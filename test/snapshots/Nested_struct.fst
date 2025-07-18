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

//Dumping the Clang AST.
// RecordDecl 0x5e52fa66c288 </home/t-visinghal/Applications/src/c2pulse/test/general/nested_struct.c:5:1, line:7:1> line:5:8 struct list definition
// `-FieldDecl 0x5e52fa66c3e8 <line:6:5, col:18> col:18 referenced next 'struct list *'
// FunctionDecl 0x5e52fa66c578 </home/t-visinghal/Applications/src/c2pulse/test/general/nested_struct.c:12:1, line:15:1> line:12:5 foo 'int (struct list *)'
// |-ParmVarDecl 0x5e52fa66c460 <col:9, col:22> col:22 used lst 'struct list *'
// `-CompoundStmt 0x5e52fa66c848 <col:26, line:15:1>
//   |-AttributedStmt 0x5e52fa66c6e0 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:18, /home/t-visinghal/Applications/src/c2pulse/test/general/nested_struct.c:13:17>
//   | |-AnnotateAttr 0x5e52fa66c668 </home/t-visinghal/Applications/src/c2pulse/test/general/../include/PulseMacros.h:8:20, col:54> pulse "lemma:admit()|END"
//   | `-NullStmt 0x5e52fa66c660 </home/t-visinghal/Applications/src/c2pulse/test/general/nested_struct.c:13:17>
//   `-BinaryOperator 0x5e52fa66c828 <line:14:3, col:26> 'struct list *' '='
//     |-MemberExpr 0x5e52fa66c778 <col:3, col:14> 'struct list *' lvalue ->next 0x5e52fa66c3e8
//     | `-ImplicitCastExpr 0x5e52fa66c760 <col:3, col:8> 'struct list *' <LValueToRValue>
//     |   `-MemberExpr 0x5e52fa66c730 <col:3, col:8> 'struct list *' lvalue ->next 0x5e52fa66c3e8
//     |     `-ImplicitCastExpr 0x5e52fa66c718 <col:3> 'struct list *' <LValueToRValue>
//     |       `-DeclRefExpr 0x5e52fa66c6f8 <col:3> 'struct list *' lvalue ParmVar 0x5e52fa66c460 'lst' 'struct list *'
//     `-ImplicitCastExpr 0x5e52fa66c810 <col:21, col:26> 'struct list *' <LValueToRValue>
//       `-MemberExpr 0x5e52fa66c7e0 <col:21, col:26> 'struct list *' lvalue ->next 0x5e52fa66c3e8
//         `-ImplicitCastExpr 0x5e52fa66c7c8 <col:21> 'struct list *' <LValueToRValue>
//           `-DeclRefExpr 0x5e52fa66c7a8 <col:21> 'struct list *' lvalue ParmVar 0x5e52fa66c460 'lst' 'struct list *'
