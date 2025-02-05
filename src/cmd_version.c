
#include "apdu.h"
#include "pico_keys.h"

int cmd_version() {
    memcpy(res_APDU, "U2F_V2", strlen("U2F_V2"));
    res_APDU_size = (uint16_t)strlen("U2F_V2");
    return SW_OK();
}
