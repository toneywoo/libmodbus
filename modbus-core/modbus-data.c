/*
 * Modbus Protocol Data Processing Implementation
 */

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "modbus-core.h"

/* Sets many bits from a single byte value (all 8 bits of the byte value are set) */
void modbus_set_bits_from_byte(uint8_t *dest, int idx, const uint8_t value)
{
    int i;

    for (i = 0; i < 8; i++) {
        dest[idx + i] = (value & (1 << i)) ? 1 : 0;
    }
}

/* Sets many bits from a table of bytes (only the bits between idx and idx + nb_bits are set) */
void modbus_set_bits_from_bytes(uint8_t *dest, int idx, unsigned int nb_bits, const uint8_t *tab_byte)
{
    unsigned int i;
    int shift = 0;

    for (i = idx; i < idx + nb_bits; i++) {
        dest[i] = tab_byte[(i - idx) / 8] & (1 << shift) ? 1 : 0;
        shift++;
        shift %= 8;
    }
}

/* Gets the byte value from many bits.
   To obtain a full byte, set nb_bits to 8. */
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int idx, unsigned int nb_bits)
{
    unsigned int i;
    uint8_t value = 0;

    if (nb_bits > 8) {
        assert(nb_bits < 8);
        nb_bits = 8;
    }

    for (i = 0; i < nb_bits; i++) {
        value |= (src[idx + i] << i);
    }

    return value;
}

/* Get a float from 4 bytes (Modbus) without any conversion (ABCD) */
float modbus_get_float_abcd(const uint16_t *src)
{
    float f;
    uint32_t i = (((uint32_t)src[0]) << 16) + src[1];
    memcpy(&f, &i, sizeof(float));
    return f;
}

/* Get a float from 4 bytes (Modbus) in inversed format (DCBA) */
float modbus_get_float_dcba(const uint16_t *src)
{
    uint16_t regs[2];
    regs[1] = src[0];
    regs[0] = src[1];
    return modbus_get_float_abcd(regs);
}

/* Get a float from 4 bytes (Modbus) with swapped bytes (BADC) */
float modbus_get_float_badc(const uint16_t *src)
{
    uint16_t regs[2];
    uint8_t *dst = (uint8_t *)regs;
    uint8_t *src_bytes = (uint8_t *)src;

    dst[0] = src_bytes[1];
    dst[1] = src_bytes[0];
    dst[2] = src_bytes[3];
    dst[3] = src_bytes[2];

    return modbus_get_float_abcd(regs);
}

/* Get a float from 4 bytes (Modbus) with swapped words (CDAB) */
float modbus_get_float_cdab(const uint16_t *src)
{
    uint16_t regs[2];
    uint8_t *dst = (uint8_t *)regs;
    uint8_t *src_bytes = (uint8_t *)src;

    dst[0] = src_bytes[2];
    dst[1] = src_bytes[3];
    dst[2] = src_bytes[0];
    dst[3] = src_bytes[1];

    return modbus_get_float_abcd(regs);
}

/* DEPRECATED - Get a float from 4 bytes in sort of Modbus format */
float modbus_get_float(const uint16_t *src)
{
    return modbus_get_float_cdab(src);
}

/* Set a float to 4 bytes for Modbus w/o any conversion (ABCD) */
void modbus_set_float_abcd(float f, uint16_t *dest)
{
    uint32_t i;
    memcpy(&i, &f, sizeof(uint32_t));
    dest[0] = (uint16_t)(i >> 16);
    dest[1] = (uint16_t)i;
}

/* Set a float to 4 bytes for Modbus with byte and word swap conversion (DCBA) */
void modbus_set_float_dcba(float f, uint16_t *dest)
{
    uint16_t regs[2];
    modbus_set_float_abcd(f, regs);
    dest[0] = regs[1];
    dest[1] = regs[0];
}

/* Set a float to 4 bytes for Modbus with byte swap conversion (BADC) */
void modbus_set_float_badc(float f, uint16_t *dest)
{
    uint16_t regs[2];
    uint8_t *src_bytes = (uint8_t *)regs;
    uint8_t *dst_bytes = (uint8_t *)dest;

    modbus_set_float_abcd(f, regs);

    dst_bytes[0] = src_bytes[1];
    dst_bytes[1] = src_bytes[0];
    dst_bytes[2] = src_bytes[3];
    dst_bytes[3] = src_bytes[2];
}

/* Set a float to 4 bytes for Modbus with word swap conversion (CDAB) */
void modbus_set_float_cdab(float f, uint16_t *dest)
{
    uint16_t regs[2];
    uint8_t *src_bytes = (uint8_t *)regs;
    uint8_t *dst_bytes = (uint8_t *)dest;

    modbus_set_float_abcd(f, regs);

    dst_bytes[0] = src_bytes[2];
    dst_bytes[1] = src_bytes[3];
    dst_bytes[2] = src_bytes[0];
    dst_bytes[3] = src_bytes[1];
}

/* DEPRECATED - Set a float to 4 bytes in a sort of Modbus format! */
void modbus_set_float(float f, uint16_t *dest)
{
    modbus_set_float_cdab(f, dest);
}