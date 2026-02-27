#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../c2pulse.h"
#include "DPE.h"
#include "EngineCore.h"
#include "HACL.h"

_requires((_slprop) _inline_pulse(a1 |-> Frac 'p1 's1))
_requires((_slprop) _inline_pulse(a2 |-> Frac 'p2 's2))
_ensures((_slprop) _inline_pulse(a1 |-> Frac 'p1 's1))
_ensures((_slprop) _inline_pulse(a2 |-> Frac 'p2 's2))
_ensures((_slprop) _inline_pulse(pure (b <==> Seq.equal 's1 's2)))
bool compare(size_t len, uint8_t *a1, uint8_t *a2)
{
    _assert((_slprop) _inline_pulse(admit()));
    return false;
}

_preserves((_slprop) _inline_pulse(is_engine_record record p repr))
bool authenticate_l0_image(engine_record_t *record)
{
    bool valid_header_sig = ed25519_verify(record->l0_image_auth_pubkey, record->l0_image_header, record->l0_image_header_size, record->l0_image_header_sig);
    if (valid_header_sig)
    {   
        uint8_t scratch[DICE_DIGEST_LEN];
        //allocate a scratch of size DICE_HASH_ALG here and use it
        hacl_hash(DICE_HASH_ALG, record->l0_binary_size, record->l0_binary, scratch);
        bool res = compare(DICE_DIGEST_LEN, scratch, record->l0_binary_hash);
        return res;
    }
    else
    {
        return false;
    }
}

_preserves((_slprop) _inline_pulse(is_engine_record record p repr))
_preserves((_slprop) _inline_pulse(uds |-> Frac uds_perm uds_bytes))
_requires((_slprop) _inline_pulse(exists* c0. cdi |-> c0))
_ensures((_slprop) _inline_pulse(exists* c1. cdi |-> c1))
void compute_cdi(uint8_t *cdi, uint8_t *uds, engine_record_t *record){
    uint8_t uds_digest[DICE_DIGEST_LEN];
    uint8_t l0_digest[DICE_DIGEST_LEN];
    hacl_hash(DICE_HASH_ALG, UDS_LEN, uds, uds_digest);
    hacl_hash(DICE_HASH_ALG, record->l0_binary_size, record->l0_binary, l0_digest);
    hacl_hmac(DICE_HASH_ALG, cdi, uds_digest, DICE_DIGEST_LEN, l0_digest, DICE_DIGEST_LEN);
}

bool engine_main(uint8_t *cdi, uint8_t *uds, engine_record_t *record){

    bool b = authenticate_l0_image(record);
    if (b){
        compute_cdi(cdi, uds, record);
        return true;
    }
    else{
        return false;
    }
}