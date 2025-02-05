

#include "file.h"
#include "fido.h"
#include "ctap.h"
#if !defined(ENABLE_EMULATION) && !defined(ESP_PLATFORM)
#include "bsp/board.h"
#endif
#ifdef ESP_PLATFORM
#include "esp_compat.h"
#endif
#include "fs/phy.h"

extern void scan_all();

int cbor_reset() {
#ifndef ENABLE_EMULATION
#if defined(ENABLE_POWER_ON_RESET) && ENABLE_POWER_ON_RESET == 1
    if (!(phy_data.opts & PHY_OPT_DISABLE_POWER_RESET) && board_millis() > 10000) {
        return CTAP2_ERR_NOT_ALLOWED;
    }
#endif
    if (wait_button_pressed() == true) {
        return CTAP2_ERR_USER_ACTION_TIMEOUT;
    }
#endif
    initialize_flash(true);
    init_fido();
    return 0;
}
