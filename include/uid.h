#ifndef PCROAST_UID_H
#define PCROAST_UID_H

#include <stddef.h>
#include <stdint.h>

/**
 * Read a 64 bit unique address and convert it to a hexadecimal string.
 * @param uid Pointer to the starting address of the 64 bit UID (LSB)
 * @param result Address to store the resulting string in. Must be at least 17 bytes long.
 */
void uid_to_str(uint8_t *uid, char *result);

#endif  // PCROAST_UID_H
