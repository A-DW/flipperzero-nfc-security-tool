#pragma once

#include <gui/scene_manager.h>

// Generate scene id and total number
#define ADD_SCENE(prefix, name, id) NfcSecurityToolScene##id,
typedef enum {
#include "nfc_security_tool_scene_config.h"
    NfcSecurityToolSceneNum,
} NfcSecurityToolScene;
#undef ADD_SCENE

// Generate scene on_enter handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
extern void (*const nfc_security_tool_scene_on_enter_handlers[])(void*);
#undef ADD_SCENE

// Generate scene on_event handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
extern bool (*const nfc_security_tool_scene_on_event_handlers[])(void*, SceneManagerEvent);
#undef ADD_SCENE

// Generate scene on_exit handlers array
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
extern void (*const nfc_security_tool_scene_on_exit_handlers[])(void*);
#undef ADD_SCENE

// Add scenes here
ADD_SCENE(nfc_security_tool, start, Start)
ADD_SCENE(nfc_security_tool, read, Read)
ADD_SCENE(nfc_security_tool, read_success, ReadSuccess)
ADD_SCENE(nfc_security_tool, read_error, ReadError)
ADD_SCENE(nfc_security_tool, analyze, Analyze)
ADD_SCENE(nfc_security_tool, emulate, Emulate)
ADD_SCENE(nfc_security_tool, clone, Clone)
ADD_SCENE(nfc_security_tool, exit, Exit)
