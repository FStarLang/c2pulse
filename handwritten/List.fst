module List

#lang-pulse
open Pulse

module Box = Pulse.Lib.Box
open Pulse.Lib.Box { box }

noeq
type node (t:Type0) = {
    head : t;
    tail : option (box (node t));
}

and llist (t:Type0) = option (box (node t))

(* Hm... we cannot prove termination of this predicate
as it is not decreasing a list in any way. Maybe we should
be define it coinductively? Admit the termination, for now. *)
[@@admit_termination]
let rec list_live #t (x:llist t)
  = match x with
    | None -> emp
    | Some v -> 
      exists* (n : node t).
        pts_to v n **
        list_live n.tail

fn push #t (x : t) (l : llist t)
  requires list_live l
  returns  l' : llist t
  ensures  list_live l'
{
  let n = Box.alloc { head = x; tail = l };
  fold (list_live (Some n));
  Some n
}

fn peek #t (l : llist t)
  requires list_live l 
  requires pure (Some? l)
  returns  x : t
  ensures  list_live l
{
  unfold (list_live l);
  let Some v = l;
  let n = Box.op_Bang v;
  fold (list_live (Some v));
  n.head
}

fn pop #t (l : llist t)
  requires pure (Some? l)
  requires list_live l
  returns  l' : llist t
  ensures  list_live l'
{
  unfold (list_live l);
  let Some v = l;
  let n = Box.op_Bang v;
  Box.free v;
  n.tail
}

fn is_empty #t (l : llist t)
  requires list_live l
  returns  b : bool
  ensures  list_live l
{
  (None? l)
}
