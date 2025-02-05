
#include "ctap2_cbor.h"
#include "fido.h"
#include "ctap.h"
#include "hid/ctap_hid.h"
#include "files.h"
#include "apdu.h"
#include "pico_keys.h"
#include "mbedtls/sha256.h"

static uint64_t expectedLength = 0, expectedNextOffset = 0;
uint8_t temp_lba[MAX_LARGE_BLOB_SIZE];

int cbor_large_blobs(const uint8_t *data, size_t len) {
    CborParser parser;
    CborValue map;
    CborEncoder encoder, mapEncoder;
    CborError error = CborNoError;
    uint64_t get = 0, offset = UINT64_MAX, length = 0, pinUvAuthProtocol = 0;
    CborByteString set = { 0 }, pinUvAuthParam = { 0 };

    CBOR_CHECK(cbor_parser_init(data, len, 0, &parser, &map));
    uint64_t val_c = 1;
    CBOR_PARSE_MAP_START(map, 1)
    {
        uint64_t val_u = 0;
        CBOR_FIELD_GET_UINT(val_u, 1);
        if (val_c <= 0 && val_c != val_u) {
            CBOR_ERROR(CTAP2_ERR_MISSING_PARAMETER);
        }
        if (val_u < val_c) {
            CBOR_ERROR(CTAP2_ERR_INVALID_CBOR);
        }
        val_c = val_u + 1;
        if (val_u == 0x01) {
            CBOR_FIELD_GET_UINT(get, 1);
        }
        else if (val_u == 0x02) {
            CBOR_FIELD_GET_BYTES(set, 1);
        }
        else if (val_u == 0x03) {
            CBOR_FIELD_GET_UINT(offset, 1);
        }
        else if (val_u == 0x04) {
            CBOR_FIELD_GET_UINT(length, 1);
        }
        else if (val_u == 0x05) {
            CBOR_FIELD_GET_BYTES(pinUvAuthParam, 1);
        }
        else if (val_u == 0x06) {
            CBOR_FIELD_GET_UINT(pinUvAuthProtocol, 1);
        }
    }
    CBOR_PARSE_MAP_END(map, 1);

    if (offset == UINT64_MAX) {
        CBOR_ERROR(CTAP1_ERR_INVALID_PARAMETER);
    }
    if (get == 0 && set.present == false) {
        CBOR_ERROR(CTAP1_ERR_INVALID_PARAMETER);
    }
    if (get != 0 && set.present == true) {
        CBOR_ERROR(CTAP1_ERR_INVALID_PARAMETER);
    }

    cbor_encoder_init(&encoder, ctap_resp->init.data + 1, CTAP_MAX_CBOR_PAYLOAD, 0);
    if (get > 0) {
        if (length != 0) {
            CBOR_ERROR(CTAP1_ERR_INVALID_PARAMETER);
        }
        if (length > MAX_FRAGMENT_LENGTH) {
            CBOR_ERROR(CTAP1_ERR_INVALID_LEN);
        }
        if (offset > file_get_size(ef_largeblob)) {
            CBOR_ERROR(CTAP1_ERR_INVALID_PARAMETER);
        }
        CBOR_CHECK(cbor_encoder_create_map(&encoder, &mapEncoder, 1));
        CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x01));
        CBOR_CHECK(cbor_encode_byte_string(&mapEncoder, file_get_data(ef_largeblob) + offset,
                                           MIN(get, file_get_size(ef_largeblob) - offset)));
    }
    else {
        if (set.len > MAX_FRAGMENT_LENGTH) {
            CBOR_ERROR(CTAP1_ERR_INVALID_LEN);
        }
        if (offset == 0) {
            if (length == 0) {
                CBOR_ERROR(CTAP1_ERR_INVALID_PARAMETER);
            }
            if (length > MAX_LARGE_BLOB_SIZE) {
                CBOR_ERROR(CTAP2_ERR_LARGE_BLOB_STORAGE_FULL);
            }
            if (length < 17) {
                CBOR_ERROR(CTAP1_ERR_INVALID_PARAMETER);
            }
            expectedLength = length;
            expectedNextOffset = 0;
        }
        else {
            if (length != 0) {
                CBOR_ERROR(CTAP1_ERR_INVALID_PARAMETER);
            }
        }
        if (offset != expectedNextOffset) {
            CBOR_ERROR(CTAP1_ERR_INVALID_SEQ);
        }
        if (pinUvAuthParam.present == false) {
            CBOR_ERROR(CTAP2_ERR_PUAT_REQUIRED);
        }
        if (pinUvAuthProtocol == 0) {
            CBOR_ERROR(CTAP2_ERR_MISSING_PARAMETER);
        }
        uint8_t verify_data[70] = { 0 };
        memset(verify_data, 0xff, 32);
        verify_data[32] = 0x0C;
        put_uint32_t_le(offset, verify_data + 34);
        mbedtls_sha256(set.data, set.len, verify_data + 38, 0);
        if (verify((uint8_t)pinUvAuthProtocol, paut.data, verify_data, (uint16_t)sizeof(verify_data), pinUvAuthParam.data) != 0) {
            CBOR_ERROR(CTAP2_ERR_PIN_AUTH_INVALID);
        }
        if (!(paut.permissions & CTAP_PERMISSION_LBW)) {
            CBOR_ERROR(CTAP2_ERR_PIN_AUTH_INVALID);
        }
        if (offset + set.len > expectedLength) {
            CBOR_ERROR(CTAP1_ERR_INVALID_PARAMETER);
        }
        if (offset == 0) {
            memset(temp_lba, 0, sizeof(temp_lba));
        }
        memcpy(temp_lba + expectedNextOffset, set.data, set.len);
        expectedNextOffset += set.len;
        if (expectedNextOffset == expectedLength) {
            uint8_t sha[32];
            mbedtls_sha256(temp_lba, expectedLength - 16, sha, 0);
            if (expectedLength > 17 && memcmp(sha, temp_lba + expectedLength - 16, 16) != 0) {
                CBOR_ERROR(CTAP2_ERR_INTEGRITY_FAILURE);
            }
            file_put_data(ef_largeblob, temp_lba, (uint16_t)expectedLength);
            low_flash_available();
        }
        goto err;
    }
    CBOR_CHECK(cbor_encoder_close_container(&encoder, &mapEncoder));

err:
    CBOR_FREE_BYTE_STRING(pinUvAuthParam);
    CBOR_FREE_BYTE_STRING(set);
    if (error != CborNoError) {
        return -CTAP2_ERR_INVALID_CBOR;
    }
    res_APDU_size = (uint16_t)cbor_encoder_get_buffer_size(&encoder, ctap_resp->init.data + 1);
    return 0;
}
