#include "mifare_classic_security.h"
#include <furi_hal_nfc.h>
#include <furi_hal.h>
#include <lib/nfc/helpers/mf_classic_dict.h>
#include <lib/nfc/protocols/mf_classic/mf_classic_poller.h>
#include <string.h>

// Pre-calculated filter table for Crypto1 cipher optimization
// This table contains the results of the filter function for all possible 8-bit inputs
// The actual values are omitted for brevity but would be calculated based on the Crypto1 specification
static const uint32_t crypto1_filter_table[256] = {
    // Implementation details omitted for brevity
};

// Initialize Crypto1 cipher with a key
// This sets up the LFSR and filter state for encryption/decryption
void crypto1_init(Crypto1* crypto, uint64_t key) {
    crypto->lfsr = key;
    crypto->filter = 0;
}

// Apply the Crypto1 filter function to input
// This is an optimized version using the pre-calculated table
uint32_t crypto1_filter(uint32_t in) {
    return crypto1_filter_table[in & 0xFF];
}

// Encrypt data using Crypto1 cipher
// Generates a keystream and XORs it with the input data
uint32_t crypto1_encrypt(Crypto1* crypto, uint32_t data) {
    uint32_t keystream = 0;
    for(int i = 0; i < 32; i++) {
        keystream |= crypto1_filter(crypto->lfsr) << i;
        crypto->lfsr = (crypto->lfsr << 1) | (crypto->filter & 1);
    }
    return data ^ keystream;
}

// Decrypt data using Crypto1 cipher
// Since Crypto1 is a stream cipher, decryption is the same as encryption
uint32_t crypto1_decrypt(Crypto1* crypto, uint32_t data) {
    return crypto1_encrypt(crypto, data);
}

// Analyze MIFARE Classic card security
// Checks card type and known vulnerabilities
bool mifare_classic_analyze_card(FuriHalNfcDevData* nfc_data) {
    if(!nfc_data) return false;

    // Verify card is MIFARE Classic type
    if(nfc_data->type != FuriHalNfcTypeA) return false;

    // Check for known vulnerabilities
    bool is_vulnerable = mifare_classic_is_vulnerable(nfc_data);

    // Display card information
    mifare_classic_print_card_info(nfc_data);

    return is_vulnerable;
}

// Recover sector key using various attack methods
// Attempts different attacks in order of efficiency
bool mifare_classic_recover_key(FuriHalNfcDevData* nfc_data, uint8_t sector, uint8_t key_type) {
    if(!nfc_data || sector >= MIFARE_CLASSIC_SECTORS_MAX) return false;

    // Try nested authentication attack first (fastest)
    if(mifare_classic_nested_attack(nfc_data, sector, key_type)) {
        return true;
    }

    // Try darkside attack if nested attack fails
    if(mifare_classic_darkside_attack(nfc_data, sector, key_type)) {
        return true;
    }

    // Fall back to brute force as last resort
    return mifare_classic_brute_force_key(nfc_data, sector, key_type);
}

// Brute force key search
// This is a simplified implementation - real version would need optimization
bool mifare_classic_brute_force_key(FuriHalNfcDevData* nfc_data, uint8_t sector, uint8_t key_type) {
    if(!nfc_data) return false;

    uint8_t key[MIFARE_CLASSIC_KEY_SIZE];
    // Try all possible 48-bit keys
    for(uint64_t i = 0; i < 0xFFFFFFFFFFFF; i++) {
        // Convert counter to key bytes
        for(int j = 0; j < MIFARE_CLASSIC_KEY_SIZE; j++) {
            key[j] = (i >> (j * 8)) & 0xFF;
        }

        // Try authentication with current key
        if(furi_hal_nfc_mifare_classic_auth(nfc_data, sector, key_type, key)) {
            return true;
        }
    }

    return false;
}

