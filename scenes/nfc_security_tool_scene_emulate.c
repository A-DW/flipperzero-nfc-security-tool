#include "../nfc_security_tool_i.h"

void nfc_security_tool_scene_emulate_on_enter(void* context) {
    NfcSecurityTool* app = context;
    Widget* widget = app->widget;

    // Clear widget
    widget_reset(widget);

    // Add header
    widget_add_string_element(widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Emulating Card");

    // Add card info
    char card_type[32];
    if(app->dev_data.protocol == NfcDeviceProtocolMifareClassic) {
        snprintf(card_type, sizeof(card_type), "MIFARE Classic");
    } else if(app->dev_data.protocol == NfcDeviceProtocolMifareUl) {
        snprintf(card_type, sizeof(card_type), "MIFARE Ultralight");
    } else if(app->dev_data.protocol == NfcDeviceProtocolIso14443_3a) {
        snprintf(card_type, sizeof(card_type), "ISO14443-3A");
    } else if(app->dev_data.protocol == NfcDeviceProtocolIso14443_4a) {
        snprintf(card_type, sizeof(card_type), "ISO14443-4A");
    } else {
        snprintf(card_type, sizeof(card_type), "Unknown");
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

    // Add status
    widget_add_string_element(
        widget, 64, 44, AlignCenter, AlignTop, FontSecondary, "Ready to scan");

    // Switch to widget view
    view_dispatcher_switch_to_view(app->view_dispatcher, NfcSecurityToolViewWidget);

    // Start NFC worker in emulation mode
    nfc_worker_start(app->nfc->worker, NfcWorkerStateEmulate, &app->dev_data, NULL, NULL);

    // Start blinking LED
    notification_message(app->notifications, &sequence_blink_start_magenta);
}

bool nfc_security_tool_scene_emulate_on_event(void* context, SceneManagerEvent event) {
    NfcSecurityTool* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        // Stop emulation
        nfc_worker_stop(app->nfc->worker);
        notification_message(app->notifications, &sequence_blink_stop);

        // Return to previous scene
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }

    return consumed;
}

void nfc_security_tool_scene_emulate_on_exit(void* context) {
    NfcSecurityTool* app = context;

    // Stop emulation if still running
    nfc_worker_stop(app->nfc->worker);

    // Stop LED blinking
    notification_message(app->notifications, &sequence_blink_stop);

    // Reset widget
    widget_reset(app->widget);
}
