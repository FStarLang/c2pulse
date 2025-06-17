module Mem

#lang-pulse
open Pulse

module Box = Pulse.Lib.Box
module Ref = Pulse.Lib.Reference
module Array = Pulse.Lib.Array

// Both Box and Ref define alloc, (!), (:=) and free.
// In this file I'm using full names for all of them.

(*
int box(int x)
{
  int *r = malloc(sizeof(int));
  *r = *r + *r;
  int v = *r;
  free(r);
  return v;
}
*)
fn box (x : int)
  returns int
{
  let r = Box.alloc x;
  Box.op_Colon_Equals r (Box.op_Bang r + Box.op_Bang r);
  let v = Box.op_Bang r;
  Box.free r;
  v
}

(*
int local(int x)
{
  int r = x;
  r = r + r;
  int v = r;
  return v;
}
*)
fn local (x : int)
  returns int
{
  let mut r = x;
  Ref.op_Colon_Equals r (Ref.op_Bang r + Ref.op_Bang r);
  let v = Ref.op_Bang r;
  v
}

fn heap_arr (x : int)
  returns int
{
  let a = Array.alloc x 100sz;
  a.(0sz) <- a.(0sz) + a.(0sz);
  let v = a.(0sz);
  Array.free a;
  v
}

fn local_arr (x : int)
  returns int
{
  let mut a = [| x; 100sz |];
  a.(0sz) <- a.(0sz) + a.(0sz);
  let v = a.(0sz);
  v
}

