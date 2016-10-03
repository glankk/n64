#ifndef GRU_UTIL_H
#define GRU_UTIL_H
#include <stddef.h>
#include <stdint.h>
#include "gru.h"

uint16_t gru_util_bswap16(uint16_t value);
uint32_t gru_util_bswap32(uint32_t value);
uint16_t gru_util_htobe16(uint16_t value);
uint16_t gru_util_htole16(uint16_t value);
uint32_t gru_util_htobe32(uint32_t value);
uint32_t gru_util_htole32(uint32_t value);
#define gru_util_betoh16 gru_util_htobe16
#define gru_util_letoh16 gru_util_htole16
#define gru_util_betoh32 gru_util_htobe32
#define gru_util_letoh32 gru_util_htole32

struct gru_yaz0
{
  char      magic[4];
  uint32_t  size;
  char      padding[8];
  uint8_t   code[];
};

enum gru_error gru_util_yaz0_encode(void **data_ptr, size_t *size_ptr);
enum gru_error gru_util_yaz0_decode(void **data_ptr, size_t *size_ptr);

uint32_t gru_util_crc32(void *data, size_t size);

#endif
