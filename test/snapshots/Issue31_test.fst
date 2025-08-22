module Issue31_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn check_null
(r : ( ref Int32.t) )
requires 
emp
returns i:int32
ensures 
emp
{
let mut r : (ref Int32.t) = r;
if((int32_to_bool (bool_to_int32 (is_null (! r)))))
{
0l;
}
else
{
1l;
};
}
