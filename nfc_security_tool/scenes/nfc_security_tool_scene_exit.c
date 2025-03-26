#include "../nfc_security_tool_i.h"

void nfc_security_tool_scene_exit_on_enter(void* context) {
    NfcSecurityTool* app = context;

    // Stop any ongoing NFC operations
    nfc_worker_stop(app->nfc->worker);

    // Stop any notifications
    notification_message(app->notifications, &sequence_blink_stop);

    // Request exit
    view_dispatcher_stop(app->view_dispatcher);
}

bool nfc_security_tool_scene_exit_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfc_security_tool_scene_exit_on_exit(void* context) {
    UNUSED(context);
}
