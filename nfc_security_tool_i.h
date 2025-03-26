#pragma once

#include "nfc_security_tool.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/loading.h>
#include <gui/modules/text_box.h>
#include <gui/modules/widget.h>
#include <notification/notification_messages.h>
#include <furi_hal.h>

// NFC includes
#include <lib/nfc/nfc.h>
#include <lib/nfc/nfc_device.h>
#include <lib/nfc/protocols/nfc_protocol.h>
#include <lib/nfc/protocols/nfc_device_base.h>
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a.h>
#include <lib/nfc/protocols/iso14443_4a/iso14443_4a.h>
#include <lib/nfc/protocols/mf_classic/mf_classic.h>
#include <lib/nfc/protocols/mf_ultralight/mf_ultralight.h>

// Local includes
#include "helpers/mifare_classic_security.h"
#include "scenes/nfc_security_tool_scene.h"

// Define view indices
typedef enum {
    NfcSecurityToolViewMenu,
    NfcSecurityToolViewPopup,
    NfcSecurityToolViewLoading,
    NfcSecurityToolViewTextBox,
    NfcSecurityToolViewWidget,
} NfcSecurityToolView;

// Main application structure
struct NfcSecurityTool {
    // GUI
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    // Views
    Submenu* submenu;
    Popup* popup;
    Loading* loading;
    TextBox* text_box;
    Widget* widget;

    // NFC
    Nfc* nfc;
    NfcDevice* device;
    FuriHalNfcDevData dev_data;

    // State
    char text_box_store[NFC_SECURITY_TOOL_TEXT_STORE_SIZE];
    bool card_detected;
    uint8_t security_level;
};

// Custom events
typedef enum {
    NfcSecurityToolCustomEventViewExit = 0x100,
    NfcSecurityToolCustomEventCardDetected,
    NfcSecurityToolCustomEventCardLost,
    NfcSecurityToolCustomEventAnalysisComplete,
} NfcSecurityToolCustomEvent;

// Scene handlers
void nfc_security_tool_scene_start_on_enter(void* context);
bool nfc_security_tool_scene_start_on_event(void* context, SceneManagerEvent event);
void nfc_security_tool_scene_start_on_exit(void* context);

void nfc_security_tool_scene_read_on_enter(void* context);
bool nfc_security_tool_scene_read_on_event(void* context, SceneManagerEvent event);
void nfc_security_tool_scene_read_on_exit(void* context);

void nfc_security_tool_scene_read_success_on_enter(void* context);
bool nfc_security_tool_scene_read_success_on_event(void* context, SceneManagerEvent event);
void nfc_security_tool_scene_read_success_on_exit(void* context);

void nfc_security_tool_scene_read_error_on_enter(void* context);
bool nfc_security_tool_scene_read_error_on_event(void* context, SceneManagerEvent event);
void nfc_security_tool_scene_read_error_on_exit(void* context);

void nfc_security_tool_scene_analyze_on_enter(void* context);
bool nfc_security_tool_scene_analyze_on_event(void* context, SceneManagerEvent event);
void nfc_security_tool_scene_analyze_on_exit(void* context);

void nfc_security_tool_scene_emulate_on_enter(void* context);
bool nfc_security_tool_scene_emulate_on_event(void* context, SceneManagerEvent event);
void nfc_security_tool_scene_emulate_on_exit(void* context);

void nfc_security_tool_scene_clone_on_enter(void* context);
bool nfc_security_tool_scene_clone_on_event(void* context, SceneManagerEvent event);
void nfc_security_tool_scene_clone_on_exit(void* context);

void nfc_security_tool_scene_exit_on_enter(void* context);
bool nfc_security_tool_scene_exit_on_event(void* context, SceneManagerEvent event);
void nfc_security_tool_scene_exit_on_exit(void* context);

// Widget callback
void nfc_security_tool_widget_callback(GuiButtonType result, InputType type, void* context);
