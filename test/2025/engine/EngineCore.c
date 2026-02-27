#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"
#include "DPE.h"
#include "EngineCore.h"
#include "HACL.h"

REQUIRES("a1 |-> Frac 'p1 's1")
REQUIRES("a2 |-> Frac 'p2 's2")
RETURNS(b:bool)
ENSURES("a1 |-> Frac 'p1 's1")
ENSURES("a2 |-> Frac 'p2 's2")
ENSURES("pure (b <==> Seq.equal 's1 's2)")
bool compare(size_t len, ISARRAY(len)uint8_t *a1, ISARRAY(len)uint8_t *a2)
{
    LEMMA(admit());
    return false;
}


ERASED_ARG(#repr:erased _)
ERASED_ARG(#p:_)
PRESERVES(is_engine_record record p repr)
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

ERASED_ARG(#uds_perm:_)
ERASED_ARG(#p:_)
ERASED_ARG(#repr:erased _)
PRESERVES(is_engine_record record p repr)
ERASED_ARG(#uds_bytes:erased _)
PRESERVES(uds |-> Frac uds_perm uds_bytes)
REQUIRES(exists* c0. cdi |-> c0)
ENSURES(exists* c1. cdi |-> c1)
void compute_cdi(ISARRAY(DICE_DIGEST_LEN)uint8_t *cdi, ISARRAY(UDS_LEN)uint8_t *uds, engine_record_t *record){
    uint8_t uds_digest[DICE_DIGEST_LEN];
    uint8_t l0_digest[DICE_DIGEST_LEN];
    hacl_hash(DICE_HASH_ALG, UDS_LEN, uds, uds_digest);
    hacl_hash(DICE_HASH_ALG, record->l0_binary_size, record->l0_binary, l0_digest);
    hacl_hmac(DICE_HASH_ALG, cdi, uds_digest, DICE_DIGEST_LEN, l0_digest, DICE_DIGEST_LEN);
}

bool engine_main(ISARRAY(DICE_DIGEST_LEN) uint8_t *cdi, ISARRAY(UDS_LEN) uint8_t *uds, engine_record_t *record){

    bool b = authenticate_l0_image(record);
    if (b){
        compute_cdi(cdi, uds, record);
        return true;
    }
    else{
        return false;
    }
}