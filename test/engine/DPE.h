#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"

#define UDS_LEN 32
#define DICE_SUCCESS 0
#define DICE_ERROR 1
#define DICE_DIGEST_LEN 64 //TODO generalize
#define SHA2_256 0 //maybe use an enum for this?
#define DICE_HASH_ALG SHA2_256 

typedef struct _engine_record_t {
    size_t  l0_image_header_size;
    ISARRAY(l0_image_header_size) uint8_t *l0_image_header;
    ISARRAY(64) uint8_t *l0_image_header_sig;
    size_t  l0_binary_size;
    ISARRAY(l0_binary_size) uint8_t *l0_binary;
    ISARRAY(DICE_DIGEST_LEN) uint8_t *l0_binary_hash;
    ISARRAY(32) uint8_t *l0_image_auth_pubkey;        
} engine_record_t;

typedef struct {
    uint32_t length;
    //const char* data
    //Vidush: Using uint8_t for now since const char* is not supported.
    uint8_t *data;
} fStar_Bytes_bytes;

typedef struct _character_string_t_s{
    uint32_t fst; 
    fStar_Bytes_bytes snd;
} character_string_t;

typedef struct _octet_string_t_s{
    uint32_t len; 
    fStar_Bytes_bytes s;
} octet_string_t;

typedef struct _deviceIDCSR_ingredients_t_s
{
  int32_t ku;
  int32_t version;
  character_string_t s_common;
  character_string_t s_org;
  character_string_t s_country;
}
deviceIDCSR_ingredients_t;

typedef struct aliasKeyCRT_ingredients_t_s
{
  int32_t version1;
  octet_string_t serialNumber;
  character_string_t i_common;
  character_string_t i_org;
  character_string_t i_country;
  fStar_Bytes_bytes notBefore;
  fStar_Bytes_bytes notAfter;
  character_string_t s_common1;
  character_string_t s_org1;
  character_string_t s_country1;
  int32_t ku1;
  int32_t l0_version;
} aliasKeyCRT_ingredients_t;

typedef struct _l0_record_t_s
{
  uint8_t *fwid;
  uint32_t deviceID_label_len;
  uint8_t *deviceID_label;
  uint32_t aliasKey_label_len;
  uint8_t *aliasKey_label;
  deviceIDCSR_ingredients_t deviceIDCSR_ingredients;
  aliasKeyCRT_ingredients_t aliasKeyCRT_ingredients;
} l0_record_t;

typedef struct _l1_context_t_s
{
  uint8_t *deviceID_pub;
  uint8_t *aliasKey_priv;
  uint8_t *aliasKey_pub;
  uint32_t deviceIDCSR_len;
  uint8_t *deviceIDCSR;
  uint32_t aliasKeyCRT_len;
  uint8_t *aliasKeyCRT;
} l1_context_t;

#define Engine_context 0
#define L0_context 1
#define L1_context 2

typedef uint8_t context_t_tags;

typedef struct _context_t_s
{
  context_t_tags tag;
  union {
    uint8_t *case_Engine_context;
    uint8_t *case_L0_context;
    l1_context_t case_L1_context;
  } u;
} context_t;

#define Engine_record 0
#define L0_record 1

typedef uint8_t record_t_tags;

typedef struct record_t_s
{
  record_t_tags tag;
  union {
    engine_record_t case_Engine_record;
    l0_record_t case_L0_record;
  } u;
} record_t;

typedef uint16_t dpe_sid_t;

typedef context_t dpe_session_state__Available__payload;

#define DPE_SessionStart 0
#define DPE_Available 1
#define DPE_InUse 2
#define DPE_SessionClosed 3
#define DPE_SessionError 4

typedef uint8_t dpe_session_state_tags;

typedef struct dpe_session_state_s
{
  dpe_session_state_tags tag;
  dpe_session_state__Available__payload _0;
} dpe_session_state;

bool dpe_uu___is_SessionStart(dpe_session_state projectee);

bool dpe_uu___is_Available(dpe_session_state projectee);

bool dpe_uu___is_InUse(dpe_session_state projectee);

bool dpe_uu___is_SessionClosed(dpe_session_state projectee);

bool dpe_uu___is_SessionError(dpe_session_state projectee);

#define Pulse_Lib_HashTable_Spec_Clean 0
#define Pulse_Lib_HashTable_Spec_Zombie 1
#define Pulse_Lib_HashTable_Spec_Used 2

typedef uint8_t pulse_Lib_HashTable_Spec_cell__uint16_t_DPE_session_state_tags;

typedef struct pulse_Lib_HashTable_Spec_cell__uint16_t_DPE_session_state_s
{
  pulse_Lib_HashTable_Spec_cell__uint16_t_DPE_session_state_tags tag;
  uint16_t k;
  dpe_session_state v;
}
pulse_Lib_HashTable_Spec_cell__uint16_t_DPE_session_state;

typedef struct dpe_ht_t_s
{
  size_t sz;
  //hashf : UInt16.t -> SizeT.t;
  //size_t (*hashf)(uint16_t x0);
  pulse_Lib_HashTable_Spec_cell__uint16_t_DPE_session_state *contents;
} dpe_ht_t;

typedef struct dpe_st_s
{
  uint16_t st_ctr;
  dpe_ht_t st_tbl;
} dpe_st;

#define FStar_Pervasives_Native_None 0
#define FStar_Pervasives_Native_Some 1

typedef uint8_t fStar_Pervasives_Native_option__DPE_st_tags;

typedef struct fStar_Pervasives_Native_option__uint16_t_s
{
  fStar_Pervasives_Native_option__DPE_st_tags tag;
  uint16_t v;
} fStar_Pervasives_Native_option__uint16_t;

fStar_Pervasives_Native_option__uint16_t dpe_open_session(void);

void dpe_initialize_context(uint16_t sid, uint8_t *uds);

bool dpe_derive_child(uint16_t sid, record_t record);

void dpe_close_session(uint16_t sid);

uint32_t dpe_certify_key(uint16_t sid, uint8_t *pub_key, uint32_t crt_len, uint8_t *crt);

void dpe_sign(uint16_t sid, uint8_t *signature, size_t msg_len, uint8_t *msg);