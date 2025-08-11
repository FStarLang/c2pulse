#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"
#include "DPE.h"

INCLUDE (
module U8 = FStar.UInt8

noeq
type context_payload_spec =
| PL_Engine : uds:Seq.seq U8.t -> context_payload_spec
| PL_L0     : cdi:Seq.seq U8.t -> context_payload_spec
| PL_L1 //TBD


let owns_payload (c: u_context_t_spec) (s: context_payload_spec) (f:bool) : slprop =
  match c with
  | Case_u_context_t_uds uds ->
    exists* v. 
      (uds |-> v) **
      maybe f (freeable_array uds) **
      pure (s == PL_Engine v)
  | Case_u_context_t_cdi cdi ->
    exists* v.
      (cdi |-> v) **
      maybe f (freeable_array cdi) **
      pure (s == PL_L0 v)
  | Case_u_context_t_l1_context l1 ->
    pure (s == PL_L1)

[@@ pulse_unfold]
let tag_relation (s:context_t_spec): slprop =
  pure (match s.tag with
        | 0uy -> Case_u_context_t_uds? s.payload
        | 1uy -> Case_u_context_t_cdi? s.payload
        | 2uy -> Case_u_context_t_l1_context? s.payload
        | _ -> False)

[@@ pulse_unfold]
let owns_context_t_aux (u: ref context_t) (pl: context_payload_spec) (f:bool) (s:context_t_spec) : slprop =
    context_t_pred u s **
    owns_payload s.payload pl f **
    tag_relation s

[@@ pulse_unfold]
let is_context (u: ref context_t) (pl: context_payload_spec) (f:bool) : slprop =
  exists* s. owns_context_t_aux u pl f s

ghost
fn intro_owns_payload_a (a:u_context_t_spec { Case_u_context_t_uds? a }) (#f:bool) (#v:_) 
requires (Case_u_context_t_uds?._0 a |-> v) ** maybe f (freeable_array (Case_u_context_t_uds?._0 a))
ensures owns_payload (Case_u_context_t_uds (Case_u_context_t_uds?._0 a)) (PL_Engine v) f
{
  fold owns_payload (Case_u_context_t_uds (Case_u_context_t_uds?._0 a)) (PL_Engine v) f
}

ghost
fn intro_owns_payload
    (u:ref context_t)
    (#s:context_t_spec)
    (#pl:u_context_t_spec)
    (#ib:context_payload_spec) #f
requires
    context_t_pred u s **
    owns_payload pl ib f **
    pure (s.payload == pl) **
    tag_relation s
ensures is_context u ib f
{
  rewrite each pl as s.payload;
}
)

ERASED_ARG(#uds_bytes:erased (Seq.seq UInt8.t))
ERASED_ARG(#p:_)
PRESERVES(uds |-> Frac p uds_bytes)
RETURNS(c:ref context_t)
ENSURES(exists* s. is_context c s _true_ ** freeable c ** pure (PL_Engine? s))
context_t* init_engine_context(ISARRAY(UDS_LEN) uint8_t *uds)
{
  ISARRAY(UDS_LEN) uint8_t *uds_buf = (uint8_t*)malloc(sizeof(uint8_t)*UDS_LEN);
  LEMMA(intro_maybe (freeable_array (!uds_buf)));
  context_t *ctx = (context_t*)malloc(sizeof(context_t));
  LEMMA(
    context_t_explode (!ctx);
    u_context_t_change_uds (! (!ctx)).payload;
    u_context_t_explode (! (!ctx)).payload
  );
  ctx->tag = ENGINE_CONTEXT;
  ctx->payload.uds = uds_buf;
  LEMMA(
    intro_owns_payload_a (Case_u_context_t_uds (!uds_buf));
    u_context_t_recover (! (!ctx)).payload #(Case_u_context_t_uds _);
    context_t_recover (!ctx);
    intro_owns_payload (!ctx)
  );
  return ctx;
}
