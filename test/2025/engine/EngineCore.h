#ifndef _ENGINE_CORE_H_
#define _ENGINE_CORE_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../c2pulse.h"
#include "DPE.h"

_include_pulse(
    open FStar.Ghost
    module SizeT = Pulse.Lib.C.SizeT
    let bool_to_bool b = b

    let my_length = 17
)

_include_pulse(
    module U8 = FStar.UInt8
    module US = FStar.SizeT

    noeq
    type engine_record_repr = {
        l0_image_header      : Seq.seq U8.t;
        l0_image_header_sig  : Seq.seq U8.t;
        l0_binary            : Seq.seq U8.t;
        l0_binary_hash       : Seq.seq U8.t;
        l0_image_auth_pubkey : Seq.seq U8.t;
    }

    [@@pulse_unfold]
    let engine_record_perm (record:engine_record_t_spec) (p:perm) (repr:engine_record_repr)
    : slprop
    =  (record.l0_image_header |-> Frac p repr.l0_image_header) **
       (record.l0_image_header_sig |-> Frac p repr.l0_image_header_sig) **
       (record.l0_binary |-> Frac p repr.l0_binary) **
       (record.l0_binary_hash |-> Frac p repr.l0_binary_hash) **
       (record.l0_image_auth_pubkey |-> Frac p repr.l0_image_auth_pubkey)

    [@@pulse_unfold]
    let is_engine_record (r:ref engine_record_t) (p:perm) (repr:engine_record_repr) =
        exists* s. engine_record_t_pred r s ** engine_record_perm s p repr

)

_preserves((_slprop) _inline_pulse(is_engine_record record p repr))
_preserves((_slprop) _inline_pulse(uds |-> Frac uds_perm uds_bytes))
_requires((_slprop) _inline_pulse(exists* c0. cdi |-> c0))
_ensures((_slprop) _inline_pulse(exists* c1. cdi |-> c1))
bool engine_main(uint8_t *cdi, uint8_t *uds, engine_record_t *record);

#endif