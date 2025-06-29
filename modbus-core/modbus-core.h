#ifndef MODBUS_CORE_H
#define MODBUS_CORE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Protocol limits */
#define MODBUS_MAX_PDU_LENGTH         253
#define MODBUS_MAX_ADU_LENGTH         256

/* Modbus function codes */
#define MODBUS_FC_READ_COILS                0x01
#define MODBUS_FC_READ_DISCRETE_INPUTS      0x02
#define MODBUS_FC_READ_HOLDING_REGISTERS    0x03
#define MODBUS_FC_READ_INPUT_REGISTERS      0x04
#define MODBUS_FC_WRITE_SINGLE_COIL         0x05
#define MODBUS_FC_WRITE_SINGLE_REGISTER     0x06
#define MODBUS_FC_READ_EXCEPTION_STATUS     0x07
#define MODBUS_FC_WRITE_MULTIPLE_COILS      0x0F
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  0x10
#define MODBUS_FC_REPORT_SLAVE_ID           0x11
#define MODBUS_FC_MASK_WRITE_REGISTER       0x16
#define MODBUS_FC_WRITE_AND_READ_REGISTERS  0x17

/* Modbus exceptions */
#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION           0x01
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS       0x02
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE         0x03
#define MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE    0x04
#define MODBUS_EXCEPTION_ACKNOWLEDGE                0x05
#define MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY       0x06
#define MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE       0x07
#define MODBUS_EXCEPTION_MEMORY_PARITY              0x08
#define MODBUS_EXCEPTION_GATEWAY_PATH               0x0A
#define MODBUS_EXCEPTION_GATEWAY_TARGET             0x0B

/* Core context structure */
typedef struct {
    uint8_t *request_buffer;  /* Request buffer */
    uint8_t *response_buffer; /* Response buffer */
    int slave;                /* Slave ID */
} modbus_t;

/* Core protocol API */
int modbus_parse_request(modbus_t *ctx, const uint8_t *buffer, int length);
int modbus_build_response(modbus_t *ctx, uint8_t *buffer, int max_length);

/* Data processing functions */
void modbus_set_bits_from_byte(uint8_t *dest, int idx, const uint8_t value);
void modbus_set_bits_from_bytes(uint8_t *dest, int idx, unsigned int nb_bits, const uint8_t *tab_byte);
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int idx, unsigned int nb_bits);
float modbus_get_float(const uint16_t *src);
float modbus_get_float_abcd(const uint16_t *src);
float modbus_get_float_dcba(const uint16_t *src);
float modbus_get_float_badc(const uint16_t *src);
float modbus_get_float_cdab(const uint16_t *src);
void modbus_set_float(float f, uint16_t *dest);
void modbus_set_float_abcd(float f, uint16_t *dest);
void modbus_set_float_dcba(float f, uint16_t *dest);
void modbus_set_float_badc(float f, uint16_t *dest);
void modbus_set_float_cdab(float f, uint16_t *dest);

#ifdef __cplusplus
}
#endif

#endif /* MODBUS_CORE_H */