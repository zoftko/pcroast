#include "uid.h"

static const char nibble_hex_lut[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void uid_to_str(uint8_t *uid, char *result) {
    result[16] = '\0';
    uint8_t nibble;
    for (int idx = 14; idx > -1; idx -= 2) {
        nibble = ((*uid) >> 4) & 0xF;
        result[idx] = nibble_hex_lut[nibble];

        nibble = (*uid) & 0xF;
        result[idx + 1] = nibble_hex_lut[nibble];

        uid++;
    }
}
