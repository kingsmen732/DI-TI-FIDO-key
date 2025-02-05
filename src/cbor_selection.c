

#include "fido.h"
#include "ctap.h"

int cbor_selection() {
    if (wait_button_pressed() == true) {
        return CTAP2_ERR_USER_ACTION_TIMEOUT;
    }
    return CTAP2_OK;
}
