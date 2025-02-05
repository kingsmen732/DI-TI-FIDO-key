

#ifndef _KEK_H_
#define _KEK_H_

#include "crypto_utils.h"
#if defined(ENABLE_EMULATION) || defined(ESP_PLATFORM)
#include <stdbool.h>
#endif


extern int load_mkek(uint8_t *);
extern int store_mkek(const uint8_t *);
extern void init_mkek();
extern void release_mkek(uint8_t *);
extern int mkek_encrypt(uint8_t *data, uint16_t len);
extern int mkek_decrypt(uint8_t *data, uint16_t len);

#define MKEK_IV_SIZE     (IV_SIZE)
#define MKEK_KEY_SIZE    (32)
#define MKEK_KEY_CS_SIZE (4)
#define MKEK_SIZE        (MKEK_IV_SIZE + MKEK_KEY_SIZE + MKEK_KEY_CS_SIZE)
#define MKEK_IV(p)       (p)
#define MKEK_KEY(p)      (MKEK_IV(p) + MKEK_IV_SIZE)
#define MKEK_CHECKSUM(p) (MKEK_KEY(p) + MKEK_KEY_SIZE)
#define DKEK_KEY_SIZE    (32)

extern uint8_t mkek_mask[MKEK_KEY_SIZE];
extern bool has_mkek_mask;

#endif
