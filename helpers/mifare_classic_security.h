#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <furi_hal.h>

// NFC includes
#include <lib/nfc/nfc.h>
#include <lib/nfc/nfc_device.h>
#include <lib/nfc/protocols/nfc_protocol.h>
#include <lib/nfc/protocols/nfc_device_base.h>
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a.h>
#include <lib/nfc/protocols/mf_classic/mf_classic.h>
#include <lib/nfc/protocols/mf_ultralight/mf_ultralight.h>

// Constants
#define MIFARE_CLASSIC_KEY_SIZE    6
#define MIFARE_CLASSIC_BLOCK_SIZE  16
#define MIFARE_CLASSIC_SECTOR_SIZE 4
#define MIFARE_CLASSIC_SECTORS_MAX 40

// Card types
typedef enum {
    CardTypeUnknown,
    CardTypeMifareClassic,
    CardTypeMifareUltralight,
    CardTypeNTAG,
} CardType;

// Security analysis results
typedef struct {
    bool has_default_keys;
    bool has_weak_keys;
    bool has_nested_attack;
    bool has_darkside_attack;
    bool has_hardnested_attack;
    bool has_staticnested_attack;
    bool has_uid_firmware;
    bool has_chinese_magic;
} SecurityAnalysisResults;

// Card information
typedef struct {
    uint8_t uid[10];
    uint8_t uid_len;
    uint8_t atqa[2];
    uint8_t sak;
    char manufacturer[32];
    CardType type;
    SecurityAnalysisResults security;
} CardInfo;

// Error codes
typedef enum {
    ErrorNone,
    ErrorTimeout,
    ErrorNoCard,
    ErrorCommunication,
    ErrorInvalidKey,
    ErrorInvalidSector,
    ErrorInvalidBlock,
    ErrorInvalidCommand,
    ErrorInvalidResponse,
    ErrorInvalidFormat,
    ErrorUnknown,
} MifareClassicError;

// Function declarations
CardType detect_card_type(FuriHalNfc* nfc);
MifareClassicError test_default_keys(FuriHalNfc* nfc, CardInfo* card);
MifareClassicError check_known_vulnerabilities(FuriHalNfc* nfc, CardInfo* card);
void get_card_details(CardInfo* card, char* output, size_t output_size);
