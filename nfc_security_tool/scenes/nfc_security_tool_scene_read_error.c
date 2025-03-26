#include "../nfc_security_tool_i.h"

void nfc_security_tool_scene_read_error_on_enter(void* context) {
    NfcSecurityTool* app = context;
    Widget* widget = app->widget;

    // Clear widget
    widget_reset(widget);

    // Add header
    widget_add_string_element(widget, 64, 12, AlignCenter, AlignTop, FontPrimary, "Error");
    widget_add_string_element(
        widget, 64, 32, AlignCenter, AlignTop, FontSecondary, "Failed to read card");

    // Add button
    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", scene_manager_previous_scene_callback, app);
    widget_add_button_element(
        widget,
        GuiButtonTypeRight,
        "Menu",
        scene_manager_search_and_switch_to_previous_scene_callback,
        app);

    // Switch to widget view
    view_dispatcher_switch_to_view(app->view_dispatcher, NfcSecurityToolViewWidget);
}

bool nfc_security_tool_scene_read_error_on_event(void* context, SceneManagerEvent event) {
    NfcSecurityTool* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case GuiButtonTypeLeft:
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
            break;
        case GuiButtonTypeRight:
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, NfcSecurityToolSceneStart);
            consumed = true;
            break;
        }
    }
    return consumed;
}

void nfc_security_tool_scene_read_error_on_exit(void* context) {
    NfcSecurityTool* app = context;
    widget_reset(app->widget);
}
