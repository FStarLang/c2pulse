// RUN: %c2pulse %s 
// RUN: cat %p/Simple_struct_test/Module_u32_pair_struct.fsti | %{FILECHECK} %s --check-prefix=C2PULSE
// RUN: %run_fstar.sh %p/Simple_struct_test/Module_u32_pair_struct.fsti 2>&1 | %{FILECHECK} %s --check-prefix=PULSE

#include <stdint.h>
#include <stdlib.h>
#include "../../test-transpiler/c/pulse_macros.h"

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;

// C2PULSE: module Module_u32_pair_struct
// C2PULSE: #lang-pulse
// C2PULSE: open Pulse
// C2PULSE: module Box = Pulse.Lib.Box

// C2PULSE: val u32_pair_struct : Type0

// C2PULSE: [@@erasable]
// C2PULSE: noeq
// C2PULSE: type u32_pair_struct_spec = {
// C2PULSE: first : UInt32.t;
// C2PULSE: second : UInt32.t

// C2PULSE: }

// C2PULSE: val u32_pair_struct_pred (_:ref u32_pair_struct) (_:u32_pair_struct_spec) : slprop

// C2PULSE: fn alloc ()
// C2PULSE: requires emp
// C2PULSE: returns x:Box.box u32_pair_struct
// C2PULSE: ensures exists* v. u32_pair_struct_pred (Box.box_to_ref x) v

// C2PULSE: fn free
// C2PULSE: (x : Box.box u32_pair_struct)
// C2PULSE: requires exists* v. u32_pair_struct_pred (Box.box_to_ref x) v
// C2PULSE: ensures emp

// C2PULSE: fn set_first
// C2PULSE: (x : ref u32_pair_struct)
// C2PULSE: (f : UInt32.t)
// C2PULSE: (#s : u32_pair_struct_spec)
// C2PULSE: requires u32_pair_struct_pred x s
// C2PULSE: ensures exists* s'. u32_pair_struct_pred x s' ** pure (s' == {s with first=f})

// C2PULSE: fn get_first
// C2PULSE: (x : ref u32_pair_struct)
// C2PULSE: (#s : u32_pair_struct_spec)
// C2PULSE: requires u32_pair_struct_pred x s
// C2PULSE: returns f:UInt32.t
// C2PULSE: ensures u32_pair_struct_pred x s ** pure (f == s.first)

// C2PULSE: fn set_second
// C2PULSE: (x : ref u32_pair_struct)
// C2PULSE: (f : UInt32.t)
// C2PULSE: (#s : u32_pair_struct_spec)
// C2PULSE: requires u32_pair_struct_pred x s
// C2PULSE: ensures exists* s'. u32_pair_struct_pred x s' ** pure (s' == {s with second=f})

// C2PULSE: fn get_second
// C2PULSE: (x : ref u32_pair_struct)
// C2PULSE: (#s : u32_pair_struct_spec)
// C2PULSE: requires u32_pair_struct_pred x s
// C2PULSE: returns f:UInt32.t
// C2PULSE: ensures u32_pair_struct_pred x s ** pure (f == s.second)

// C2PULSE: noeq
// C2PULSE: type u32_pair_struct_refs = {
// C2PULSE: first : ref UInt32.t;
// C2PULSE: second : ref UInt32.t

// C2PULSE: }

// C2PULSE: let u32_pair_struct_refs_pred (x:u32_pair_struct_refs) (s:u32_pair_struct_spec) : slprop = (x.first |-> s.first) ** (x.second |-> s.second)

// C2PULSE: val recover (x:ref u32_pair_struct) (y:u32_pair_struct_refs) : slprop

// C2PULSE: fn explode
// C2PULSE: (x : ref u32_pair_struct)
// C2PULSE: (#s : u32_pair_struct_spec)
// C2PULSE: requires u32_pair_struct_pred x s
// C2PULSE: returns y:u32_pair_struct_refs
// C2PULSE: ensures u32_pair_struct_refs_pred y s
// C2PULSE: ensures recover x y

// C2PULSE: ghost
// C2PULSE: fn restore
// C2PULSE: (x : ref u32_pair_struct)
// C2PULSE: (y : u32_pair_struct_refs)
// C2PULSE: (#s : u32_pair_struct_spec)
// C2PULSE: requires u32_pair_struct_refs_pred y s
// C2PULSE: requires recover x y
// C2PULSE: ensures u32_pair_struct_pred x s

// C2PULSE: ghost
// C2PULSE: fn u32_pair_struct_refs_pred_unfold
// C2PULSE: (x : u32_pair_struct_refs)
// C2PULSE: (#s : u32_pair_struct_spec)
// C2PULSE: requires u32_pair_struct_refs_pred x s
// C2PULSE: ensures x.first |-> s.first
// C2PULSE: ensures x.second |-> s.second

// C2PULSE: ghost
// C2PULSE: fn u32_pair_struct_refs_pred_fold
// C2PULSE: (x : u32_pair_struct_refs)
// C2PULSE: (#a0 : erased UInt32.t)
// C2PULSE: (#a1 : erased UInt32.t)
// C2PULSE: requires x.first |-> a0
// C2PULSE: requires x.second |-> a1
// C2PULSE: ensures u32_pair_struct_refs_pred x ({first = a0;second = a1;})

// COM: ===========================================================================

// PULSE: All verification conditions discharged successfully