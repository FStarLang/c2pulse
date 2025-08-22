module Issue39

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn test0
(x : Int32.t)
returns res : Int32.t
ensures 
pure (Int32.v res == 1)
{
let mut x : Int32.t = x;
x := 1l;
(! x);
}

fn test1
(x : Int32.t)
returns res : Int32.t
ensures 
pure (Int32.v res == 1)
{
let mut x : Int32.t = x;
((x)) := 1l;
(! x);
}

fn test2
(x : Int32.t)
returns res : Int32.t
ensures 
pure (Int32.v res == 1)
{
let mut x : Int32.t = x;
let mut p : (ref Int32.t) = (x);
(! p) := 1l;
(! (! p));
}

fn test3
(x : Int32.t)
returns res : Int32.t
ensures 
pure (Int32.v res == 1)
{
let mut x : Int32.t = x;
let mut p : (ref Int32.t) = (x);
(! p) := 1l;
(! (! p));
}

fn test4
(x : Int32.t)
returns res : Int32.t
ensures 
pure (Int32.v res == 1)
{
let mut x : Int32.t = x;
let mut p : (ref Int32.t) = (x);
(! p) := 1l;
(! x);
}

fn test5
(x : Int32.t)
returns res : Int32.t
ensures 
pure (Int32.v res == 1)
{
let mut x : Int32.t = x;
let mut p : (ref Int32.t) = (x);
if((int32_to_bool 1l))
{
(! p) := 1l;
}
else
{
()
};
(! x);
}
