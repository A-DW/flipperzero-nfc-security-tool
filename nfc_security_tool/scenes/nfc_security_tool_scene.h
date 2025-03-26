#pragma once

#include <gui/scene_manager.h>

// Scene handlers configuration
extern const SceneManagerHandlers nfc_security_tool_scene_handlers;

// Scene enumeration
#include "nfc_security_tool_scene_config.h"

// Generate scene id enum
#define ADD_SCENE(prefix, name, id) NfcSecurityToolScene##id,
typedef enum {
#include "nfc_security_tool_scene_config.h"
    NfcSecurityToolSceneNum,
} NfcSecurityToolScene;
#undef ADD_SCENE

// Generate scene prototypes
#define ADD_SCENE(prefix, name, id)                                                \
    void prefix##_scene_##name##_on_enter(void* context);                          \
    bool prefix##_scene_##name##_on_event(void* context, SceneManagerEvent event); \
    void prefix##_scene_##name##_on_exit(void* context);
#include "nfc_security_tool_scene_config.h"
#undef ADD_SCENE
