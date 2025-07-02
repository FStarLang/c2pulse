// RUN: %c2pulse %s 
// RUN: cat %p/Simple_nested_struct_test.fst | %{FILECHECK} %s --check-prefix=C2PULSE
// RUN: %run_fstar.sh %p/Simple_nested_struct_test.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE

#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;

typedef struct _u64_pair_struct {
  uint64_t first;
  u32_pair_struct *second;
} u64_pair_struct;

// C2PULSE: module Simple_nested_struct_test
// C2PULSE: #lang-pulse
// C2PULSE: open Pulse
// C2PULSE: open Pulse.Lib.C


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


// C2PULSE: assume val u32_pair_struct_allocated (x: ref u32_pair_struct) : slprop

// C2PULSE: fn u32_pair_struct_alloc ()
// C2PULSE: { admit () }

// C2PULSE: fn u32_pair_struct_free (x:ref u32_pair_struct)
// C2PULSE: { admit() }


// C2PULSE: ghost fn u32_pair_struct_explode (x:ref u32_pair_struct) (#s:u32_pair_struct_spec)
// C2PULSE: {unfold u32_pair_struct_pred}


// C2PULSE: ghost
// C2PULSE: fn u32_pair_struct_recover (x:ref u32_pair_struct) (#a0 : UInt32.t) (#a1 : UInt32.t) 
// C2PULSE: {fold u32_pair_struct_pred x ({first = a0; second = a1}) }

// C2PULSE: noeq
// C2PULSE: type u64_pair_struct = {
// C2PULSE: first: ref UInt64.t;
// C2PULSE: second: ref (ref u32_pair_struct);
// C2PULSE: }

// C2PULSE: [@@erasable]
// C2PULSE: noeq
// C2PULSE: type u64_pair_struct_spec = {
// C2PULSE: first : UInt64.t;
// C2PULSE: second : ref u32_pair_struct
// C2PULSE: }

// C2PULSE: let u64_pair_struct_pred (x:ref u64_pair_struct) (s:u64_pair_struct_spec) : slprop =
// C2PULSE: exists* (y: u64_pair_struct). (x |-> y) **
// C2PULSE: (y.first |-> s.first) **
// C2PULSE: (y.second |-> s.second)

// C2PULSE: assume val u64_pair_struct_allocated (x: ref u64_pair_struct) : slprop

// C2PULSE: fn u64_pair_struct_alloc ()
// C2PULSE: { admit () }

// C2PULSE: fn u64_pair_struct_free (x:ref u64_pair_struct)
// C2PULSE: { admit() }


// C2PULSE: ghost fn u64_pair_struct_explode (x:ref u64_pair_struct) (#s:u64_pair_struct_spec)
// C2PULSE: {unfold u64_pair_struct_pred}


// C2PULSE: ghost
// C2PULSE: fn u64_pair_struct_recover (x:ref u64_pair_struct) (#a0 : UInt64.t) (#a1 : (ref u32_pair_struct)) 
// C2PULSE: {fold u64_pair_struct_pred x ({first = a0; second = a1}) }

// COM: ===========================================================================

// PULSE: All verification conditions discharged successfully
