#include "nfc_security_tool_i.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_nfc.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>
#include <toolbox/stream/file_stream.h>
#include <toolbox/path.h>
#include <toolbox/stream/file_stream.h>
#include <furi/core/string.h>

// NFC protocol includes
#include <lib/nfc/nfc.h>
#include <lib/nfc/nfc_device.h>
#include <lib/nfc/protocols/nfc_protocol.h>
#include <lib/nfc/protocols/nfc_device_base.h>
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a.h>
#include <lib/nfc/protocols/iso14443_4a/iso14443_4a.h>
#include <lib/nfc/protocols/mf_classic/mf_classic.h>
#include <lib/nfc/protocols/mf_ultralight/mf_ultralight.h>
#include <lib/nfc/protocols/mf_ultralight/mf_ultralight_poller.h>

#include "mifare_classic_security.h"

// Application tag for logging
#define TAG              "NfcSecurityTool"
// Buffer size for NFC operations
#define NFC_BUFFER_SIZE  1024
#define MENU_ITEMS_COUNT 5

// Custom notification sequences
static const NotificationSequence sequence_success = { 
    &message_green_255,
    &message_green_255,
    &message_delay_50,
    &message_green_0,
    NULL,
};

static const NotificationSequence sequence_error = { 
    &message_red_255,
    &message_red_255,
    &message_delay_50,
    &message_red_0,
    NULL,
};

static const NotificationSequence sequence_blink_start_blue = { 
    &message_blue_255,
    &message_blue_255,
    NULL,
};

// Application state enum
typedef enum {
    NfcSecurityToolStateIdle = 0,
    NfcSecurityToolStateReading,
    NfcSecurityToolStateEmulating,
    NfcSecurityToolStateAnalyzing,
    NfcSecurityToolStateCloning,
} NfcSecurityToolState;

// UI state structure
typedef struct {
    char detailed_status[128];
    char card_info[64];
    uint8_t selected_menu_item;
    bool is_processing;
} NfcSecurityToolUI;

// Main application structure
typedef struct {
    NfcSecurityToolState state;
    NfcSecurityToolUI ui;
    NotificationApp* notifications;
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    bool card_detected;
    NfcDevice* device;
    uint8_t uid[10];
    uint8_t uid_len;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    Submenu* submenu;
    Popup* popup;
    Loading* loading;
    TextBox* text_box;
    Widget* widget;
    Nfc* nfc;
    uint8_t security_level;
} NfcSecurityTool;

// Menu items as string constants
static const char* const menu_labels[MENU_ITEMS_COUNT] =
    {"Read Card", "Analyze Security", "Emulate Card", "Clone Card", "Exit"};

// Function declarations
static void draw_callback(Canvas* canvas, void* ctx);
static void input_callback(InputEvent* input_event, void* ctx);
static bool nfc_read_card(NfcSecurityTool* app);
static void nfc_analyze_card(NfcSecurityTool* app);
static void nfc_emulate_card(NfcSecurityTool* app);
static void nfc_clone_card(NfcSecurityTool* app);

// Enhanced draw callback
static void draw_callback(Canvas* canvas, void* ctx) {
    NfcSecurityTool* app = ctx;
    canvas_clear(canvas);

    // Draw title with border
    canvas_draw_frame(canvas, 0, 0, 128, 14);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 11, AlignCenter, AlignBottom, "NFC Security Tool");

    // Draw status bar
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_frame(canvas, 0, 16, 128, 12);
    if(app->card_detected) {
        canvas_draw_str_aligned(canvas, 64, 25, AlignCenter, AlignBottom, app->ui.card_info);
    } else {
        canvas_draw_str_aligned(canvas, 64, 25, AlignCenter, AlignBottom, "No Card Detected");
    }

    // Draw menu items
    for(int i = 0; i < MENU_ITEMS_COUNT; i++) {
        if(i == app->ui.selected_menu_item) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 0, 30 + (i * 12), 128, 12);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }
        canvas_draw_str(canvas, 4, 39 + (i * 12), menu_labels[i]);
    }

    // Draw detailed status at the bottom
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, 0, 94, 128, 34);
    canvas_set_font(canvas, FontSecondary);
    string_fit_width(app->ui.detailed_status, 120);
    canvas_draw_str_aligned(canvas, 64, 107, AlignCenter, AlignBottom, app->ui.detailed_status);
}

// Input callback for handling user interactions
static void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    NfcSecurityTool* app = ctx;

    if(input_event->type == InputTypeShort) {
        switch(input_event->key) {
        case InputKeyUp:
            if(app->ui.selected_menu_item > 0) app->ui.selected_menu_item--;
            break;
        case InputKeyDown:
            if(app->ui.selected_menu_item < MENU_ITEMS_COUNT - 1) app->ui.selected_menu_item++;
            break;
        case InputKeyOk:
            switch(app->ui.selected_menu_item) {
            case 0: // Read Card
                nfc_read_card(app);
                break;
            case 1: // Analyze Security
                nfc_analyze_card(app);
                break;
            case 2: // Emulate Card
                nfc_emulate_card(app);
                break;
            case 3: // Clone Card
                nfc_clone_card(app);
                break;
            case 4: // Exit
                // Handle exit 
    nfc_worker_stop(app->nfc->worker);
                break;
            }
            break;
        case InputKeyBack:
            // Handle back button
            break;
        default:
            break;
        }
    }
}

