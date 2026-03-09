#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "DPE.h"
#include "EngineCore.h"

_include_pulse(
  $declare(context_t s)
  [@@erasable]
  noeq type context_full_data =
    | PL_Engine of (Seq.seq UInt8.t)
    | PL_L0 of (Seq.seq UInt8.t)
    | PL_L1 // tbd

  let tag_relation ($(s): $type(context_t)) (h: context_full_data) : prop =
    match $(s.tag) with
    | 0uy -> Field__u_context_t__uds? $(s.payload) /\ PL_Engine? h
    | 1uy -> Field__u_context_t__cdi? $(s.payload) /\ PL_L0? h
    | 2uy -> Field__u_context_t__l1_context? $(s.payload) /\ PL_L1? h
    | _ -> False
)

_include_pulse(
  $declare(context_t s)

  let context_full_pred ([@@@mkey] $(s): $type(context_t)) (h: context_full_data) : slprop =
    match $(s.payload), h with
    | Field__u_context_t__uds uds_ptr, PL_Engine uds_data ->
      uds_ptr |-> uds_data ** freeable_array uds_ptr
    | Field__u_context_t__cdi cdi_ptr, PL_L0 cdi_data ->
      cdi_ptr |-> cdi_data ** freeable_array cdi_ptr
    | Field__u_context_t__l1_context _, PL_L1 ->
      emp
    | _ -> pure False

  let engine_state ($(s): $type(context_t)) #x =
    observe (context_full_pred $(s)) #x

  ghost fn elim_context_full_pred_uds ($(s): $type(context_t)) (#h: context_full_data)
    requires with_pure (tag_relation $(s) h /\ PL_Engine? h)
    requires context_full_pred $(s) h
    ensures Pulse.Lib.Array.pts_to $(s.payload.uds) (PL_Engine?._0 h)
    ensures freeable_array $(s.payload.uds)
  {
    unfold context_full_pred;
    rewrite each $(s.payload) as Field__u_context_t__uds $(s.payload.uds);
    rewrite each h as PL_Engine (PL_Engine?._0 h);
  }
  
  ghost fn elim_context_full_pred_cdi ($(s): $type(context_t)) (#h: context_full_data)
    requires with_pure (tag_relation $(s) h /\ PL_L0? h)
    requires context_full_pred $(s) h
    ensures Pulse.Lib.Array.pts_to $(s.payload.cdi) (PL_L0?._0 h)
    ensures freeable_array $(s.payload.cdi)
  {
    unfold context_full_pred;
    rewrite each $(s.payload) as Field__u_context_t__cdi $(s.payload.cdi);
    rewrite each h as PL_L0 (PL_L0?._0 h);
  }
  
  ghost fn elim_context_full_pred_l1 ($(s): $type(context_t)) (#h: context_full_data)
    requires with_pure (tag_relation $(s) h /\ PL_L1? h)
    requires context_full_pred $(s) h
  {
    unfold context_full_pred;
    rewrite each $(s.payload) as Field__u_context_t__l1_context $(s.payload.l1_context);
    rewrite each h as PL_L1;
  }

  ghost fn intro_context_full_pred_uds ($(s): $type(context_t)) #uds
    requires with_pure (tag_relation $(s) (PL_Engine uds))
    requires Pulse.Lib.Array.pts_to $(s.payload.uds) uds
    requires freeable_array $(s.payload.uds)
    ensures context_full_pred $(s) (PL_Engine uds)
  {
    rewrite $(s.payload.uds) |-> uds ** freeable_array $(s.payload.uds)
      as context_full_pred $(s) (PL_Engine uds);
  }

  ghost fn intro_context_full_pred_cdi ($(s): $type(context_t)) #cdi
    requires with_pure (tag_relation $(s) (PL_L0 cdi))
    requires Pulse.Lib.Array.pts_to $(s.payload.cdi) cdi
    requires freeable_array $(s.payload.cdi)
    ensures context_full_pred $(s) (PL_L0 cdi)
  {
    rewrite $(s.payload.cdi) |-> cdi ** freeable_array $(s.payload.cdi)
      as context_full_pred $(s) (PL_L0 cdi);
  }
)

void memcpy_(size_t len, _array const uint8_t *a1, _array uint8_t *a2)
  _preserves(a1._length == len)
  _preserves(a2._length == len)
  _ensures((_slprop) _inline_pulse(rewrites_to (value_of $(a2)) (value_of $(a1))))
{
  _ghost_stmt(admit());
}

void free_(_consumes _allocated_array _array uint8_t *arr)
{
  _ghost_stmt(admit());
}

_refine((_slprop) _inline_pulse(
  exists* state.
    pure (tag_relation $(*this) state) **
    context_full_pred $(*this) state))
typedef context_t *context_obj;

_allocated
typedef context_obj allocated_context_obj;
allocated_context_obj init_engine_context(const uds_array uds)
  _ensures((bool) _inline_pulse(engine_state $(*return) == PL_Engine (value_of $(uds))))
{
  uint8_t *uds_buf = (uint8_t*)calloc(UDS_LEN, sizeof(uint8_t));
  memcpy_(UDS_LEN, uds, uds_buf);
  context_t *ctx = (context_t*)malloc(sizeof(context_t));
  *ctx = (context_t) {
    .tag = ENGINE_CONTEXT,
    .payload = (u_context_t) { .uds = uds_buf },
  };
  _ghost_stmt(intro_context_full_pred_uds $(*ctx));
  return ctx;
}

_include_pulse (
  ghost fn elim_maybe_true (p:slprop)
  requires maybe _true_ p
  ensures p
  { unfold maybe; }
)

void init_l0_context(context_obj ctx, const dice_digest cdi)
  _requires((bool) _inline_pulse(PL_Engine? (engine_state $(*ctx))))
  _ensures((bool) _inline_pulse(engine_state $(*ctx) == PL_L0 (value_of $(cdi))))
{
  uint8_t *cdi_buf = (uint8_t*)calloc(DICE_DIGEST_LEN, sizeof(uint8_t));
  memcpy_(DICE_DIGEST_LEN, cdi, cdi_buf);
  _ghost_stmt(elim_context_full_pred_uds $(*ctx));
  uint8_t* uds_buf = ctx->payload.uds;
  free_(uds_buf);
  ctx->tag = 1;
  ctx->payload.cdi = cdi_buf;
  _ghost_stmt(intro_context_full_pred_cdi $(*ctx));
  return;
}

void destroy_uds_context(_consumes _allocated context_obj ctx)
  _requires(ctx->tag == 0)
{
  _ghost_stmt(elim_context_full_pred_uds $(*ctx));
  uint8_t* uds_buf = ctx->payload.uds;
  free_(uds_buf);
  free(ctx);
  return;
}

void mk_l0_context(context_obj ctx, _consumes _allocated_array dice_digest cdi)
  _requires(ctx->tag == 0)
  _ensures((bool) _inline_pulse(engine_state $(*ctx) == PL_L0 (old (value_of $(cdi)))))
{
  _assert(cdi._length == DICE_DIGEST_LEN);
  _ghost_stmt(elim_context_full_pred_uds $(*ctx));
  uint8_t* uds_buf = ctx->payload.uds;
  free_(uds_buf);
  ctx->tag = 1;
  ctx->payload.cdi = cdi;
  _ghost_stmt(intro_context_full_pred_cdi $(*ctx));
}

bool derive_child_from_context(context_obj ctx, const engine_record_t *record)
  _requires(ctx->tag == 0)
  _ensures(return ==> (bool) _inline_pulse(PL_L0? (engine_state $(*ctx))))
  _ensures(!return ==> (bool) _inline_pulse(engine_state $(*ctx) == old (engine_state $(*ctx))))
{
  _ghost_stmt(elim_context_full_pred_uds $(*ctx));
  uint8_t *cdi_buf = (uint8_t*)calloc(DICE_DIGEST_LEN, sizeof(uint8_t));
  // _assert(cdi_buf._length == DICE_DIGEST_LEN);
  bool ok = false; // engine_main(cdi_buf, ctx->payload.uds, record);
  if (ok) {
    _ghost_stmt(intro_context_full_pred_uds $(*ctx));
    mk_l0_context(ctx, cdi_buf);
    return true;
  } else {
    _ghost_stmt(intro_context_full_pred_uds $(*ctx));
    free_(cdi_buf);
    return false;
  }
}