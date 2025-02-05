

#ifndef _FILES_H_
#define _FILES_H_

#include "file.h"

#define EF_KEY_DEV      0xCC00
#define EF_KEY_DEV_ENC  0xCC01
#define EF_MKEK         0xCC0F
#define EF_EE_DEV       0xCE00
#define EF_EE_DEV_EA    0xCE01
#define EF_COUNTER      0xC000
#define EF_OPTS         0xC001
#define EF_PIN          0x1080
#define EF_AUTHTOKEN    0x1090
#define EF_MINPINLEN    0x1100
#define EF_DEV_CONF     0x1122
#define EF_CRED         0xCF00 // Creds at 0xCF00 - 0xCFFF
#define EF_RP           0xD000 // RPs at 0xD000 - 0xD0FF
#define EF_LARGEBLOB    0x1101 // Large Blob Array
#define EF_OATH_CRED    0xBA00 // OATH Creds at 0xBA00 - 0xBAFE
#define EF_OATH_CODE    0xBAFF
#define EF_OTP_SLOT1    0xBB00
#define EF_OTP_SLOT2    0xBB01
#define EF_OTP_PIN      0x10A0 // Nitrokey OTP PIN

extern file_t *ef_keydev;
extern file_t *ef_certdev;
extern file_t *ef_counter;
extern file_t *ef_pin;
extern file_t *ef_authtoken;
extern file_t *ef_keydev_enc;
extern file_t *ef_largeblob;
extern file_t *ef_mkek;

#endif //_FILES_H_
