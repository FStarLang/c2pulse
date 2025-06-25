// RUN: %c2pulse %s 
// RUN: cat %p/Swap_struct/Module_0.fst | %{FILECHECK} %s --check-prefix=C2PULSE
// RUN: cat %p/Swap_struct/Module_u32_pair_struct.fsti | %{FILECHECK} %s --check-prefix=C2PULSE_INCLUDE
// RUN: %run_fstar.sh %p/Swap_struct/Module_0.fst %p/Swap_struct/Module_u32_pair_struct.fsti 2>&1 | %{FILECHECK} %s --check-prefix=PULSE

#include <stdint.h>
#include <stdlib.h>
#include "../../test-transpiler/c/pulse_macros.h"

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;


REQUIRES(emp)
RETURNS (x:Box.box u32_pair_struct)
ENSURES (exists* (s:u32_pair_struct_spec). u32_pair_struct_pred (Box.box_to_ref x) s ** pure (s == {first = 0ul; second = 1ul}))
u32_pair_struct* new_u32_pair_struct()
{
  u32_pair_struct* x = (u32_pair_struct*)malloc(sizeof(u32_pair_struct));
  x->first = 0;
  x->second = 1;
  return x;
}

ERASED_ARG(#s:u32_pair_struct_spec)
REQUIRES(u32_pair_struct_pred x s)
ENSURES("exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))")
void swap_fields (u32_pair_struct* x)
{
  uint32_t f1 = x->first;
  x->first = x->second;
  x->second = f1;
}

REQUIRES("x |-> 'x")
REQUIRES("y |-> 'y")
ENSURES("x |-> 'y")
ENSURES("y |-> 'x")
void swap_refs (uint32_t* x, uint32_t* y)
{
  uint32_t tmp = *x;
  *x = *y;
  *y = tmp;
}

REQUIRES(emp)
ENSURES(emp)
int main ()
{
  HEAPALLOCATED()u32_pair_struct* x = new_u32_pair_struct();
  swap_fields (x);  //translate the heap allocated Box to a ref
  free(x);
}

// C2PULSE: module Module_0

// C2PULSE: #lang-pulse
// C2PULSE: open Pulse

// C2PULSE: module Box = Pulse.Lib.Box
// C2PULSE: open Module_u32_pair_struct

// C2PULSE: fn new_u32_pair_struct ()
// C2PULSE: requires emp
// C2PULSE: returns x:Box.box u32_pair_struct
// C2PULSE: ensures exists* (s:u32_pair_struct_spec). u32_pair_struct_pred (Box.box_to_ref x) s ** pure (s == {first = 0ul; second = 1ul})
// C2PULSE: {
// C2PULSE: let x = Module_u32_pair_struct.alloc ();
// C2PULSE: Module_u32_pair_struct.set_first (Box.box_to_ref x) 0ul;
// C2PULSE: Module_u32_pair_struct.set_second (Box.box_to_ref x) 1ul;
// C2PULSE: x;
// C2PULSE: }

// C2PULSE: fn swap_fields
// C2PULSE: (#s:u32_pair_struct_spec)
// C2PULSE: (x : ref u32_pair_struct)
// C2PULSE: requires u32_pair_struct_pred x s
// C2PULSE: ensures exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))
// C2PULSE: {
// C2PULSE: let f1 = (Module_u32_pair_struct.get_first x);
// C2PULSE: Module_u32_pair_struct.set_first x (Module_u32_pair_struct.get_second x);
// C2PULSE: Module_u32_pair_struct.set_second x f1;
// C2PULSE: }

// C2PULSE: fn swap_refs
// C2PULSE: (x : ref UInt32.t)
// C2PULSE: (y : ref UInt32.t)
// C2PULSE: requires x |-> 'x
// C2PULSE: requires y |-> 'y
// C2PULSE: ensures x |-> 'y
// C2PULSE: ensures y |-> 'x
// C2PULSE: {
// C2PULSE: let tmp = (! x);
// C2PULSE: x := (! y);
// C2PULSE: y := tmp;
// C2PULSE: }

// C2PULSE: fn main ()
// C2PULSE: requires emp
// C2PULSE: ensures emp
// C2PULSE: {
// C2PULSE: let x = (new_u32_pair_struct ());
// C2PULSE: (swap_fields (Box.box_to_ref x));
// C2PULSE: (free x);
// C2PULSE: }

// COM: ===========================================================================

// C2PULSE_INCLUDE: module Module_u32_pair_struct

// C2PULSE_INCLUDE: #lang-pulse

// C2PULSE_INCLUDE: open Pulse

// C2PULSE_INCLUDE: module Box = Pulse.Lib.Box
// C2PULSE_INCLUDE: val u32_pair_struct : Type0
// C2PULSE_INCLUDE: [@@erasable]
// C2PULSE_INCLUDE: noeq
// C2PULSE_INCLUDE: type u32_pair_struct_spec = {
// C2PULSE_INCLUDE: first : UInt32.t;
// C2PULSE_INCLUDE: second : UInt32.t
// C2PULSE_INCLUDE: }

// C2PULSE_INCLUDE: val u32_pair_struct_pred (_:ref u32_pair_struct) (_:u32_pair_struct_spec) : slprop

// C2PULSE_INCLUDE: fn alloc ()
// C2PULSE_INCLUDE: requires emp
// C2PULSE_INCLUDE: returns x:Box.box u32_pair_struct
// C2PULSE_INCLUDE: ensures exists* v. u32_pair_struct_pred (Box.box_to_ref x) v

// C2PULSE_INCLUDE: fn free
// C2PULSE_INCLUDE: (x : Box.box u32_pair_struct)
// C2PULSE_INCLUDE: requires exists* v. u32_pair_struct_pred (Box.box_to_ref x) v
// C2PULSE_INCLUDE: ensures emp

// C2PULSE_INCLUDE: fn set_first
// C2PULSE_INCLUDE: (x : ref u32_pair_struct)
// C2PULSE_INCLUDE: (f : UInt32.t)
// C2PULSE_INCLUDE: (#s : u32_pair_struct_spec)
// C2PULSE_INCLUDE: requires u32_pair_struct_pred x s
// C2PULSE_INCLUDE: ensures exists* s'. u32_pair_struct_pred x s' ** pure (s' == {s with first=f})

// C2PULSE_INCLUDE: fn get_first
// C2PULSE_INCLUDE: (x : ref u32_pair_struct)
// C2PULSE_INCLUDE: (#s : u32_pair_struct_spec)
// C2PULSE_INCLUDE: requires u32_pair_struct_pred x s
// C2PULSE_INCLUDE: returns f:UInt32.t
// C2PULSE_INCLUDE: ensures u32_pair_struct_pred x s ** pure (f == s.first)
 
// C2PULSE_INCLUDE: fn set_second
// C2PULSE_INCLUDE: (x : ref u32_pair_struct)
// C2PULSE_INCLUDE: (f : UInt32.t)
// C2PULSE_INCLUDE: (#s : u32_pair_struct_spec)
// C2PULSE_INCLUDE: requires u32_pair_struct_pred x s
// C2PULSE_INCLUDE: ensures exists* s'. u32_pair_struct_pred x s' ** pure (s' == {s with second=f})

// C2PULSE_INCLUDE: fn get_second
// C2PULSE_INCLUDE: (x : ref u32_pair_struct)
// C2PULSE_INCLUDE: (#s : u32_pair_struct_spec)
// C2PULSE_INCLUDE: requires u32_pair_struct_pred x s
// C2PULSE_INCLUDE: returns f:UInt32.t
// C2PULSE_INCLUDE: ensures u32_pair_struct_pred x s ** pure (f == s.second)

// C2PULSE_INCLUDE: noeq
// C2PULSE_INCLUDE: type u32_pair_struct_refs = {
// C2PULSE_INCLUDE: first : ref UInt32.t;
// C2PULSE_INCLUDE: second : ref UInt32.t
// C2PULSE_INCLUDE: }

// C2PULSE_INCLUDE: let u32_pair_struct_refs_pred (x:u32_pair_struct_refs) (s:u32_pair_struct_spec) : slprop = (x.first |-> s.first) ** (x.second |-> s.second)

// C2PULSE_INCLUDE: val recover (x:ref u32_pair_struct) (y:u32_pair_struct_refs) : slprop

// C2PULSE_INCLUDE: fn explode
// C2PULSE_INCLUDE: (x : ref u32_pair_struct)
// C2PULSE_INCLUDE: (#s : u32_pair_struct_spec)
// C2PULSE_INCLUDE: requires u32_pair_struct_pred x s
// C2PULSE_INCLUDE: returns y:u32_pair_struct_refs
// C2PULSE_INCLUDE: ensures u32_pair_struct_refs_pred y s
// C2PULSE_INCLUDE: ensures recover x y

// C2PULSE_INCLUDE: ghost
// C2PULSE_INCLUDE: fn restore
// C2PULSE_INCLUDE: (x : ref u32_pair_struct)
// C2PULSE_INCLUDE: (y : u32_pair_struct_refs)
// C2PULSE_INCLUDE: (#s : u32_pair_struct_spec)
// C2PULSE_INCLUDE: requires u32_pair_struct_refs_pred y s
// C2PULSE_INCLUDE: requires recover x y
// C2PULSE_INCLUDE: ensures u32_pair_struct_pred x s

// C2PULSE_INCLUDE: ghost
// C2PULSE_INCLUDE: fn u32_pair_struct_refs_pred_unfold
// C2PULSE_INCLUDE: (x : u32_pair_struct_refs)
// C2PULSE_INCLUDE: (#s : u32_pair_struct_spec)
// C2PULSE_INCLUDE: requires u32_pair_struct_refs_pred x s
// C2PULSE_INCLUDE: ensures x.first |-> s.first
// C2PULSE_INCLUDE: ensures x.second |-> s.second

// C2PULSE_INCLUDE: ghost
// C2PULSE_INCLUDE: fn u32_pair_struct_refs_pred_fold
// C2PULSE_INCLUDE: (x : u32_pair_struct_refs)
// C2PULSE_INCLUDE: (#a0 : erased UInt32.t)
// C2PULSE_INCLUDE: (#a1 : erased UInt32.t)
// C2PULSE_INCLUDE: requires x.first |-> a0
// C2PULSE_INCLUDE: requires x.second |-> a1
// C2PULSE_INCLUDE: ensures u32_pair_struct_refs_pred x ({first = a0;second = a1;})

// COM: ===========================================================================

// PULSE: All verification conditions discharged successfully