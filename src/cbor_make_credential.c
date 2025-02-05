

#include "cbor_make_credential.h"
#include "ctap2_cbor.h"
#include "hid/ctap_hid.h"
#include "fido.h"
#include "ctap.h"
#include "files.h"
#include "apdu.h"
#include "credential.h"
#include "mbedtls/sha256.h"
#include "random.h"
#include "pico_keys.h"

int cbor_make_credential(const uint8_t *data, size_t len) {
    CborParser parser;
    CborValue map;
    CborError error = CborNoError;
    CborByteString clientDataHash = { 0 }, pinUvAuthParam = { 0 };
    PublicKeyCredentialRpEntity rp = { 0 };
    PublicKeyCredentialUserEntity user = { 0 };
    PublicKeyCredentialParameters pubKeyCredParams[MAX_CREDENTIAL_COUNT_IN_LIST] = { 0 };
    size_t pubKeyCredParams_len = 0;
    PublicKeyCredentialDescriptor excludeList[MAX_CREDENTIAL_COUNT_IN_LIST] = { 0 };
    size_t excludeList_len = 0;
    CredOptions options = { 0 };
    uint64_t pinUvAuthProtocol = 0, enterpriseAttestation = 0;
    uint8_t *aut_data = NULL;
    size_t resp_size = 0;
    CredExtensions extensions = { 0 };
    //options.present = true;
    //options.up = ptrue;
    options.uv = pfalse;
    //options.rk = pfalse;

    CBOR_CHECK(cbor_parser_init(data, len, 0, &parser, &map));
    uint64_t val_c = 1;
    CBOR_PARSE_MAP_START(map, 1)
    {
        uint64_t val_u = 0;
        CBOR_FIELD_GET_UINT(val_u, 1);
        if (val_c <= 4 && val_c != val_u) {
            CBOR_ERROR(CTAP2_ERR_MISSING_PARAMETER);
        }
        if (val_u < val_c) {
            CBOR_ERROR(CTAP2_ERR_INVALID_CBOR);
        }
        val_c = val_u + 1;
        if (val_u == 0x01) { // clientDataHash
            CBOR_FIELD_GET_BYTES(clientDataHash, 1);
        }
        else if (val_u == 0x02) { // rp
            CBOR_PARSE_MAP_START(_f1, 2)
            {
                CBOR_FIELD_GET_KEY_TEXT(2);
                CBOR_FIELD_KEY_TEXT_VAL_TEXT(2, "id", rp.id);
                CBOR_FIELD_KEY_TEXT_VAL_TEXT(2, "name", rp.parent.name);
            }
            CBOR_PARSE_MAP_END(_f1, 2);
        }
        else if (val_u == 0x03) { // user
            CBOR_PARSE_MAP_START(_f1, 2)
            {
                CBOR_FIELD_GET_KEY_TEXT(2);
                CBOR_FIELD_KEY_TEXT_VAL_BYTES(2, "id", user.id);
                CBOR_FIELD_KEY_TEXT_VAL_TEXT(2, "name", user.parent.name);
                CBOR_FIELD_KEY_TEXT_VAL_TEXT(2, "displayName", user.displayName);
                CBOR_ADVANCE(2);
            }
            CBOR_PARSE_MAP_END(_f1, 2);
        }
        else if (val_u == 0x04) { // pubKeyCredParams
            CBOR_PARSE_ARRAY_START(_f1, 2)
            {
                PublicKeyCredentialParameters *pk = &pubKeyCredParams[pubKeyCredParams_len];
                CBOR_PARSE_MAP_START(_f2, 3)
                {
                    CBOR_FIELD_GET_KEY_TEXT(3);
                    CBOR_FIELD_KEY_TEXT_VAL_TEXT(3, "type", pk->type);
                    CBOR_FIELD_KEY_TEXT_VAL_INT(3, "alg", pk->alg);
                }
                CBOR_PARSE_MAP_END(_f2, 3);
                pubKeyCredParams_len++;
            }
            CBOR_PARSE_ARRAY_END(_f1, 2);
        }
        else if (val_u == 0x05) { // excludeList
            CBOR_PARSE_ARRAY_START(_f1, 2)
            {
                PublicKeyCredentialDescriptor *pc = &excludeList[excludeList_len];
                CBOR_PARSE_MAP_START(_f2, 3)
                {
                    CBOR_FIELD_GET_KEY_TEXT(3);
                    CBOR_FIELD_KEY_TEXT_VAL_BYTES(3, "id", pc->id);
                    CBOR_FIELD_KEY_TEXT_VAL_TEXT(3, "type", pc->type);
                    if (strcmp(_fd3, "transports") == 0) {
                        CBOR_PARSE_ARRAY_START(_f3, 4)
                        {
                            CBOR_FIELD_GET_TEXT(pc->transports[pc->transports_len], 4);
                            pc->transports_len++;
                        }
                        CBOR_PARSE_ARRAY_END(_f3, 4);
                    }
                }
                CBOR_PARSE_MAP_END(_f2, 3);
                excludeList_len++;
            }
            CBOR_PARSE_ARRAY_END(_f1, 2);
        }
        else if (val_u == 0x06) { // extensions
            extensions.present = true;
            CBOR_PARSE_MAP_START(_f1, 2)
            {
                CBOR_FIELD_GET_KEY_TEXT(2);
                CBOR_FIELD_KEY_TEXT_VAL_BOOL(2, "hmac-secret", extensions.hmac_secret);
                CBOR_FIELD_KEY_TEXT_VAL_UINT(2, "credProtect", extensions.credProtect);
                CBOR_FIELD_KEY_TEXT_VAL_BOOL(2, "minPinLength", extensions.minPinLength);
                CBOR_FIELD_KEY_TEXT_VAL_BYTES(2, "credBlob", extensions.credBlob);
                CBOR_FIELD_KEY_TEXT_VAL_BOOL(2, "largeBlobKey", extensions.largeBlobKey);
                CBOR_FIELD_KEY_TEXT_VAL_BOOL(2, "thirdPartyPayment", extensions.thirdPartyPayment);
                CBOR_ADVANCE(2);
            }
            CBOR_PARSE_MAP_END(_f1, 2);
        }
        else if (val_u == 0x07) { // options
            options.present = true;
            CBOR_PARSE_MAP_START(_f1, 2)
            {
                CBOR_FIELD_GET_KEY_TEXT(2);
                CBOR_FIELD_KEY_TEXT_VAL_BOOL(2, "rk", options.rk);
                CBOR_FIELD_KEY_TEXT_VAL_BOOL(2, "up", options.up);
                CBOR_FIELD_KEY_TEXT_VAL_BOOL(2, "uv", options.uv);
                CBOR_ADVANCE(2);
            }
            CBOR_PARSE_MAP_END(_f1, 2);
        }
        else if (val_u == 0x08) { // pinUvAuthParam
            CBOR_FIELD_GET_BYTES(pinUvAuthParam, 1);
        }
        else if (val_u == 0x09) { // pinUvAuthProtocol
            CBOR_FIELD_GET_UINT(pinUvAuthProtocol, 1);
        }
        else if (val_u == 0x0A) { // enterpriseAttestation
            CBOR_FIELD_GET_UINT(enterpriseAttestation, 1);
        }
    }
    CBOR_PARSE_MAP_END(map, 1);

    uint8_t flags = FIDO2_AUT_FLAG_AT;
    uint8_t rp_id_hash[32] = {0};
    mbedtls_sha256((uint8_t *) rp.id.data, rp.id.len, rp_id_hash, 0);

    if (pinUvAuthParam.present == true) {
        if (pinUvAuthParam.len == 0 || pinUvAuthParam.data == NULL) {
            if (check_user_presence() == false) {
                CBOR_ERROR(CTAP2_ERR_OPERATION_DENIED);
            }
            if (!file_has_data(ef_pin)) {
                CBOR_ERROR(CTAP2_ERR_PIN_NOT_SET);
            }
            else {
                CBOR_ERROR(CTAP2_ERR_PIN_AUTH_INVALID);
            }
        }
        else {
            if (pinUvAuthProtocol == 0) {
                CBOR_ERROR(CTAP2_ERR_MISSING_PARAMETER);
            }
            if (pinUvAuthProtocol != 1 && pinUvAuthProtocol != 2) {
                CBOR_ERROR(CTAP1_ERR_INVALID_PARAMETER);
            }
        }
    }

    int curve = -1, alg = 0;
    if (pubKeyCredParams_len == 0) {
        CBOR_ERROR(CTAP2_ERR_MISSING_PARAMETER);
    }

    for (unsigned int i = 0; i < pubKeyCredParams_len; i++) {
        if (pubKeyCredParams[i].type.present == false) {
            CBOR_ERROR(CTAP2_ERR_INVALID_CBOR);
        }
        if (pubKeyCredParams[i].alg == 0) {
            CBOR_ERROR(CTAP2_ERR_INVALID_CBOR);
        }
        if (strcmp(pubKeyCredParams[i].type.data, "public-key") != 0) {
            CBOR_ERROR(CTAP2_ERR_CBOR_UNEXPECTED_TYPE);
        }
        if (pubKeyCredParams[i].alg == FIDO2_ALG_ES256) {
            if (curve <= 0) {
                curve = FIDO2_CURVE_P256;
            }
        }
        else if (pubKeyCredParams[i].alg == FIDO2_ALG_ES384) {
            if (curve <= 0) {
                curve = FIDO2_CURVE_P384;
            }
        }
        else if (pubKeyCredParams[i].alg == FIDO2_ALG_ES512) {
            if (curve <= 0) {
                curve = FIDO2_CURVE_P521;
            }
        }
        else if (pubKeyCredParams[i].alg == FIDO2_ALG_ES256K) {
            if (curve <= 0) {
                curve = FIDO2_CURVE_P256K1;
            }
        }
        else if (pubKeyCredParams[i].alg <= FIDO2_ALG_RS256 && pubKeyCredParams[i].alg >= FIDO2_ALG_RS512) {
            // pass
        }
        //else {
        //    CBOR_ERROR(CTAP2_ERR_CBOR_UNEXPECTED_TYPE);
        //}
        if (curve > 0 && alg == 0) {
            alg = (int)pubKeyCredParams[i].alg;
        }
    }
    if (curve <= 0) {
        CBOR_ERROR(CTAP2_ERR_UNSUPPORTED_ALGORITHM);
    }

    if (options.present) {
        if (options.uv == ptrue) { //5.3
            CBOR_ERROR(CTAP2_ERR_INVALID_OPTION);
        }
        if (options.up != NULL) { //5.6
            CBOR_ERROR(CTAP2_ERR_INVALID_OPTION);
        }
        //else if (options.up == NULL) //5.7
        //rup = ptrue;
    }
    if (pinUvAuthParam.present == false && options.uv == pfalse && file_has_data(ef_pin)) { //8.1
        CBOR_ERROR(CTAP2_ERR_PUAT_REQUIRED);
    }
    if (enterpriseAttestation > 0) {
        if (!(get_opts() & FIDO2_OPT_EA)) {
            CBOR_ERROR(CTAP1_ERR_INVALID_PARAMETER);
        }
        if (enterpriseAttestation != 1 && enterpriseAttestation != 2) { //9.2.1
            CBOR_ERROR(CTAP2_ERR_INVALID_OPTION);
        }
        //Unfinished. See 6.1.2.9
    }
    if (pinUvAuthParam.present == true) { //11.1
        int ret = verify((uint8_t)pinUvAuthProtocol, paut.data, clientDataHash.data, (uint16_t)clientDataHash.len, pinUvAuthParam.data);
        if (ret != CborNoError) {
            CBOR_ERROR(CTAP2_ERR_PIN_AUTH_INVALID);
        }
        if (!(paut.permissions & CTAP_PERMISSION_MC)) {
            CBOR_ERROR(CTAP2_ERR_PIN_AUTH_INVALID);
        }
        if (paut.has_rp_id == true && memcmp(paut.rp_id_hash, rp_id_hash, 32) != 0) {
            CBOR_ERROR(CTAP2_ERR_PIN_AUTH_INVALID);
        }
        if (getUserVerifiedFlagValue() == false) {
            CBOR_ERROR(CTAP2_ERR_PIN_AUTH_INVALID);
        }
        flags |= FIDO2_AUT_FLAG_UV;
        if (paut.has_rp_id == false) {
            memcpy(paut.rp_id_hash, rp_id_hash, 32);
            paut.has_rp_id = true;
        }
    }

    for (size_t e = 0; e < excludeList_len; e++) { //12.1
        if (excludeList[e].type.present == false || excludeList[e].id.present == false) {
            CBOR_ERROR(CTAP2_ERR_MISSING_PARAMETER);
        }
        if (strcmp(excludeList[e].type.data, (char *)"public-key") != 0) {
            continue;
        }
        Credential ecred = {0};
        if (credential_load(excludeList[e].id.data, excludeList[e].id.len, rp_id_hash,
                            &ecred) == 0 &&
            (ecred.extensions.credProtect != CRED_PROT_UV_REQUIRED ||
             (flags & FIDO2_AUT_FLAG_UV))) {
                credential_free(&ecred);
            CBOR_ERROR(CTAP2_ERR_CREDENTIAL_EXCLUDED);
        }
        credential_free(&ecred);
    }

    if (extensions.largeBlobKey == pfalse ||
        (extensions.largeBlobKey == ptrue && options.rk != ptrue)) {
        CBOR_ERROR(CTAP2_ERR_INVALID_OPTION);
    }

    if (options.up == ptrue || options.up == NULL) { //14.1
        if (pinUvAuthParam.present == true) {
            if (getUserPresentFlagValue() == false) {
                if (check_user_presence() == false) {
                    CBOR_ERROR(CTAP2_ERR_OPERATION_DENIED);
                }
            }
        }
        flags |= FIDO2_AUT_FLAG_UP;
        if (options.up == ptrue) {
            clearUserPresentFlag();
            clearUserVerifiedFlag();
            clearPinUvAuthTokenPermissionsExceptLbw();
        }
    }

    const known_app_t *ka = find_app_by_rp_id_hash(rp_id_hash);

    uint8_t cred_id[MAX_CRED_ID_LENGTH] = {0};
    size_t cred_id_len = 0;

    CBOR_CHECK(credential_create(&rp.id, &user.id, &user.parent.name, &user.displayName, &options,
                                 &extensions, (!ka || ka->use_sign_count == ptrue), alg, curve,
                                 cred_id, &cred_id_len));

    if (getUserVerifiedFlagValue()) {
        flags |= FIDO2_AUT_FLAG_UV;
    }
    size_t ext_len = 0;
    uint8_t ext[512] = {0};
    CborEncoder encoder, mapEncoder, mapEncoder2;
    if (extensions.present == true) {
        cbor_encoder_init(&encoder, ext, sizeof(ext), 0);
        int l = 0;
        uint8_t minPinLen = 0;
        if (extensions.hmac_secret != NULL) {
            l++;
        }
        if (extensions.credProtect != 0) {
            l++;
        }
        if (extensions.minPinLength != NULL) {
            file_t *ef_minpin = search_by_fid(EF_MINPINLEN, NULL, SPECIFY_EF);
            if (file_has_data(ef_minpin)) {
                uint8_t *minpin_data = file_get_data(ef_minpin);
                for (int o = 2; o < file_get_size(ef_minpin); o += 32) {
                    if (memcmp(minpin_data + o, rp_id_hash, 32) == 0) {
                        minPinLen = minpin_data[0];
                        if (minPinLen > 0) {
                            l++;
                        }
                        break;
                    }
                }
            }
        }
        if (extensions.credBlob.present == true) {
            l++;
        }
        CBOR_CHECK(cbor_encoder_create_map(&encoder, &mapEncoder, l));
        if (extensions.credBlob.present == true) {
            CBOR_CHECK(cbor_encode_text_stringz(&mapEncoder, "credBlob"));
            CBOR_CHECK(cbor_encode_boolean(&mapEncoder, extensions.credBlob.len < MAX_CREDBLOB_LENGTH));
        }
        if (extensions.credProtect != 0) {
            CBOR_CHECK(cbor_encode_text_stringz(&mapEncoder, "credProtect"));
            CBOR_CHECK(cbor_encode_uint(&mapEncoder, extensions.credProtect));
        }
        if (extensions.hmac_secret != NULL) {

            CBOR_CHECK(cbor_encode_text_stringz(&mapEncoder, "hmac-secret"));
            CBOR_CHECK(cbor_encode_boolean(&mapEncoder, *extensions.hmac_secret));
        }
        if (minPinLen > 0) {

            CBOR_CHECK(cbor_encode_text_stringz(&mapEncoder, "minPinLength"));
            CBOR_CHECK(cbor_encode_uint(&mapEncoder, minPinLen));
        }

        CBOR_CHECK(cbor_encoder_close_container(&encoder, &mapEncoder));
        ext_len = cbor_encoder_get_buffer_size(&encoder, ext);
        flags |= FIDO2_AUT_FLAG_ED;
    }
    mbedtls_ecdsa_context ekey;
    mbedtls_ecdsa_init(&ekey);
    int ret = fido_load_key(curve, cred_id, &ekey);
    if (ret != 0) {
        mbedtls_ecdsa_free(&ekey);
        CBOR_ERROR(CTAP1_ERR_OTHER);
    }
    const mbedtls_ecp_curve_info *cinfo = mbedtls_ecp_curve_info_from_grp_id(ekey.grp.id);
    if (cinfo == NULL) {
        mbedtls_ecdsa_free(&ekey);
        CBOR_ERROR(CTAP1_ERR_OTHER);
    }
    size_t olen = 0;
    uint32_t ctr = get_sign_counter();
    uint8_t cbor_buf[1024] = {0};
    cbor_encoder_init(&encoder, cbor_buf, sizeof(cbor_buf), 0);
    CBOR_CHECK(COSE_key(&ekey, &encoder, &mapEncoder));
    size_t rs = cbor_encoder_get_buffer_size(&encoder, cbor_buf);

    size_t aut_data_len = 32 + 1 + 4 + (16 + 2 + cred_id_len + rs) + ext_len;
    aut_data = (uint8_t *) calloc(1, aut_data_len + clientDataHash.len);
    uint8_t *pa = aut_data;
    memcpy(pa, rp_id_hash, 32); pa += 32;
    *pa++ = flags;
    pa += put_uint32_t_be(ctr, pa);
    memcpy(pa, aaguid, 16); pa += 16;
    pa += put_uint16_t_be(cred_id_len, pa);
    memcpy(pa, cred_id, cred_id_len); pa += (uint16_t)cred_id_len;
    memcpy(pa, cbor_buf, rs); pa += (uint16_t)rs;
    memcpy(pa, ext, ext_len); pa += (uint16_t)ext_len;
    if ((size_t)(pa - aut_data) != aut_data_len) {
        mbedtls_ecdsa_free(&ekey);
        CBOR_ERROR(CTAP1_ERR_OTHER);
    }

    memcpy(pa, clientDataHash.data, clientDataHash.len);
    uint8_t hash[64] = {0}, sig[MBEDTLS_ECDSA_MAX_LEN] = {0};
    const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (ekey.grp.id == MBEDTLS_ECP_DP_SECP384R1) {
        md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA384);
    }
    else if (ekey.grp.id == MBEDTLS_ECP_DP_SECP521R1) {
        md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
    }
    ret = mbedtls_md(md, aut_data, aut_data_len + clientDataHash.len, hash);

    bool self_attestation = true;
    if (enterpriseAttestation == 2 || (ka && ka->use_self_attestation == pfalse)) {
        mbedtls_ecdsa_free(&ekey);
        mbedtls_ecdsa_init(&ekey);
        uint8_t key[32] = {0};
        if (load_keydev(key) != 0) {
            CBOR_ERROR(CTAP1_ERR_OTHER);
        }
        ret = mbedtls_ecp_read_key(MBEDTLS_ECP_DP_SECP256R1, &ekey, key, 32);
        mbedtls_platform_zeroize(key, sizeof(key));
        md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        self_attestation = false;
    }
    ret = mbedtls_ecdsa_write_signature(&ekey, mbedtls_md_get_type(md), hash, mbedtls_md_get_size(md), sig, sizeof(sig), &olen, random_gen, NULL);
    mbedtls_ecdsa_free(&ekey);

    if (user.id.len > 0 && user.parent.name.len > 0 && user.displayName.len > 0) {
       if (memcmp(user.parent.name.data, "+pico", 5) == 0) {
            options.rk = pfalse;
#ifndef ENABLE_EMULATION
            uint8_t *p = (uint8_t *)user.parent.name.data + 5;
            if (memcmp(p, "CommissionProfile", 17) == 0) {
                ret = phy_unserialize_data(user.id.data, user.id.len, &phy_data);
                if (ret == PICOKEY_OK) {
                    file_put_data(ef_phy, user.id.data, user.id.len);
                }
            }
#endif
            if (ret != 0) {
                CBOR_ERROR(CTAP2_ERR_PROCESSING);
            }
            low_flash_available();
       }
    }

    uint8_t largeBlobKey[32] = {0};
    if (extensions.largeBlobKey == ptrue && options.rk == ptrue) {
        ret = credential_derive_large_blob_key(cred_id, cred_id_len, largeBlobKey);
        if (ret != 0) {
            CBOR_ERROR(CTAP2_ERR_PROCESSING);
        }
    }

    cbor_encoder_init(&encoder, ctap_resp->init.data + 1, CTAP_MAX_CBOR_PAYLOAD, 0);
    CBOR_CHECK(cbor_encoder_create_map(&encoder, &mapEncoder, extensions.largeBlobKey == ptrue && options.rk == ptrue ? 5 : 4));

    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x01));
    CBOR_CHECK(cbor_encode_text_stringz(&mapEncoder, "packed"));
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x02));
    CBOR_CHECK(cbor_encode_byte_string(&mapEncoder, aut_data, aut_data_len));
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x03));

    CBOR_CHECK(cbor_encoder_create_map(&mapEncoder, &mapEncoder2, self_attestation == false || is_nitrokey ? 3 : 2));
    CBOR_CHECK(cbor_encode_text_stringz(&mapEncoder2, "alg"));
    CBOR_CHECK(cbor_encode_negative_int(&mapEncoder2, self_attestation || is_nitrokey ? -alg : -FIDO2_ALG_ES256));
    CBOR_CHECK(cbor_encode_text_stringz(&mapEncoder2, "sig"));
    CBOR_CHECK(cbor_encode_byte_string(&mapEncoder2, sig, olen));
    if (self_attestation == false || is_nitrokey) {
        CborEncoder arrEncoder;
        file_t *ef_cert = NULL;
        if (enterpriseAttestation == 2) {
            ef_cert = search_by_fid(EF_EE_DEV_EA, NULL, SPECIFY_EF);
        }
        if (!file_has_data(ef_cert)) {
            ef_cert = ef_certdev;
        }
        CBOR_CHECK(cbor_encode_text_stringz(&mapEncoder2, "x5c"));
        CBOR_CHECK(cbor_encoder_create_array(&mapEncoder2, &arrEncoder, 1));
        CBOR_CHECK(cbor_encode_byte_string(&arrEncoder, file_get_data(ef_cert), file_get_size(ef_cert)));
        CBOR_CHECK(cbor_encoder_close_container(&mapEncoder2, &arrEncoder));
    }
    CBOR_CHECK(cbor_encoder_close_container(&mapEncoder, &mapEncoder2));

    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x04));
    CBOR_CHECK(cbor_encode_boolean(&mapEncoder, enterpriseAttestation == 2));

    if (extensions.largeBlobKey == ptrue && options.rk == ptrue) {
        CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x05));
        CBOR_CHECK(cbor_encode_byte_string(&mapEncoder, largeBlobKey, sizeof(largeBlobKey)));
    }
    mbedtls_platform_zeroize(largeBlobKey, sizeof(largeBlobKey));
    CBOR_CHECK(cbor_encoder_close_container(&encoder, &mapEncoder));
    resp_size = cbor_encoder_get_buffer_size(&encoder, ctap_resp->init.data + 1);

    if (options.rk == ptrue) {
        if (credential_store(cred_id, cred_id_len, rp_id_hash) != 0) {
            CBOR_ERROR(CTAP2_ERR_KEY_STORE_FULL);
        }
    }
    ctr++;
    file_put_data(ef_counter, (uint8_t *) &ctr, sizeof(ctr));
    low_flash_available();
