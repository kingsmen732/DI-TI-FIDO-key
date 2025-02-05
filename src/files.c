

#include "files.h"

file_t file_entries[] = {
    { .fid = 0x3f00, .parent = 0xff, .name = NULL, .type = FILE_TYPE_DF, .data = NULL, .ef_structure = 0, .acl = { 0 } }, // MF
    { .fid = EF_KEY_DEV, .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // Device Key
    { .fid = EF_KEY_DEV_ENC, .parent = 0, .name = NULL,.type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // Device Key Enc
    { .fid = EF_MKEK, .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // MKEK
    { .fid = EF_EE_DEV,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // End Entity Certificate Device
    { .fid = EF_EE_DEV_EA,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // End Entity Enterprise Attestation Certificate
    { .fid = EF_COUNTER,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // Global counter
    { .fid = EF_PIN,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // PIN
    { .fid = EF_AUTHTOKEN,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // AUTH TOKEN
    { .fid = EF_MINPINLEN,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // MIN PIN LENGTH
    { .fid = EF_OPTS,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // Global options
    { .fid = EF_LARGEBLOB,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } }, // Large Blob
    { .fid = EF_OTP_PIN,  .parent = 0, .name = NULL, .type = FILE_TYPE_INTERNAL_EF | FILE_DATA_FLASH, .data = NULL, .ef_structure = FILE_EF_TRANSPARENT, .acl = { 0xff } },
    { .fid = 0x0000, .parent = 0xff, .name = NULL, .type = FILE_TYPE_NOT_KNOWN, .data = NULL, .ef_structure = 0, .acl = { 0 } }  //end
};

const file_t *MF = &file_entries[0];
const file_t *file_last = &file_entries[sizeof(file_entries) / sizeof(file_t) - 1];
file_t *ef_keydev = NULL;
file_t *ef_certdev = NULL;
file_t *ef_counter = NULL;
file_t *ef_pin = NULL;
file_t *ef_authtoken = NULL;
file_t *ef_keydev_enc = NULL;
file_t *ef_largeblob = NULL;
file_t *ef_mkek = NULL;
