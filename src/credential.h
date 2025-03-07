
#ifndef _CREDENTIAL_H_
#define _CREDENTIAL_H_

#include "ctap2_cbor.h"

typedef struct CredOptions {
    const bool *rk;
    const bool *up;
    const bool *uv;
    bool present;
} CredOptions;

typedef struct CredExtensions {
    const bool *hmac_secret;
    uint64_t credProtect;
    const bool *minPinLength;
    CborByteString credBlob;
    const bool *largeBlobKey;
    const bool *thirdPartyPayment;
    bool present;
} CredExtensions;

typedef struct Credential {
    CborCharString rpId;
    CborByteString userId;
    CborCharString userName;
    CborCharString userDisplayName;
    uint64_t creation;
    CredExtensions extensions;
    const bool *use_sign_count;
    int64_t alg;
    int64_t curve;
    CborByteString id;
    CredOptions opts;
    bool present;
} Credential;

#define CRED_PROT_UV_OPTIONAL               0x01
#define CRED_PROT_UV_OPTIONAL_WITH_LIST     0x02
#define CRED_PROT_UV_REQUIRED               0x03

#define CRED_PROTO                          "\xf1\xd0\x02\x01"

extern int credential_verify(uint8_t *cred_id, size_t cred_id_len, const uint8_t *rp_id_hash);
extern int credential_create(CborCharString *rpId,
                             CborByteString *userId,
                             CborCharString *userName,
                             CborCharString *userDisplayName,
                             CredOptions *opts,
                             CredExtensions *extensions,
                             bool use_sign_count,
                             int alg,
                             int curve,
                             uint8_t *cred_id,
                             size_t *cred_id_len);
extern void credential_free(Credential *cred);
extern int credential_store(const uint8_t *cred_id, size_t cred_id_len, const uint8_t *rp_id_hash);
extern int credential_load(const uint8_t *cred_id,
                           size_t cred_id_len,
                           const uint8_t *rp_id_hash,
                           Credential *cred);
extern int credential_derive_hmac_key(const uint8_t *cred_id, size_t cred_id_len, uint8_t *outk);
extern int credential_derive_large_blob_key(const uint8_t *cred_id,
                                            size_t cred_id_len,
                                            uint8_t *outk);

#endif // _CREDENTIAL_H_
