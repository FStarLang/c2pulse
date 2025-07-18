module Simple_nested_struct_test

#lang-pulse

open Pulse
open Pulse.Lib.C



noeq
type u32_pair_struct = {
first: ref UInt32.t;
second: ref UInt32.t;
}

[@@erasable]
noeq
type u32_pair_struct_spec = {
first : UInt32.t;
second : UInt32.t

}

let u32_pair_struct_pred (x:ref u32_pair_struct) (s:u32_pair_struct_spec) : slprop =
exists* (y: u32_pair_struct). (x |-> y) **
(y.first |-> s.first) **
(y.second |-> s.second)

assume val u32_pair_struct_spec_default : u32_pair_struct_spec

assume val u32_pair_struct_default (u32_pair_struct_var_spec:u32_pair_struct_spec) : u32_pair_struct

ghost
fn u32_pair_struct_pack (u32_pair_struct_var:ref u32_pair_struct) (#u32_pair_struct_var_spec:u32_pair_struct_spec)
requires u32_pair_struct_var|-> u32_pair_struct_default u32_pair_struct_var_spec
ensures exists* v. u32_pair_struct_pred u32_pair_struct_var v ** pure (v == u32_pair_struct_var_spec)
{ admit() }

ghost
fn u32_pair_struct_unpack (u32_pair_struct_var:ref u32_pair_struct)
requires exists* v. u32_pair_struct_pred u32_pair_struct_var v 
ensures exists* u. u32_pair_struct_var |-> u
{ admit() }

fn u32_pair_struct_alloc ()
returns x:ref u32_pair_struct
ensures freeable x
ensures exists* v. u32_pair_struct_pred x v
{ admit () }

fn u32_pair_struct_free (x:ref u32_pair_struct)
requires freeable x
requires exists* v. u32_pair_struct_pred x v
{ admit() }


ghost fn u32_pair_struct_explode (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
requires u32_pair_struct_pred x s
ensures exists* (v: u32_pair_struct). (x |-> v) ** (v.first |-> s.first) ** (v.second |-> s.second)
{unfold u32_pair_struct_pred}


ghost
fn u32_pair_struct_recover (x:ref u32_pair_struct) (#a0 : UInt32.t) (#a1 : UInt32.t) 
requires exists* (y: u32_pair_struct). (x |-> y) ** (y.first |-> a0) ** (y.second |-> a1)
ensures exists* w. u32_pair_struct_pred x w ** pure (w == {first = a0; second = a1})
{fold u32_pair_struct_pred x ({first = a0; second = a1}) }

noeq
type u64_pair_struct = {
first: ref UInt64.t;
second: ref (ref u32_pair_struct);
}

[@@erasable]
noeq
type u64_pair_struct_spec = {
first : UInt64.t;
second : ref u32_pair_struct

}

let u64_pair_struct_pred (x:ref u64_pair_struct) (s:u64_pair_struct_spec) : slprop =
exists* (y: u64_pair_struct). (x |-> y) **
(y.first |-> s.first) **
(y.second |-> s.second)

assume val u64_pair_struct_spec_default : u64_pair_struct_spec

assume val u64_pair_struct_default (u64_pair_struct_var_spec:u64_pair_struct_spec) : u64_pair_struct

ghost
fn u64_pair_struct_pack (u64_pair_struct_var:ref u64_pair_struct) (#u64_pair_struct_var_spec:u64_pair_struct_spec)
requires u64_pair_struct_var|-> u64_pair_struct_default u64_pair_struct_var_spec
ensures exists* v. u64_pair_struct_pred u64_pair_struct_var v ** pure (v == u64_pair_struct_var_spec)
{ admit() }

ghost
fn u64_pair_struct_unpack (u64_pair_struct_var:ref u64_pair_struct)
requires exists* v. u64_pair_struct_pred u64_pair_struct_var v 
ensures exists* u. u64_pair_struct_var |-> u
{ admit() }

fn u64_pair_struct_alloc ()
returns x:ref u64_pair_struct
ensures freeable x
ensures exists* v. u64_pair_struct_pred x v
{ admit () }

fn u64_pair_struct_free (x:ref u64_pair_struct)
requires freeable x
requires exists* v. u64_pair_struct_pred x v
{ admit() }


ghost fn u64_pair_struct_explode (x:ref u64_pair_struct) (#s:u64_pair_struct_spec)
requires u64_pair_struct_pred x s
ensures exists* (v: u64_pair_struct). (x |-> v) ** (v.first |-> s.first) ** (v.second |-> s.second)
{unfold u64_pair_struct_pred}


ghost
fn u64_pair_struct_recover (x:ref u64_pair_struct) (#a0 : UInt64.t) (#a1 : (ref u32_pair_struct)) 
requires exists* (y: u64_pair_struct). (x |-> y) ** (y.first |-> a0) ** (y.second |-> a1)
ensures exists* w. u64_pair_struct_pred x w ** pure (w == {first = a0; second = a1})
{fold u64_pair_struct_pred x ({first = a0; second = a1}) }

//Dumping the Clang AST.
// RecordDecl 0x5f3a70b38cc8 </home/t-visinghal/Applications/src/c2pulse/test/general/simple_nested_struct_test.c:5:9, line:8:1> line:5:16 struct _u32_pair_struct definition
// |-FieldDecl 0x5f3a70b38de0 <line:6:3, col:12> col:12 first 'uint32_t':'unsigned int'
// `-FieldDecl 0x5f3a70b38e40 <line:7:3, col:12> col:12 second 'uint32_t':'unsigned int'
// TypedefDecl 0x5f3a70b38ee8 </home/t-visinghal/Applications/src/c2pulse/test/general/simple_nested_struct_test.c:5:1, line:8:3> col:3 referenced u32_pair_struct 'struct _u32_pair_struct'
// `-ElaboratedType 0x5f3a70b38e90 'struct _u32_pair_struct' sugar
//   `-RecordType 0x5f3a70b38d50 'struct _u32_pair_struct'
//     `-Record 0x5f3a70b38cc8 '_u32_pair_struct'
// RecordDecl 0x5f3a70b38f58 </home/t-visinghal/Applications/src/c2pulse/test/general/simple_nested_struct_test.c:10:9, line:13:1> line:10:16 struct _u64_pair_struct definition
// |-FieldDecl 0x5f3a70b39070 <line:11:3, col:12> col:12 first 'uint64_t':'unsigned long'
// `-FieldDecl 0x5f3a70b39198 <line:12:3, col:20> col:20 second 'u32_pair_struct *'
// TypedefDecl 0x5f3a70b39248 </home/t-visinghal/Applications/src/c2pulse/test/general/simple_nested_struct_test.c:10:1, line:13:3> col:3 u64_pair_struct 'struct _u64_pair_struct'
// `-ElaboratedType 0x5f3a70b391f0 'struct _u64_pair_struct' sugar
//   `-RecordType 0x5f3a70b38fe0 'struct _u64_pair_struct'
//     `-Record 0x5f3a70b38f58 '_u64_pair_struct'
