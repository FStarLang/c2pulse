module Issue64_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn foo ()
{
let mut size_expr0 : SizeT.t = (uint64_to_sizet (int32_to_uint64 32l));
let mut bytes : (array UInt8.t) = alloc_array #UInt8.t !size_expr0;
let mut size_expr1 : SizeT.t = (uint64_to_sizet (int32_to_uint64 32l));
let mut bytes1 : (array UInt8.t) = alloc_array #UInt8.t !size_expr1;
(free_array #UInt8.t (! bytes));
(free_array #UInt8.t (! bytes1));
}
