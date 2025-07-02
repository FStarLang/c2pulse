module Pulse.Lib.C.Ref
#lang-pulse
open Pulse
open Pulse.Lib.C.Inhabited
include Pulse.Lib.Reference

instance inhabited_ref (#a:Type) ([@@@mkey] r:ref a) : Pulse.Lib.C.Inhabited.inhabited (ref a) = {
  witness = null
}

val freeable (#a:Type) ([@@@mkey] r:ref a) : slprop

fn alloc_ref (#a:Type) {| inhabited a |} ()
  returns  r : ref a
  ensures  exists* x. r |-> x
  ensures  freeable r

fn free_ref (#a:Type) ([@@@mkey] r:ref a)
  requires freeable r
  requires exists* x. r |-> x