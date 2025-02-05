

#ifndef _CBOR_MAKE_CREDENTIAL_H_
#define _CBOR_MAKE_CREDENTIAL_H_

#include "ctap2_cbor.h"

typedef struct PublicKeyCredentialEntity {
    CborCharString name;
} PublicKeyCredentialEntity;

typedef struct PublicKeyCredentialRpEntity {
    PublicKeyCredentialEntity parent;
    CborCharString id;
} PublicKeyCredentialRpEntity;

typedef struct PublicKeyCredentialUserEntity {
    PublicKeyCredentialEntity parent;
    CborByteString id;
    CborCharString displayName;
} PublicKeyCredentialUserEntity;

typedef struct PublicKeyCredentialParameters {
    CborCharString type;
    int64_t alg;
} PublicKeyCredentialParameters;

typedef struct PublicKeyCredentialDescriptor {
    CborCharString type;
    CborByteString id;
    CborCharString transports[8];
    size_t transports_len;
} PublicKeyCredentialDescriptor;


#endif //_CBOR_MAKE_CREDENTIAL_H_
