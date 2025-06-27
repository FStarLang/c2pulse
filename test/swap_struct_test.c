// RUN: %c2pulse %s 
// RUN: cat %p/Swap_struct_test/Module_0.fst | %{FILECHECK} %s --check-prefix=C2PULSE
// RUN: %run_fstar.sh %p/Swap_struct_test/Module_0.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE

#include <stdint.h>
#include <stdlib.h>
#include "pulse_macros.h"

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

// C2PULSE: module Module_0

// C2PULSE: #lang-pulse

// C2PULSE: open Pulse



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

// C2PULSE: assume val u32_pair_struct_allocated (x: ref u32_pair_struct) : slprop

// C2PULSE: fn u32_pair_struct_alloc ()
// C2PULSE: returns x:ref u32_pair_struct
// C2PULSE: ensures u32_pair_struct_allocated x
// C2PULSE: ensures exists* v. u32_pair_struct_pred x v
// C2PULSE: { admit () }

// C2PULSE: fn u32_pair_struct_free (x:ref u32_pair_struct)
// C2PULSE: requires u32_pair_struct_allocated x
// C2PULSE: requires exists* v. u32_pair_struct_pred x v
// C2PULSE: { admit() }


// C2PULSE: ghost fn u32_pair_struct_explode (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
// C2PULSE: requires u32_pair_struct_pred x s
// C2PULSE: ensures exists* (v: u32_pair_struct). (x |-> ({first=v.first; second=v.second} <: u32_pair_struct))
// C2PULSE:  ** (v.first |-> s.first) ** (v.second |-> s.second)
// C2PULSE: {unfold u32_pair_struct_pred}


// C2PULSE: fn u32_pair_struct_to_first (x: ref u32_pair_struct) (#first #second :erased _)
// C2PULSE: requires x |-> ({ first;second } <: u32_pair_struct)
// C2PULSE: requires reveal first |-> 'y
// C2PULSE: returns first': ref UInt32.t
// C2PULSE: ensures (x |-> ({first=first'; second } <: u32_pair_struct))
// C2PULSE: ensures first' |-> 'y
// C2PULSE: ensures pure (first == first')
// C2PULSE: { let vx' = !x; rewrite each first as vx'.first; vx'.first }

// C2PULSE: fn u32_pair_struct_to_second (x: ref u32_pair_struct) (#first #second :erased _)
// C2PULSE: requires x |-> ({ first;second } <: u32_pair_struct)
// C2PULSE: requires reveal second |-> 'y
// C2PULSE: returns second': ref UInt32.t
// C2PULSE: ensures (x |-> ({first; second=second' } <: u32_pair_struct))
// C2PULSE: ensures second' |-> 'y
// C2PULSE: ensures pure (second == second')
// C2PULSE: { let vx' = !x; rewrite each second as vx'.second; vx'.second }

// C2PULSE: fn u32_pair_struct_get_first (x: ref u32_pair_struct) (#first #second :erased _)
// C2PULSE: requires x |-> ({ first;second } <: u32_pair_struct)
// C2PULSE: requires reveal first |-> 'y
// C2PULSE: returns first': UInt32.t
// C2PULSE: ensures (x |-> ({ first;second } <: u32_pair_struct))
// C2PULSE: ensures reveal first |-> 'y
// C2PULSE: ensures pure ('y == first')
// C2PULSE: { let vfirst = u32_pair_struct_to_first x; let ret = !vfirst; rewrite each vfirst as first; ret }

// C2PULSE: fn u32_pair_struct_get_second (x: ref u32_pair_struct) (#first #second :erased _)
// C2PULSE: requires x |-> ({ first;second } <: u32_pair_struct)
// C2PULSE: requires reveal second |-> 'y
// C2PULSE: returns second': UInt32.t
// C2PULSE: ensures (x |-> ({ first;second } <: u32_pair_struct))
// C2PULSE: ensures reveal second |-> 'y
// C2PULSE: ensures pure ('y == second')
// C2PULSE: { let vsecond = u32_pair_struct_to_second x; let ret = !vsecond; rewrite each vsecond as second; ret }

// C2PULSE: fn u32_pair_struct_set_first (x: ref u32_pair_struct) (#first #second :erased _) (first': UInt32.t)
// C2PULSE: requires x |-> ({ first;second } <: u32_pair_struct)
// C2PULSE: requires reveal first |-> 'y
// C2PULSE: ensures (x |-> ({ first;second } <: u32_pair_struct))
// C2PULSE: ensures reveal first |-> first'
// C2PULSE: { let vfirst = u32_pair_struct_to_first x;vfirst:=first'; rewrite each vfirst as first }

// C2PULSE: fn u32_pair_struct_set_second (x: ref u32_pair_struct) (#first #second :erased _) (second': UInt32.t)
// C2PULSE: requires x |-> ({ first;second } <: u32_pair_struct)
// C2PULSE: requires reveal second |-> 'y
// C2PULSE: ensures (x |-> ({ first;second } <: u32_pair_struct))
// C2PULSE: ensures reveal second |-> second'
// C2PULSE: { let vsecond = u32_pair_struct_to_second x;vsecond:=second'; rewrite each vsecond as second }

// C2PULSE: ghost
// C2PULSE: fn u32_pair_struct_recover (x:ref u32_pair_struct) (#a0 : UInt32.t) (#a1 : UInt32.t) 
// C2PULSE: requires exists* (y: u32_pair_struct). (x |-> y) ** (y.first |-> a0) ** (y.second |-> a1)
// C2PULSE: ensures u32_pair_struct_pred x ({first = a0; second = a1})
// C2PULSE: {fold u32_pair_struct_pred x ({first = a0; second = a1}) }

// COM: ===========================================================================

// PULSE: All verification conditions discharged successfully