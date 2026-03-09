#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "c2pulse.h"
#include "DPE.h"
#include "EngineCore.h"
#include "HACL.h"

bool compare(size_t len, const _array uint8_t *a1, const _array uint8_t *a2)
    _requires(a1._length == len)
    _requires(a2._length == len)
    _ensures((_slprop) _inline_pulse(pure ($(return) <==> Seq.equal (value_of $(a1)) (value_of $(a2)))))
{
    _ghost_stmt(admit());
    return false;
}

bool authenticate_l0_image(const engine_record_t *record)
{
    bool valid_header_sig = ed25519_verify(record->l0_image_auth_pubkey, record->l0_image_header, record->l0_image_header_size, record->l0_image_header_sig);
    if (valid_header_sig)
    {   
        // uint8_t scratch[DICE_DIGEST_LEN];
        uint8_t *scratch = (uint8_t *) calloc(DICE_DIGEST_LEN, sizeof(uint8_t));
        //allocate a scratch of size DICE_HASH_ALG here and use it
        hacl_hash(DICE_HASH_ALG, record->l0_binary_size, record->l0_binary, scratch);
        bool res = compare(DICE_DIGEST_LEN, scratch, record->l0_binary_hash);
        free(scratch);
        return res;
    }
    else
    {
        return false;
    }
}

void compute_cdi(dice_digest cdi, const uds_array uds, const engine_record_t *record) {
    // uint8_t uds_digest[DICE_DIGEST_LEN];
    // uint8_t l0_digest[DICE_DIGEST_LEN];
    uint8_t *uds_digest = (uint8_t *) calloc(DICE_DIGEST_LEN, sizeof(uint8_t));
    uint8_t *l0_digest = (uint8_t *) calloc(DICE_DIGEST_LEN, sizeof(uint8_t));
    hacl_hash(DICE_HASH_ALG, UDS_LEN, uds, uds_digest);
    hacl_hash(DICE_HASH_ALG, record->l0_binary_size, record->l0_binary, l0_digest);
    hacl_hmac(DICE_HASH_ALG, cdi, uds_digest, DICE_DIGEST_LEN, l0_digest, DICE_DIGEST_LEN);
    free(uds_digest);
    free(l0_digest);
}

bool engine_main(dice_digest cdi, const uds_array uds, const engine_record_t *record) {
    if (authenticate_l0_image(record)) {
        compute_cdi(cdi, uds, record);
        return true;
    } else {
        return false;
    }
}