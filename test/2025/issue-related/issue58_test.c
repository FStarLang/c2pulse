#include "../c2pulse.h"
#include <stddef.h>
#include <stdint.h>

#define DICE_DIGEST_LEN 64

typedef struct _engine_record_t {
    size_t  l0_image_header_size;
    uint8_t *l0_image_header;
    uint8_t *l0_image_header_sig;
    size_t  l0_binary_size;
    uint8_t *l0_binary;
    uint8_t *l0_binary_hash;
    uint8_t *l0_image_auth_pubkey;        
} engine_record_t;
