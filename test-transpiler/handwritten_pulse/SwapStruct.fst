module SwapStruct
#lang-pulse
open Pulse
module U32 = FStar.UInt32
module Box = Pulse.Lib.Box

(* The following is to be generated from the C type declaration:

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;
*)

//1. An abstract type representing a u32_pair_struct
noeq
type u32_pair_struct = {
  first: ref FStar.UInt32.t;
  second: ref FStar.UInt32.t;
}

//2. A purely functional specification type for the struct
[@@erasable]
noeq
type u32_pair_struct_spec = {
  first: FStar.UInt32.t;
  second: FStar.UInt32.t;
}

//3. A predicate that relates a u32_pair_struct to its specification
let u32_pair_struct_pred (x:ref u32_pair_struct) (s:u32_pair_struct_spec) : slprop =
  exists* (y: u32_pair_struct). (x |-> y) **
  (y.first |-> s.first) **
  (y.second |-> s.second)

//4. A utility function to heap u32_pair_struct_allocate and u32_pair_struct_free a u32_pair_struct
assume val u32_pair_struct_allocated (x: ref u32_pair_struct) : slprop

fn u32_pair_struct_alloc ()
returns x:ref u32_pair_struct
ensures u32_pair_struct_allocated x
ensures exists* v. u32_pair_struct_pred x v
{ admit () }

fn u32_pair_struct_free (x:ref u32_pair_struct)
requires u32_pair_struct_allocated x
requires exists* v. u32_pair_struct_pred x v
{ admit () }

//5. A ghost function that unfolds the predicate for u32_pair_struct_refs
ghost fn u32_pair_struct_explode (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
requires u32_pair_struct_pred x s
ensures exists* (v: u32_pair_struct). 
  (x |-> v) **
  (v.first |-> s.first) **
  (v.second |-> s.second)
{ unfold u32_pair_struct_pred }


//8. A ghost function that folds the predicate for u32_pair_struct_refs
ghost
fn u32_pair_struct_recover (x:ref u32_pair_struct) (#a0 #a1 :erased U32.t)
requires exists* (y: u32_pair_struct). (x |-> y) ** (y.first |-> a0) ** (y.second |-> a1)
ensures exists* w. u32_pair_struct_pred x w ** pure (w == {first = a0; second = a1})
{ fold u32_pair_struct_pred x ({first = a0; second = a1}) }

(*
RETURNS x
ENSURES (exists* (s:u32_pair_struct_spec). Box.box u32_pair_struct_pred x s ** pure (s == {first = 0ul; second = 0ul}))
u32_pair_struct* new_u32_pair_struct ()
{
  u32_pair_struct* x = ( u32_pair_struct* )malloc(sizeof(u32_pair_struct));
  x->f = 0ul;
  x->s = 0ul;
  return x;
}
*)
fn new_u32_pair_struct ()
requires emp
returns x:ref u32_pair_struct
ensures u32_pair_struct_allocated x
ensures (u32_pair_struct_pred x { first = 0ul; second = 1ul })
{
  let x = u32_pair_struct_alloc(); //note the translation of the casted malloc to a typed allocation
  u32_pair_struct_explode x;
  (!x).first := 0ul; // set first field to 0
  (!x).second := 1ul; // set second field to 1
  u32_pair_struct_recover x;
  x
}

(*
ERASED_ARG(#s:u32_pair_struct_spec)
REQUIRES(u32_pair_struct_pred x s)
ENSURES(exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))
void swap_fields (u32_pair_struct* x, #s:u32_pair_struct_spec s)
{
  uint32_t f1 = x->f1;
  x->f1 = x->f2;
  x->f2 = f1;
}
*)
fn swap_fields (#s:u32_pair_struct_spec) (x:ref u32_pair_struct)
requires u32_pair_struct_pred x s
ensures exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))
{
  u32_pair_struct_explode x;
  let f1 = !(!x).first; 
  (!x).first := !(!x).second; // set first field to second field
  (!x).second := f1; // set second field to first field
  u32_pair_struct_recover x;
}


(*

void swap_refs (uint32_t* x, uint32_t* y)
REQUIRES(x |-> 'x)
REQUIRES(y |-> 'y)
ENSURES(x |-> 'y)
ENSURES(y |-> 'x)
{
  uint32_t tmp = *x;
  *x = *y;
  *y = tmp;
}

*)
fn swap_refs (x y:ref U32.t)
requires x |-> 'x
requires y |-> 'y
ensures x |-> 'y
ensures y |-> 'x
{
  let tmp = !x;
  x := !y;
  y := tmp;
}

(*
ERASED_ARG(#s:u32_pair_struct_spec)
REQUIRES(u32_pair_struct_pred x s)
ENSURES(exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first}))
void swap_fields_alt (u32_pair_struct* x)
{
  swap_refs(&x->f1, &x->f2);
}
*)

fn swap_fields_alt (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
requires u32_pair_struct_pred x s
ensures exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first})
{
  u32_pair_struct_explode x;
  swap_refs (!x).first (!x).second;
  u32_pair_struct_recover x;
}

(*

void main ()
{
  u32_pair_struct* x = new_u32_pair_struct ();
  swap_fields (x);
  swap_fields_alt (x);
  ASSERT (u32_pair_struct_pred x {first = 0ul; second = 1ul});
  u32_pair_struct_free(x);
}

*)
fn test_main ()
requires emp
ensures emp
{
  let x = new_u32_pair_struct ();
  // Swap fields using the first method
  swap_fields x;

  swap_fields_alt x;

  assert (u32_pair_struct_pred x {first = 0ul; second = 1ul});

  u32_pair_struct_free x;
}
