module Test_bool_2

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn foo
(p : Int64.t)
returns Int32.t
{
let mut p : Int64.t = p;
if((int32_to_bool (bool_to_int32 (op_AmpAmp (int64_to_bool (! p)) (int64_to_bool (! p))))))
{
()
}
else
{
()
};
0l;
}