// Enhanced card reading function
static bool nfc_read_card(NfcSecurityTool* app) {
    bool success = false;

    snprintf(app->ui.detailed_status, sizeof(app->ui.detailed_status), "Searching for card...");

    // Configure NFC hardware for reading
    if(furi_hal_nfc_acquire() == FuriHalNfcErrorNone) {
        if(furi_hal_nfc_set_mode(FuriHalNfcModePoller, FuriHalNfcTechIso14443a) ==
           FuriHalNfcErrorNone) {
            if(furi_hal_nfc_poller_field_on() == FuriHalNfcErrorNone) {
                // Wait for card presence
                FuriHalNfcEvent event = furi_hal_nfc_poller_wait_event(100);
                if(event & FuriHalNfcEventFieldOn) {
                    app->card_detected = true;

                    // Free previous device if it exists
                    if(app->device) {
                        nfc_device_free(app->device);
                    }

                    // Initialize new device
                    app->device = nfc_device_alloc();
                    if(app->device) {
                        success = true;
                        snprintf(app->ui.card_info, sizeof(app->ui.card_info), "Card detected");
                        notification_message(app->notifications, &sequence_success);
                    }
                }
            }
        }
        furi_hal_nfc_release();
    }

    if(!success) {
        snprintf(app->ui.card_info, sizeof(app->ui.card_info), "No card detected");
        notification_message(app->notifications, &sequence_error);
    }

    return success;
}

// Enhanced security analysis function
static void nfc_analyze_card(NfcSecurityTool* app) {
    if(!app->card_detected) {
        snprintf(
            app->ui.detailed_status, sizeof(app->ui.detailed_status), "Please read a card first");
        return;
    }

    // Display security analysis results
    snprintf(
        app->ui.detailed_status,
        sizeof(app->ui.detailed_status),
        "Security Level: %d/10\n%s",
        0, // TODO: Implement security level calculation
        "Security analysis in progress...");

    app->state = NfcSecurityToolStateAnalyzing;
}

// Enhanced emulation function
static void nfc_emulate_card(NfcSecurityTool* app) {
    if(!app->card_detected || !app->device) {
        snprintf(
            app->ui.detailed_status, sizeof(app->ui.detailed_status), "Please read a card first");
        return;
    }

    // Configure emulation
    if(furi_hal_nfc_acquire() == FuriHalNfcErrorNone) {
        if(furi_hal_nfc_set_mode(FuriHalNfcModeListener, FuriHalNfcTechIso14443a) ==
           FuriHalNfcErrorNone) {
            app->state = NfcSecurityToolStateEmulating;
            snprintf(
                app->ui.detailed_status,
                sizeof(app->ui.detailed_status),
                "Emulating card...\nUID: %02X:%02X:%02X:%02X",
                app->uid[0],
                app->uid[1],
                app->uid[2],
                app->uid[3]);

            notification_message(app->notifications, &sequence_blink_start_blue);
        }
    }
    furi_hal_nfc_release();
}

// Enhanced clone function
static void nfc_clone_card(NfcSecurityTool* app) {
    if(!app->card_detected) {
        snprintf(
            app->ui.detailed_status, sizeof(app->ui.detailed_status), "Please read a card first");
        return;
    }

    app->state = NfcSecurityToolStateCloning;
    snprintf(
        app->ui.detailed_status, sizeof(app->ui.detailed_status), "Place blank card to clone");
}

static bool nfc_security_tool_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    NfcSecurityTool* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool nfc_security_tool_back_event_callback(void* context) {
    furi_assert(context);
    NfcSecurityTool* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void nfc_security_tool_tick_event_callback(void* context) {
    furi_assert(context);
    NfcSecurityTool* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

NfcSecurityTool* nfc_security_tool_alloc() {
    NfcSecurityTool* app = malloc(sizeof(NfcSecurityTool));

    // GUI
    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // View Dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, nfc_security_tool_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, nfc_security_tool_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, nfc_security_tool_tick_event_callback, 100);

    // Scene Manager
    app->scene_manager = scene_manager_alloc(&nfc_security_tool_scene_handlers, app);

    // Views
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, NfcSecurityToolViewMenu, submenu_get_view(app->submenu));

    app->popup = popup_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, NfcSecurityToolViewPopup, popup_get_view(app->popup));

    app->loading = loading_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, NfcSecurityToolViewLoading, loading_get_view(app->loading));

    app->text_box = text_box_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, NfcSecurityToolViewTextBox, text_box_get_view(app->text_box));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, NfcSecurityToolViewWidget, widget_get_view(app->widget));

    // NFC
    app->nfc = nfc_alloc();
    app->device = NULL;
    app->card_detected = false;
    app->security_level = 0;

    return app;
}

void nfc_security_tool_free(NfcSecurityTool* app) {
    furi_assert(app);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, NfcSecurityToolViewMenu);
    view_dispatcher_remove_view(app->view_dispatcher, NfcSecurityToolViewPopup);
    view_dispatcher_remove_view(app->view_dispatcher, NfcSecurityToolViewLoading);
    view_dispatcher_remove_view(app->view_dispatcher, NfcSecurityToolViewTextBox);
    view_dispatcher_remove_view(app->view_dispatcher, NfcSecurityToolViewWidget);

    submenu_free(app->submenu);
    popup_free(app->popup);
    loading_free(app->loading);
    text_box_free(app->text_box);
    widget_free(app->widget);

    // Scene Manager
    scene_manager_free(app->scene_manager);

    // View Dispatcher
    view_dispatcher_free(app->view_dispatcher);

    // NFC
    nfc_free(app->nfc);
    if(app->device) {
        nfc_device_free(app->device);
    }

    // Records
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);

    free(app);
}

int32_t nfc_security_tool_app(void* p) {
    UNUSED(p);
    NfcSecurityTool* app = nfc_security_tool_alloc();

    // Start application
    scene_manager_next_scene(app->scene_manager, NfcSecurityToolSceneStart);
    view_dispatcher_run(app->view_dispatcher);

    // Free resources
    nfc_security_tool_free(app);
    return 0;
}
