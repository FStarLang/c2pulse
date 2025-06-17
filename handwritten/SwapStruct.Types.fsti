module SwapStruct.Types
#lang-pulse
open FStar.Set
open Pulse
module U32 = FStar.UInt32
module Box = Pulse.Lib.Box
(* All of the following is to be generated from the C type declaration:

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;

Note: In the translation below, I have focused on pointers to structs, and
pointers to their fields, and setters, getters for (x->f). 

We should eventually also support struct passing and direct field access (x.f)
*)

//1. An abstract type representing a u32_pair_struct
val u32_pair_struct : Type0

//2. A purely functional specification type for the struct
[@@erasable]
noeq
type u32_pair_struct_spec = {
  first: FStar.UInt32.t;
  second: FStar.UInt32.t 
}

//3. A predicate that relates a u32_pair_struct to its specification
val u32_pair_struct_pred (_:ref u32_pair_struct) (_:u32_pair_struct_spec) : slprop

//4. A utility function to heap allocate and free a u32_pair_struct
fn alloc ()
requires emp
returns x:Box.box u32_pair_struct
ensures exists* v. u32_pair_struct_pred (Box.box_to_ref x) v //not a great model of uninitialized memory, but it's a start

fn free (x:Box.box u32_pair_struct)
requires exists* v. u32_pair_struct_pred (Box.box_to_ref x) v
ensures emp

//5. Setters and getters for each field
fn set_first (x:ref u32_pair_struct) (f:U32.t) (#s:u32_pair_struct_spec)
requires u32_pair_struct_pred x s
ensures exists* s'. u32_pair_struct_pred x s' ** pure (s' == {s with first=f})

fn get_first (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
requires u32_pair_struct_pred x s
returns f:U32.t
ensures u32_pair_struct_pred x s ** pure (f == s.first)

fn set_second (x:ref u32_pair_struct) (f:U32.t) (#s:u32_pair_struct_spec)
requires u32_pair_struct_pred x s
ensures exists* s'. u32_pair_struct_pred x s' ** pure (s' == {s with second=f})

fn get_second (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
requires u32_pair_struct_pred x s
returns f:U32.t
ensures u32_pair_struct_pred x s ** pure (f == s.second)

//5. A type containing pointers to each field of the struct
noeq
type u32_pair_struct_refs = {
  first: ref FStar.UInt32.t;
  second: ref FStar.UInt32.t 
}

//6. A predicate that relates a u32_pair_struct_refs to its specification
let u32_pair_struct_refs_pred (x:u32_pair_struct_refs) (s:u32_pair_struct_spec) : slprop =
  (x.first |-> s.first) **
  (x.second |-> s.second)

//7. A predicate that relates a u32_pair_struct to its field pointers, u32_pair_struct_refs
val recover (x:ref u32_pair_struct) (y:u32_pair_struct_refs) : slprop

//8. A function that converts a u32_pair_struct to u32_pair_struct_refs
//   i.e., borrowing pointrs to the fields of a struct
fn explode (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
requires u32_pair_struct_pred x s
returns y:u32_pair_struct_refs
ensures u32_pair_struct_refs_pred y s
ensures recover x y

//9. A function that converts u32_pair_struct_refs back to u32_pair_struct
//   i.e., restoring the struct from its field pointers
ghost
fn restore (x:ref u32_pair_struct) (y:u32_pair_struct_refs) (#s:u32_pair_struct_spec)
requires u32_pair_struct_refs_pred y s
requires recover x y
ensures u32_pair_struct_pred x s

//10. A ghost function that unfolds the predicate for u32_pair_struct_refs
ghost
fn u32_pair_struct_refs_pred_unfold (x:u32_pair_struct_refs) (#s:u32_pair_struct_spec)
requires u32_pair_struct_refs_pred x s
ensures x.first |-> s.first
ensures x.second |-> s.second

//11. A ghost function that folds the predicate for u32_pair_struct_refs
ghost
fn u32_pair_struct_refs_pred_fold (x:u32_pair_struct_refs) (#f #s:erased U32.t)
requires x.first |-> f
requires x.second |-> s
ensures u32_pair_struct_refs_pred x ({first = f; second = s})