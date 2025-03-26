#include "../nfc_security_tool_i.h"
#include <dolphin/dolphin.h>

void nfc_security_tool_scene_read_success_on_enter(void* context) {
    NfcSecurityTool* app = context;
    Widget* widget = app->widget;

    // Clear widget
    widget_reset(widget);

    // Add header
    widget_add_string_element(
        widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Card Read Success!");

    // Add card type
    char card_type[32];
    if(app->dev_data.protocol == NfcDeviceProtocolMifareClassic) {
        snprintf(card_type, sizeof(card_type), "Type: MIFARE Classic");
    } else if(app->dev_data.protocol == NfcDeviceProtocolMifareUl) {
        snprintf(card_type, sizeof(card_type), "Type: MIFARE Ultralight");
    } else if(app->dev_data.protocol == NfcDeviceProtocolIso14443_3a) {
        snprintf(card_type, sizeof(card_type), "Type: ISO14443-3A");
    } else if(app->dev_data.protocol == NfcDeviceProtocolIso14443_4a) {
        snprintf(card_type, sizeof(card_type), "Type: ISO14443-4A");
    } else {
        snprintf(card_type, sizeof(card_type), "Type: Unknown");
    }
    widget_add_string_element(widget, 64, 20, AlignCenter, AlignTop, FontSecondary, card_type);

    // Add UID
    char uid_str[32];
    snprintf(
        uid_str,
        sizeof(uid_str),
        "UID: %02X %02X %02X %02X",
        app->dev_data.uid[0],
        app->dev_data.uid[1],
        app->dev_data.uid[2],
        app->dev_data.uid[3]);
    widget_add_string_element(widget, 64, 32, AlignCenter, AlignTop, FontSecondary, uid_str);

    // Add button
    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Back", scene_manager_previous_scene_callback, app);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "Analyze", scene_manager_next_scene_callback, app);

    // Switch to widget view
    view_dispatcher_switch_to_view(app->view_dispatcher, NfcSecurityToolViewWidget);

    // Record activity
    dolphin_deed(DolphinDeedNfcRead);
}

bool nfc_security_tool_scene_read_success_on_event(void* context, SceneManagerEvent event) {
    NfcSecurityTool* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case GuiButtonTypeLeft:
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
            break;
        case GuiButtonTypeRight:
            scene_manager_next_scene(app->scene_manager, NfcSecurityToolSceneAnalyze);
            consumed = true;
            break;
        }
    }
    return consumed;
}

void nfc_security_tool_scene_read_success_on_exit(void* context) {
    NfcSecurityTool* app = context;
    widget_reset(app->widget);
}
