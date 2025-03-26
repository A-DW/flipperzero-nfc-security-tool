cmake_minimum_required(VERSION 3.5)

project(nfc_security_tool)

add_library(
    nfc_security_tool
    STATIC
    nfc_security_tool.c
    helpers/mifare_classic_security.c
    scenes/nfc_security_tool_scene.c
    scenes/nfc_security_tool_scene_analyze.c
    scenes/nfc_security_tool_scene_clone.c
)

target_include_directories(
    nfc_security_tool
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers
    ${CMAKE_CURRENT_SOURCE_DIR}/scenes
)

# Ignore unused parameter warnings for SDK headers
target_compile_options(
    nfc_security_tool
    PRIVATE
    -Wno-error=unused-parameter
)

add_flipper_application(
    nfc_security_tool
    SOURCES
    nfc_security_tool.c
    TARGETS nfc_security_tool
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    APP_NFC_SECURITY_TOOL
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        furi
        furi-hal
        gui
        dialogs
        toolbox
        nfc
        nfc_protocols
        nfc_workers
        nfc_helpers
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wno-unused-parameter
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${FLIPPER_APP_DIR}/apps/NFC
) 