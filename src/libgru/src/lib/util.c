#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/param.h>
#include <vector/vector.h>
#include "gru.h"
#include "util.h"

#if !defined BYTE_ORDER || !defined LITTLE_ENDIAN || !defined BIG_ENDIAN
#error byte order unsupported
#endif

uint16_t gru_util_bswap16(uint16_t value)
{
  return ((value & 0x00FF) << 8) |
         ((value & 0xFF00) >> 8);
}

uint32_t gru_util_bswap32(uint32_t value)
{
  return ((value & 0x000000FF) << 24) |
         ((value & 0x0000FF00) << 8)  |
         ((value & 0x00FF0000) >> 8)  |
         ((value & 0xFF000000) >> 24);
}

uint16_t gru_util_htobe16(uint16_t value)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  return gru_util_bswap16(value);
#else
  return value;
#endif
}

uint16_t gru_util_htole16(uint16_t value)
{
#if BYTE_ORDER == BIG_ENDIAN
  return gru_util_bswap16(value);
#else
  return value;
#endif
}

uint32_t gru_util_htobe32(uint32_t value)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  return gru_util_bswap32(value);
#else
  return value;
#endif
}

uint32_t gru_util_htole32(uint32_t value)
{
#if BYTE_ORDER == BIG_ENDIAN
  return gru_util_bswap32(value);
#else
  return value;
#endif
}

static void yaz0_find_match(uint8_t *data, size_t size, size_t position,
                            size_t *match_position, size_t *match_size)
{
  *match_size = 0;
  for (size_t i = (position > 0x1000 ? position - 0x1000 : 0);
       i < position; ++i)
  {
    size_t j;
    for (j = 0; position + j < size && j < 0x111; ++j)
      if (data[i + j] != data[position + j])
        break;
    if (j > *match_size) {
      *match_position = i;
      *match_size = j;
    }
  }
}

enum gru_error gru_util_yaz0_encode(void **data_ptr, size_t *size_ptr)
{
  size_t src_size = *size_ptr;
  uint8_t *src = *data_ptr;
  struct vector code;
  vector_init(&code, sizeof(uint8_t));
  uint8_t group[1 + 3 * 8];
  size_t group_size = 0;
  uint8_t gh_mask = 0x00;
  size_t lookahead_pos = 0;
  size_t lookahead_size = 0;
  for (size_t i = 0; i < src_size; gh_mask >>= 1) {
    if (gh_mask == 0x00) {
      if (!vector_push_back(&code, group_size, group))
        goto error;
      gh_mask = 0x80;
      group[0] = 0xFF;
      memset(&group[1], 0, 8);
      group_size = 1;
    }
    size_t chunk_pos;
    size_t chunk_size;
    if (lookahead_size > 0) {
      chunk_pos  = lookahead_pos;
      chunk_size = lookahead_size;
      lookahead_size = 0;
    }
    else {
      yaz0_find_match(src, src_size, i, &chunk_pos, &chunk_size);
      if (chunk_size >= 0x03) {
        yaz0_find_match(src, src_size, i + 1, &lookahead_pos, &lookahead_size);
        if (lookahead_size >= chunk_size + 2)
          chunk_size = 1;
        else
          lookahead_size = 0;
      }
    }
    uint16_t chunk_dist = i - chunk_pos - 1;
    if (chunk_size >= 0x12) {
      group[0] &= ~gh_mask;
      group[group_size++] = ((chunk_dist & 0xF00) >> 8);
      group[group_size++] = (chunk_dist & 0x0FF);
      group[group_size++] = chunk_size - 0x12;
      i += chunk_size;
    }
    else if (chunk_size >= 0x03) {
      group[0] &= ~gh_mask;
      group[group_size++] = ((chunk_size - 0x02)  << 4) |
                            ((chunk_dist & 0xF00) >> 8);
      group[group_size++] = (chunk_dist & 0x0FF);
      i += chunk_size;
    }
    else
      group[group_size++] = src[i++];
  }
  if (!vector_push_back(&code, group_size, group))
    goto error;
  size_t output_size = sizeof(struct gru_yaz0) + code.size;
  struct gru_yaz0 *output = malloc(output_size);
  if (output_size > 0 && !output)
    goto error;
  memcpy(&output->magic, "Yaz0", sizeof(output->magic));
  output->size = gru_util_htobe32(src_size);
  memset(&output->padding, 0, sizeof(output->padding));
  memcpy(&output->code, code.begin, code.size);
  *data_ptr = output;
  *size_ptr = output_size;
  vector_destroy(&code);
  return GRU_SUCCESS;
error:
  vector_destroy(&code);
  return GRU_ERROR_MEMORY;
}

enum gru_error gru_util_yaz0_decode(void **data_ptr, size_t *size_ptr)
{
  size_t src_size = *size_ptr;
  struct gru_yaz0 *src = *data_ptr;
  size_t code_size = src_size - sizeof(*src);
  if (src_size < sizeof(*src) ||
      memcmp(&src->magic, "Yaz0", sizeof(src->magic)) != 0)
    return GRU_ERROR_DATA;
  size_t output_size = gru_util_betoh32(src->size);
  uint8_t *output = calloc(output_size, 1);
  if (output_size > 0 && !output)
    return GRU_ERROR_MEMORY;
  for (size_t i = 0, j = 0; i < code_size && j < output_size;) {
    uint8_t gh = src->code[i++];
    for (size_t k = 0; k < 8 && j < output_size; ++k, gh <<= 1) {
      if (gh & 0x80)
        output[j++] = (i < code_size ? src->code[i++] : 0x00);
      else {
        size_t chunk_size = 0;
        size_t chunk_dist = 0;
        if (src->code[i] & 0xF0) {
          if (i > code_size - 2)
            goto error;
          chunk_size = ((src->code[i] & 0xF0) >> 4) + 0x02;
          chunk_dist = ((src->code[i++] & 0x0F) << 8);
          chunk_dist |= src->code[i++];
        }
        else {
          if (i > code_size - 3)
            goto error;
          chunk_dist = ((src->code[i++] & 0x0F) << 8);
          chunk_dist |= src->code[i++];
          chunk_size = src->code[i++] + 0x12;
        }
        if (++chunk_dist > j)
          goto error;
        for (size_t l = 0; l < chunk_size && j < output_size; ++l, ++j)
          output[j] = output[j - chunk_dist];
      }
    }
  }
  *data_ptr = output;
  *size_ptr = output_size;
  return GRU_SUCCESS;
error:
  free(output);
  return GRU_ERROR_DATA;
}

static uint32_t *crc32_table()
{
  static uint32_t crc_table[256];
  static gru_bool_t generate_table = GRU_TRUE;
  if (generate_table) {
    const uint32_t p = 0xEDB88320;
    for (int i = 0; i < 256; ++i) {
      uint32_t crc = i;
      for (int j = 0; j < 8; ++j)
        crc = (crc >> 1) ^ ((crc & 1) ? p : 0);
      crc_table[i] = crc;
    }
    generate_table = GRU_FALSE;
  }
  return crc_table;
}

uint32_t gru_util_crc32(void *data, size_t size)
{
  uint32_t *crc_table = crc32_table();
  uint8_t *data_u8 = data;
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < size; ++i)
    crc = (crc >> 8) ^ crc_table[(crc ^ data_u8[i]) & 0xFF];
  return ~crc;
}