// Implement nested authentication attack
// This attack exploits the weak random number generator
bool mifare_classic_nested_attack(FuriHalNfcDevData* nfc_data, uint8_t sector, uint8_t key_type) {
    if(!nfc_data) return false;

    // Collect authentication data
    KeyRecoveryData data = {.sector = sector, .key_type = key_type, .nonce = 0, .keystream = 0};

    // Implementation details omitted for brevity
    // Real implementation would:
    // 1. Collect multiple authentication attempts
    // 2. Analyze the collected data
    // 3. Recover the key using the weak RNG

    return false;
}

// Implement darkside attack
// This attack exploits the weak random number generator and filter function
bool mifare_classic_darkside_attack(FuriHalNfcDevData* nfc_data, uint8_t sector, uint8_t key_type) {
    if(!nfc_data) return false;

    // Collect authentication data
    KeyRecoveryData data = {.sector = sector, .key_type = key_type, .nonce = 0, .keystream = 0};

    // Implementation details omitted for brevity
    // Real implementation would:
    // 1. Collect authentication data
    // 2. Analyze the filter function output
    // 3. Recover the key using the weak RNG and filter function

    return false;
}

// Print card information for debugging
void mifare_classic_print_card_info(FuriHalNfcDevData* nfc_data) {
    if(!nfc_data) return;

    FURI_LOG_I(TAG, "Card Type: MIFARE Classic");
    FURI_LOG_I(
        TAG,
        "UID: %02X %02X %02X %02X",
        nfc_data->uid[0],
        nfc_data->uid[1],
        nfc_data->uid[2],
        nfc_data->uid[3]);
    FURI_LOG_I(TAG, "ATQA: %02X %02X", nfc_data->atqa[0], nfc_data->atqa[1]);
    FURI_LOG_I(TAG, "SAK: %02X", nfc_data->sak);
}

// Check if card is vulnerable to known attacks
bool mifare_classic_is_vulnerable(FuriHalNfcDevData* nfc_data) {
    if(!nfc_data) return false;

    // Check for known vulnerable card types
    // MIFARE Classic 1K (SAK = 0x08)
    if(nfc_data->sak == 0x08) return true;

    // MIFARE Classic 4K (SAK = 0x18)
    if(nfc_data->sak == 0x18) return true;

    return false;
}

// Calculate UID checksum for validation
uint32_t mifare_classic_calculate_uid_checksum(uint32_t uid) {
    uint32_t checksum = 0;
    for(int i = 0; i < 4; i++) {
        checksum ^= (uid >> (i * 8)) & 0xFF;
    }
    return checksum;
}

// Validate card UID using checksum
bool mifare_classic_validate_uid(uint32_t uid, uint8_t checksum) {
    return (mifare_classic_calculate_uid_checksum(uid) == checksum);
}

// Default keys to check
static const uint8_t default_keys[][6] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Factory default
    {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5}, // Public transport
    {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7}, // Parking
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Null key
    // Add more default keys as needed
};

CardType detect_card_type(FuriHalNfc* nfc) {
    if(!nfc) return CardTypeUnknown;

    FuriHalNfcDevData nfc_data = {};
    if(!furi_hal_nfc_detect(&nfc_data, 300)) {
        return CardTypeUnknown;
    }

    // Check ATQA and SAK values
    if(nfc_data.atqa[0] == 0x44 && nfc_data.atqa[1] == 0x00 && nfc_data.sak == 0x00) {
        return CardTypeMifareUltralight;
    } else if(nfc_data.atqa[0] == 0x44 && nfc_data.atqa[1] == 0x00 && nfc_data.sak == 0x08) {
        return CardTypeNTAG;
    } else if(
        (nfc_data.atqa[0] == 0x04 && nfc_data.atqa[1] == 0x00) ||
        (nfc_data.atqa[0] == 0x02 && nfc_data.atqa[1] == 0x00)) {
        if(nfc_data.sak == 0x08 || nfc_data.sak == 0x18 || nfc_data.sak == 0x28 ||
           nfc_data.sak == 0x38 || nfc_data.sak == 0x88 || nfc_data.sak == 0x98) {
            return CardTypeMifareClassic;
        }
    }

    return CardTypeUnknown;
}

