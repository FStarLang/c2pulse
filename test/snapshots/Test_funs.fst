module Test_funs

#lang-pulse

open Pulse
open Pulse.Lib.C



fn abs_decl
(x : Int32.t)
returns y:Int32.t
ensures pure (Int32.v y > 0)
{
admit();
}

fn foo_decl
(a : Int32.t)
(b : Int32.t)
returns Int32.t
{
admit();
}

fn bar_decl
(c : Int32.t)
returns Int32.t
{
admit();
}

fn bar_impl
(c : Int32.t)
returns Int32.t
{
let mut c : Int32.t = c;
let mut cc : Int32.t = (foo_decl (! c) (! c));
(! cc);
}

fn foo_impl
(a : Int32.t)
(b : Int32.t)
returns Int32.t
{
let mut a : Int32.t = a;
let mut b : Int32.t = b;
let mut abs_a : Int32.t = (abs_decl (! a));
let mut res : Int32.t = (bar_decl (! abs_a));
(! res);
}
