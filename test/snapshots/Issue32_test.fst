module Issue32_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn check_null
(r : ( ref Int32.t) )
requires 
emp
returns i : int32
ensures 
emp
{
let mut r : (ref Int32.t) = r;
if((int32_to_bool (bool_to_int32 (not (is_null (! r))))))
{
1l;
}
else
{
0l;
};
}
