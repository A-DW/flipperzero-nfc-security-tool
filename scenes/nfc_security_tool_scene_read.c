#include "../nfc_security_tool_i.h"

void nfc_security_tool_scene_read_on_enter(void* context) {
    NfcSecurityTool* app = context;

    // Configure loading view
    loading_set_header(app->loading, "Searching for NFC card...");
    view_dispatcher_switch_to_view(app->view_dispatcher, NfcSecurityToolViewLoading);

    // Start NFC worker
    nfc_worker_start(app->nfc->worker, NfcWorkerStateRead, &app->dev_data, NULL, NULL);

    notification_message(app->notifications, &sequence_blink_start_blue);
}

bool nfc_security_tool_scene_read_on_event(void* context, SceneManagerEvent event) {
    NfcSecurityTool* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case NfcSecurityToolCustomEventCardDetected:
            notification_message(app->notifications, &sequence_success);
            scene_manager_next_scene(app->scene_manager, NfcSecurityToolSceneReadSuccess);
            consumed = true;
            break;
        case NfcSecurityToolCustomEventCardLost:
            notification_message(app->notifications, &sequence_blink_start_blue);
            consumed = true;
            break;
        }
    }
    return consumed;
}

void nfc_security_tool_scene_read_on_exit(void* context) {
    NfcSecurityTool* app = context;

    // Stop notifications
    notification_message(app->notifications, &sequence_blink_stop);

    // Stop NFC worker
    nfc_worker_stop(app->nfc->worker);
}
