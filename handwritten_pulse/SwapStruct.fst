module SwapStruct
#lang-pulse
open SwapStruct.Types
open Pulse
module U32 = FStar.UInt32
module Box = Pulse.Lib.Box

(* The type definitions corresponding to the following struct 
   are generated in SwapStruct.Types:

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;

*)


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
returns x:Box.box u32_pair_struct
ensures exists* (s:u32_pair_struct_spec). SwapStruct.Types.u32_pair_struct_pred (Box.box_to_ref x) s ** pure (s == {first = 0ul; second = 1ul})
{
  let x = SwapStruct.Types.alloc(); //note the translatio of the casted malloc to a typed allocation
  SwapStruct.Types.set_first (Box.box_to_ref x) 0ul; // assigning to a field of the struct translated to a setter
  SwapStruct.Types.set_second (Box.box_to_ref x) 1ul; // assigning to a field of the struct translated to a getter
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
fn swap_fields (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
requires SwapStruct.Types.u32_pair_struct_pred x s
ensures exists* (s':u32_pair_struct_spec). SwapStruct.Types.u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))
{
  let f1 = SwapStruct.Types.get_first x;
  SwapStruct.Types.set_first x (SwapStruct.Types.get_second x);
  SwapStruct.Types.set_second x f1;
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
requires SwapStruct.Types.u32_pair_struct_pred x s
ensures exists* (s':u32_pair_struct_spec). SwapStruct.Types.u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first})
{
  let y = SwapStruct.Types.explode x;
  SwapStruct.Types.u32_pair_struct_refs_pred_unfold y;
  swap_refs y.first y.second;
  SwapStruct.Types.u32_pair_struct_refs_pred_fold y;
  SwapStruct.Types.restore x y;
}

(*

void main () 
{
  u32_pair_struct* x = new_u32_pair_struct ();
  swap_fields (x);  //translate the heap allocated Box to a ref
  swap_fields_alt (x); //translate the heap allocated Box to a ref
  ASSERT (u32_pair_struct_pred x {first = 0ul; second = 1ul});
  free(x);
}

*)
fn test_main ()
requires emp
ensures emp
{
  let x = new_u32_pair_struct ();
  // Swap fields using the first method
  swap_fields (Box.box_to_ref x);

  swap_fields_alt (Box.box_to_ref x);

  assert (SwapStruct.Types.u32_pair_struct_pred (Box.box_to_ref x) {first = 0ul; second = 1ul});

  SwapStruct.Types.free x;
}
  