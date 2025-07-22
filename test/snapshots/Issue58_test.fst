module Issue58_test

#lang-pulse

open Pulse
open Pulse.Lib.C



noeq
type engine_record_t = {
l0_image_header_size: ref SizeT.t;
l0_image_header: ref (array UInt8.t);
l0_image_header_sig: ref (array UInt8.t);
l0_binary_size: ref SizeT.t;
l0_binary: ref (array UInt8.t);
l0_binary_hash: ref (array UInt8.t);
l0_image_auth_pubkey: ref (array UInt8.t);
}

[@@erasable]
noeq
type engine_record_t_spec = {
l0_image_header_size : SizeT.t;
l0_image_header : array UInt8.t;
l0_image_header_sig : array UInt8.t;
l0_binary_size : SizeT.t;
l0_binary : array UInt8.t;
l0_binary_hash : array UInt8.t;
l0_image_auth_pubkey : array UInt8.t

}

let engine_record_t_relations (s:engine_record_t_spec) : prop = 
Pulse.Lib.Array.length s.l0_image_header == Pulse.Lib.C.SizeT.as_int s.l0_image_header_size /\
Pulse.Lib.Array.length s.l0_image_header_sig == 64 /\
Pulse.Lib.Array.length s.l0_binary == Pulse.Lib.C.SizeT.as_int s.l0_binary_size /\
Pulse.Lib.Array.length s.l0_binary_hash == 64 /\
Pulse.Lib.Array.length s.l0_image_auth_pubkey == 32

let engine_record_t_pred (x:ref engine_record_t) (s:engine_record_t_spec) : slprop =
exists* (y: engine_record_t). (x |-> y) **
(y.l0_image_header_size |-> s.l0_image_header_size) **
(y.l0_image_header |-> s.l0_image_header) **
(y.l0_image_header_sig |-> s.l0_image_header_sig) **
(y.l0_binary_size |-> s.l0_binary_size) **
(y.l0_binary |-> s.l0_binary) **
(y.l0_binary_hash |-> s.l0_binary_hash) **
(y.l0_image_auth_pubkey |-> s.l0_image_auth_pubkey)
** pure (engine_record_t_relations s)

assume val engine_record_t_spec_default : engine_record_t_spec

assume val engine_record_t_default (engine_record_t_var_spec:engine_record_t_spec) : engine_record_t

ghost
fn engine_record_t_pack (engine_record_t_var:ref engine_record_t) (#engine_record_t_var_spec:engine_record_t_spec)
requires engine_record_t_var|-> engine_record_t_default engine_record_t_var_spec
ensures exists* v. engine_record_t_pred engine_record_t_var v ** pure (v == engine_record_t_var_spec)
{ admit() }

ghost
fn engine_record_t_unpack (engine_record_t_var:ref engine_record_t)
requires exists* v. engine_record_t_pred engine_record_t_var v 
ensures exists* u. engine_record_t_var |-> u
{ admit() }

fn engine_record_t_alloc ()
returns x:ref engine_record_t
ensures freeable x
ensures exists* v. engine_record_t_pred x v
{ admit () }

fn engine_record_t_free (x:ref engine_record_t)
requires freeable x
requires exists* v. engine_record_t_pred x v
{ admit() }


ghost fn engine_record_t_explode (x:ref engine_record_t) (#s:engine_record_t_spec)
requires engine_record_t_pred x s
ensures exists* (v: engine_record_t). (x |-> v) ** (v.l0_image_header_size |-> s.l0_image_header_size) ** 
(v.l0_image_header |-> s.l0_image_header) ** 
(v.l0_image_header_sig |-> s.l0_image_header_sig) ** 
(v.l0_binary_size |-> s.l0_binary_size) ** 
(v.l0_binary |-> s.l0_binary) ** 
(v.l0_binary_hash |-> s.l0_binary_hash) ** 
(v.l0_image_auth_pubkey |-> s.l0_image_auth_pubkey)

** pure (engine_record_t_relations s)
{unfold engine_record_t_pred}


ghost
fn engine_record_t_recover (x:ref engine_record_t) (#a0 : SizeT.t)
(#a1 : (array UInt8.t))
(#a2 : (array UInt8.t))
(#a3 : SizeT.t)
(#a4 : (array UInt8.t))
(#a5 : (array UInt8.t))
(#a6 : (array UInt8.t))

requires exists* (y: engine_record_t). (x |-> y) ** 
(y.l0_image_header_size |-> a0) **
(y.l0_image_header |-> a1) **
(y.l0_image_header_sig |-> a2) **
(y.l0_binary_size |-> a3) **
(y.l0_binary |-> a4) **
(y.l0_binary_hash |-> a5) **
(y.l0_image_auth_pubkey |-> a6)** pure (engine_record_t_relations {
l0_image_header_size = a0;
l0_image_header = a1;
l0_image_header_sig = a2;
l0_binary_size = a3;
l0_binary = a4;
l0_binary_hash = a5;
l0_image_auth_pubkey = a6})

ensures exists* w. engine_record_t_pred x w ** pure (w == {l0_image_header_size = a0;
l0_image_header = a1;
l0_image_header_sig = a2;
l0_binary_size = a3;
l0_binary = a4;
l0_binary_hash = a5;
l0_image_auth_pubkey = a6})
{fold engine_record_t_pred x ({l0_image_header_size = a0;
l0_image_header = a1;
l0_image_header_sig = a2;
l0_binary_size = a3;
l0_binary = a4;
l0_binary_hash = a5;
l0_image_auth_pubkey = a6}) }
