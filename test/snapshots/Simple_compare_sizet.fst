module Simple_compare_sizet

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo
(i : UInt64.t)
(j : UInt64.t)
returns _Bool
{
let mut i : UInt64.t = i;
let mut j : UInt64.t = j;
if((int32_to_bool (bool_to_int32 (UInt64.lt (! i) (! j)))))
{
(int32_to_bool 1l);
}
else
{
(int32_to_bool 0l);
};
}

fn bar
(i : UInt64.t)
(j : UInt64.t)
returns _Bool
{
let mut i : UInt64.t = i;
let mut j : UInt64.t = j;
if((int32_to_bool (bool_to_int32 (UInt64.lt (! i) (! j)))))
{
(int32_to_bool 1l);
}
else
{
(int32_to_bool 0l);
};
}

fn baz
(i : UInt32.t)
(j : UInt64.t)
returns _Bool
{
let mut i : UInt32.t = i;
let mut j : UInt64.t = j;
if((int32_to_bool (bool_to_int32 (UInt64.lt (uint32_to_uint64 (! i)) (! j)))))
{
(int32_to_bool 1l);
}
else
{
(int32_to_bool 0l);
};
}

fn boo
(i : UInt64.t)
(j : UInt32.t)
returns _Bool
{
let mut i : UInt64.t = i;
let mut j : UInt32.t = j;
if((int32_to_bool (bool_to_int32 (UInt64.lt (! i) (uint32_to_uint64 (! j))))))
{
(int32_to_bool 1l);
}
else
{
(int32_to_bool 0l);
};
}
