module Issue_anon_1

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn test
(x : Int32.t)
(y : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
let mut k : Int32.t = 1l;
k := 4l;
((bool_to_int32 (op_AmpAmp (int32_to_bool (bool_to_int32 (op_AmpAmp (int32_to_bool (! x)) (int32_to_bool (! y))))) (int32_to_bool (! k)))));
}