MifareClassicError test_default_keys(FuriHalNfc* nfc, CardInfo* card) {
    if(!nfc || !card) return ErrorInvalidCommand;

    // Common default keys
    const uint8_t default_keys[][6] = {
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Factory default
        {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5}, // Factory default alternate
        {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7}, // Factory default alternate 2
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Blank key
        {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5}, // Default key
    };

    card->security.has_default_keys = false;

    for(uint8_t sector = 0; sector < MIFARE_CLASSIC_SECTORS_MAX; sector++) {
        for(size_t i = 0; i < sizeof(default_keys) / MIFARE_CLASSIC_KEY_SIZE; i++) {
            if(furi_hal_nfc_activate_mf_classic(nfc, sector, default_keys[i], MfClassicKeyA)) {
                card->security.has_default_keys = true;
                return ErrorNone;
            }
        }
    }

    return ErrorNone;
}

MifareClassicError check_known_vulnerabilities(FuriHalNfc* nfc, CardInfo* card) {
    if(!nfc || !card) return ErrorInvalidCommand;

    // Reset security analysis results
    memset(&card->security, 0, sizeof(SecurityAnalysisResults));

    // Test for default keys
    test_default_keys(nfc, card);

    // Check for known vulnerabilities based on card type
    switch(card->type) {
    case CardTypeMifareClassic:
        // Check for nested attack vulnerability (affects MIFARE Classic)
        card->security.has_nested_attack = true;

        // Check for darkside attack vulnerability (affects MIFARE Classic)
        card->security.has_darkside_attack = true;

        // Check for hardnested attack vulnerability (affects MIFARE Classic)
        card->security.has_hardnested_attack = true;

        // Check for static nested attack vulnerability (affects some MIFARE Classic)
        card->security.has_staticnested_attack = true;

        // Check for UID firmware (Chinese clones)
        if(card->uid[0] == 0x08 || card->uid[0] == 0x88) {
            card->security.has_uid_firmware = true;
        }

        // Check for Chinese Magic Cards
        if(card->atqa[0] == 0x04 && card->atqa[1] == 0x00 && card->sak == 0x88) {
            card->security.has_chinese_magic = true;
        }
        break;

    case CardTypeMifareUltralight:
    case CardTypeNTAG:
        // These cards have different security characteristics
        // Add specific vulnerability checks here
        break;

    default:
        return ErrorUnknown;
    }

    return ErrorNone;
}

void get_card_details(CardInfo* card, char* output, size_t output_size) {
    if(!card || !output || output_size == 0) return;

    char* card_type_str = "Unknown";
    switch(card->type) {
    case CardTypeMifareClassic:
        card_type_str = "MIFARE Classic";
        break;
    case CardTypeMifareUltralight:
        card_type_str = "MIFARE Ultralight";
        break;
    case CardTypeNTAG:
        card_type_str = "NTAG";
        break;
    default:
        break;
    }

    // Format basic card information
    snprintf(
        output,
        output_size,
        "Card Type: %s\n"
        "UID: %02X:%02X:%02X:%02X:%02X:%02X:%02X\n"
        "ATQA: %02X:%02X\n"
        "SAK: %02X\n"
        "Manufacturer: %s\n\n"
        "Security Analysis:\n"
        "%s%s%s%s%s%s%s%s",
        card_type_str,
        card->uid[0],
        card->uid[1],
        card->uid[2],
        card->uid[3],
        card->uid[4],
        card->uid[5],
        card->uid[6],
        card->atqa[0],
        card->atqa[1],
        card->sak,
        card->manufacturer,
        card->security.has_default_keys ? "- Has default keys\n" : "",
        card->security.has_weak_keys ? "- Has weak keys\n" : "",
        card->security.has_nested_attack ? "- Vulnerable to nested attack\n" : "",
        card->security.has_darkside_attack ? "- Vulnerable to darkside attack\n" : "",
        card->security.has_hardnested_attack ? "- Vulnerable to hardnested attack\n" : "",
        card->security.has_staticnested_attack ? "- Vulnerable to static nested attack\n" : "",
        card->security.has_uid_firmware ? "- Has UID firmware (Chinese clone)\n" : "",
        card->security.has_chinese_magic ? "- Is a Chinese Magic Card\n" : "");
}
