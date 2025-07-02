// RUN: %c2pulse %s 
// RUN: cat %p/Swap_struct_test.fst | %{FILECHECK} %s --check-prefix=C2PULSE
// RUN: %run_fstar.sh %p/Swap_struct_test.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE

#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h" 

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;


REQUIRES(emp)
RETURNS(x:ref u32_pair_struct)
ENSURES(u32_pair_struct_allocated x)
ENSURES((u32_pair_struct_pred x { first = 0ul; second = 1ul }))
u32_pair_struct* new_u32_pair_struct ()
{
  u32_pair_struct* x = ( u32_pair_struct* )malloc(sizeof(u32_pair_struct));
  x->first = 0ul;
  x->second = 1ul;
  return x;
}

ERASED_ARG(#s : u32_pair_struct_spec)
REQUIRES(u32_pair_struct_pred x s)
ENSURES("exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == ({first = s.second; second = s.first}))")
void swap_fields(u32_pair_struct *x) {
  uint32_t f1 = x->first;
  x->first = x->second;
  x->second = f1;
}

REQUIRES("x |-> 'x")
REQUIRES("y |-> 'y")
ENSURES("x |-> 'y")
ENSURES("y |-> 'x")
void swap_refs(uint32_t *x, uint32_t *y) {
  uint32_t tmp = *x;
  *x = *y;
  *y = tmp;
}

ERASED_ARG(#s : u32_pair_struct_spec)
REQUIRES(u32_pair_struct_pred x s)
ENSURES("exists* (s':u32_pair_struct_spec). u32_pair_struct_pred x s' ** pure (s' == {first = s.second; second = s.first})")
void swap_fields_alt(u32_pair_struct *x) { swap_refs(&x->first, &x->second); }

void main() {
  u32_pair_struct *x = new_u32_pair_struct();
  swap_fields(x);
  swap_fields_alt(x);
  ASSERT(u32_pair_struct_pred x {first = 0ul; second = 1ul});
  free(x);
}

// C2PULSE: module Swap_struct
// C2PULSE: #lang-pulse
// C2PULSE: open Pulse
// C2PULSE: open Pulse.Lib.C
// C2PULSE: module Box = Pulse.Lib.Box


// C2PULSE: noeq
// C2PULSE: type u32_pair_struct = {
// C2PULSE: first: ref UInt32.t;
// C2PULSE: second: ref UInt32.t;
// C2PULSE: }

// C2PULSE: [@@erasable]
// C2PULSE: noeq
// C2PULSE: type u32_pair_struct_spec = {
// C2PULSE: first : UInt32.t;
// C2PULSE: second : UInt32.t
// C2PULSE: }

// C2PULSE: let u32_pair_struct_pred (x:ref u32_pair_struct) (s:u32_pair_struct_spec) : slprop =
// C2PULSE: exists* (y: u32_pair_struct). (x |-> y) **
// C2PULSE: (y.first |-> s.first) **
// C2PULSE: (y.second |-> s.second)

// C2PULSE: fn u32_pair_struct_alloc ()
// C2PULSE: returns x:ref u32_pair_struct
// C2PULSE: { admit () }

// C2PULSE: fn u32_pair_struct_free (x:ref u32_pair_struct)
// C2PULSE: { admit() }

// C2PULSE: ghost fn u32_pair_struct_explode (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
// C2PULSE: {unfold u32_pair_struct_pred}

// C2PULSE: ghost
// C2PULSE: fn u32_pair_struct_recover (x:ref u32_pair_struct) (#a0 : UInt32.t) (#a1 : UInt32.t) 
// C2PULSE: {fold u32_pair_struct_pred x ({first = a0; second = a1}) }

// C2PULSE: fn new_u32_pair_struct ()
// C2PULSE: {
// C2PULSE: let x = u32_pair_struct_alloc ();
// C2PULSE: u32_pair_struct_explode x;
// C2PULSE: Mku32_pair_struct?.first (! x) := 0ul;
// C2PULSE: Mku32_pair_struct?.second (! x) := 1ul;
// C2PULSE: u32_pair_struct_recover x;
// C2PULSE: x;
// C2PULSE: }

// C2PULSE: fn swap_fields
// C2PULSE: (x : ref u32_pair_struct)
// C2PULSE: (#s : u32_pair_struct_spec)
// C2PULSE: {
// C2PULSE: u32_pair_struct_explode x;
// C2PULSE: let f1 = (!(!x).first);
// C2PULSE: Mku32_pair_struct?.first (! x) := (!(!x).second);
// C2PULSE: Mku32_pair_struct?.second (! x) := f1;
// C2PULSE: u32_pair_struct_recover x;
// C2PULSE: }

// C2PULSE: fn swap_refs

// C2PULSE: fn swap_fields_alt
// C2PULSE: (x : ref u32_pair_struct)
// C2PULSE: (#s : u32_pair_struct_spec)
// C2PULSE: {
// C2PULSE: u32_pair_struct_explode x;
// C2PULSE: (swap_refs (!x).first (!x).second);
// C2PULSE: u32_pair_struct_recover x;
// C2PULSE: }

// C2PULSE: fn main ()
// C2PULSE: {
// C2PULSE: let x = (new_u32_pair_struct ());
// C2PULSE: (swap_fields x);
// C2PULSE: (swap_fields_alt x);
// C2PULSE: assert(u32_pair_struct_pred x {first = 0ul; second = 1ul});
// C2PULSE: (u32_pair_struct_free x);
// C2PULSE: }

// COM: ===========================================================================

// PULSE: All verification conditions discharged successfully