err:
    CBOR_FREE_BYTE_STRING(clientDataHash);
    CBOR_FREE_BYTE_STRING(pinUvAuthParam);
    CBOR_FREE_BYTE_STRING(rp.id);
    CBOR_FREE_BYTE_STRING(rp.parent.name);
    CBOR_FREE_BYTE_STRING(user.id);
    CBOR_FREE_BYTE_STRING(user.displayName);
    CBOR_FREE_BYTE_STRING(user.parent.name);
    if (extensions.present == true) {
        CBOR_FREE_BYTE_STRING(extensions.credBlob);
    }
    for (size_t n = 0; n < MAX_CREDENTIAL_COUNT_IN_LIST; n++) {
        CBOR_FREE_BYTE_STRING(pubKeyCredParams[n].type);
    }

    for (size_t m = 0; m < MAX_CREDENTIAL_COUNT_IN_LIST; m++) {
        CBOR_FREE_BYTE_STRING(excludeList[m].type);
        CBOR_FREE_BYTE_STRING(excludeList[m].id);
        for (size_t n = 0; n < excludeList[m].transports_len; n++) {
            CBOR_FREE_BYTE_STRING(excludeList[m].transports[n]);
        }
    }
    if (aut_data) {
        free(aut_data);
    }
    if (error != CborNoError) {
        if (error == CborErrorImproperValue) {
            return CTAP2_ERR_CBOR_UNEXPECTED_TYPE;
        }
        return error;
    }
    res_APDU_size = (uint16_t)resp_size;
    return 0;
}
