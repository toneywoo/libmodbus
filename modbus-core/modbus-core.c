/*
 * Modbus Protocol Core Implementation
 */

#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "modbus-core.h"

/* Internal use */
#define MSG_LENGTH_UNDEFINED -1

/* Computes the length of the expected response including checksum */
static unsigned int compute_response_length_from_request(modbus_t *ctx, uint8_t *req)
{
    int length;
    const int offset = 0; /* No header in core implementation */

    switch (req[offset]) {
    case MODBUS_FC_READ_COILS:
    case MODBUS_FC_READ_DISCRETE_INPUTS: {
        int nb = (req[offset + 3] << 8) | req[offset + 4];
        length = 2 + (nb / 8) + ((nb % 8) ? 1 : 0);
    } break;
    case MODBUS_FC_WRITE_AND_READ_REGISTERS:
    case MODBUS_FC_READ_HOLDING_REGISTERS:
    case MODBUS_FC_READ_INPUT_REGISTERS:
        length = 2 + 2 * (req[offset + 3] << 8 | req[offset + 4]);
        break;
    case MODBUS_FC_READ_EXCEPTION_STATUS:
        length = 3;
        break;
    case MODBUS_FC_REPORT_SLAVE_ID:
        return MSG_LENGTH_UNDEFINED;
    case MODBUS_FC_MASK_WRITE_REGISTER:
        length = 7;
        break;
    default:
        length = 5;
    }

    return offset + length;
}

/* Core protocol API implementation */
int modbus_parse_request(modbus_t *ctx, const uint8_t *buffer, int length)
{
    /* Basic validation */
    if (!ctx || !buffer || length < 4) {
        errno = EINVAL;
        return -1;
    }

    /* Store request in context */
    if (ctx->request_buffer) {
        free(ctx->request_buffer);
    }
    ctx->request_buffer = malloc(length);
    if (!ctx->request_buffer) {
        errno = ENOMEM;
        return -1;
    }
    memcpy(ctx->request_buffer, buffer, length);

    return 0;
}

int modbus_build_response(modbus_t *ctx, uint8_t *buffer, int max_length)
{
    /* Basic validation */
    if (!ctx || !buffer || max_length < 4 || !ctx->request_buffer) {
        errno = EINVAL;
        return -1;
    }

    /* For simplicity, just echo the request as response */
    int req_len = strlen((char*)ctx->request_buffer);
    if (req_len > max_length) {
        errno = EMSGSIZE;
        return -1;
    }
    
    memcpy(buffer, ctx->request_buffer, req_len);
    return req_len;
}

/* Data processing functions */
void modbus_set_bits_from_byte(uint8_t *dest, int idx, const uint8_t value)
{
    if (dest && idx >= 0) {
        dest[idx] = value ? 1 : 0;
    }
}

void modbus_set_bits_from_bytes(uint8_t *dest, int idx, unsigned int nb_bits, 
                               const uint8_t *tab_byte)
{
    unsigned int i;
    int bit;

    for (i = 0; i < nb_bits; i++) {
        bit = 0x01 << (i % 8);
        dest[idx + i] = (tab_byte[i / 8] & bit) ? 1 : 0;
    }
}

uint8_t modbus_get_byte_from_bits(const uint8_t *src, int idx, unsigned int nb_bits)
{
    unsigned int i;
    int bit;
    uint8_t value = 0;

    for (i = 0; i < nb_bits && i < 8; i++) {
        bit = 0x01 << (i % 8);
        if (src[idx + i]) {
            value |= bit;
        }
    }

    return value;
}

float modbus_get_float(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = (((uint32_t)src[0]) << 16) + src[1];
    memcpy(&f, &i, sizeof(float));

    return f;
}

float modbus_get_float_abcd(const uint16_t *src)
{
    return modbus_get_float(src);
}

float modbus_get_float_dcba(const uint16_t *src)
{
    uint16_t regs[2];
    regs[1] = src[0];
    regs[0] = src[1];
    return modbus_get_float(regs);
}

float modbus_get_float_badc(const uint16_t *src)
{
    uint16_t regs[2];
    uint8_t *dst = (uint8_t *)regs;
    uint8_t *src_bytes = (uint8_t *)src;

    dst[0] = src_bytes[1];
    dst[1] = src_bytes[0];
    dst[2] = src_bytes[3];
    dst[3] = src_bytes[2];

    return modbus_get_float(regs);
}

float modbus_get_float_cdab(const uint16_t *src)
{
    uint16_t regs[2];
    uint8_t *dst = (uint8_t *)regs;
    uint8_t *src_bytes = (uint8_t *)src;

    dst[0] = src_bytes[2];
    dst[1] = src_bytes[3];
    dst[2] = src_bytes[0];
    dst[3] = src_bytes[1];

    return modbus_get_float(regs);
}

void modbus_set_float(float f, uint16_t *dest)
{
    uint32_t i;
    memcpy(&i, &f, sizeof(uint32_t));
    dest[0] = (uint16_t)(i >> 16);
    dest[1] = (uint16_t)i;
}

void modbus_set_float_abcd(float f, uint16_t *dest)
{
    modbus_set_float(f, dest);
}

void modbus_set_float_dcba(float f, uint16_t *dest)
{
    uint16_t regs[2];
    modbus_set_float(f, regs);
    dest[0] = regs[1];
    dest[1] = regs[0];
}

void modbus_set_float_badc(float f, uint16_t *dest)
{
    uint16_t regs[2];
    uint8_t *src_bytes = (uint8_t *)regs;
    uint8_t *dst_bytes = (uint8_t *)dest;

    modbus_set_float(f, regs);

    dst_bytes[0] = src_bytes[1];
    dst_bytes[1] = src_bytes[0];
    dst_bytes[2] = src_bytes[3];
    dst_bytes[3] = src_bytes[2];
}

void modbus_set_float_cdab(float f, uint16_t *dest)
{
    uint16_t regs[2];
    uint8_t *src_bytes = (uint8_t *)regs;
    uint8_t *dst_bytes = (uint8_t *)dest;

    modbus_set_float(f, regs);

    dst_bytes[0] = src_bytes[2];
    dst_bytes[1] = src_bytes[3];
    dst_bytes[2] = src_bytes[0];
    dst_bytes[3] = src_bytes[1];
}