#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification.h>
#include <furi.h>
#include <furi_hal.h>

#include "helpers/mifare_classic_security.h"
#include "scenes/nfc_security_tool_scene.h"

#define NFC_SECURITY_TOOL_TEXT_STORE_SIZE 4096

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct NfcSecurityTool NfcSecurityTool;

// Application allocation/deallocation
NfcSecurityTool* nfc_security_tool_alloc();
void nfc_security_tool_free(NfcSecurityTool* app);

// Application entry point
int32_t nfc_security_tool_app(void* p);

#ifdef __cplusplus
}
#endif
