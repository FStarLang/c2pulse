#include "../include/PulseMacros.h"
#include <stddef.h>
#include <stdint.h>

#define DICE_DIGEST_LEN 64

typedef struct _engine_record_t {
    size_t  l0_image_header_size;
    ISARRAY(l0_image_header_size) uint8_t *l0_image_header;
    ISARRAY(64) uint8_t *l0_image_header_sig;
    size_t  l0_binary_size;
    ISARRAY(l0_binary_size) uint8_t *l0_binary;
    ISARRAY(DICE_DIGEST_LEN) uint8_t *l0_binary_hash;
    ISARRAY(32) uint8_t *l0_image_auth_pubkey;        
} engine_record_t;
