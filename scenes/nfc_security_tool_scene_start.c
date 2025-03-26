#include "../nfc_security_tool_i.h"

enum SubmenuIndex {
    SubmenuIndexRead,
    SubmenuIndexAnalyze,
    SubmenuIndexEmulate,
    SubmenuIndexClone,
    SubmenuIndexExit,
};

void nfc_security_tool_scene_start_submenu_callback(void* context, uint32_t index) {
    NfcSecurityTool* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void nfc_security_tool_scene_start_on_enter(void* context) {
    NfcSecurityTool* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu,
        "Read Card",
        SubmenuIndexRead,
        nfc_security_tool_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Analyze Security",
        SubmenuIndexAnalyze,
        nfc_security_tool_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Emulate Card",
        SubmenuIndexEmulate,
        nfc_security_tool_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Clone Card",
        SubmenuIndexClone,
        nfc_security_tool_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu, "Exit", SubmenuIndexExit, nfc_security_tool_scene_start_submenu_callback, app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, NfcSecurityToolSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, NfcSecurityToolViewMenu);
}

bool nfc_security_tool_scene_start_on_event(void* context, SceneManagerEvent event) {
    NfcSecurityTool* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SubmenuIndexRead:
            scene_manager_next_scene(app->scene_manager, NfcSecurityToolSceneRead);
            consumed = true;
            break;
        case SubmenuIndexAnalyze:
            scene_manager_next_scene(app->scene_manager, NfcSecurityToolSceneAnalyze);
            consumed = true;
            break;
        case SubmenuIndexEmulate:
            scene_manager_next_scene(app->scene_manager, NfcSecurityToolSceneEmulate);
            consumed = true;
            break;
        case SubmenuIndexClone:
            scene_manager_next_scene(app->scene_manager, NfcSecurityToolSceneClone);
            consumed = true;
            break;
        case SubmenuIndexExit:
            scene_manager_next_scene(app->scene_manager, NfcSecurityToolSceneExit);
            consumed = true;
            break;
        }
        scene_manager_set_scene_state(app->scene_manager, NfcSecurityToolSceneStart, event.event);
    }

    return consumed;
}

void nfc_security_tool_scene_start_on_exit(void* context) {
    NfcSecurityTool* app = context;
    submenu_reset(app->submenu);
}
