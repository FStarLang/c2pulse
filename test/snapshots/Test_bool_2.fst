module Test_bool_2

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo
(p : Int64.t)
{
if((int32_to_bool (bool_to_int32 (op_AmpAmp (int64_to_bool p) (int64_to_bool p)))))
{
()
}
else
{
()
};
}
