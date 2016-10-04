#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include "gru.h"
#include "ups.h"
#include "blob.h"
#include "util.h"

struct ups_chunk
{
  size_t  copy_length;
  size_t  xor_length;
  void   *xor_data;
};

static enum gru_error read_varint(struct gru_blob *blob, size_t *p_ptr,
                                  size_t *result_ptr)
{
  uint8_t *data = blob->data;
  size_t result = 0;
  size_t shift = 0;
  for (size_t p = *p_ptr; p < blob->size; ++p) {
    uint8_t b = data[p];
    if (b & 0x80) {
      if (shift + 7 > CHAR_BIT * sizeof(size_t))
        return GRU_ERROR_RANGE;
      *p_ptr = p + 1;
      result += (b & 0x7F) << shift;
      *result_ptr = result;
      return GRU_SUCCESS;
    }
    result += (b | 0x80) << shift;
    shift += 7;
  }
  return GRU_ERROR_DATA;
}

static enum gru_error write_varint(struct vector *data, size_t value)
{
  while (1) {
    uint8_t d = value & 0x7F;
    value >>= 7;
    if (value == 0) {
      d |= 0x80;
      if (!vector_push_back(data, 1, &d))
        return GRU_ERROR_MEMORY;
      break;
    }
    if (!vector_push_back(data, 1, &d))
      return GRU_ERROR_MEMORY;
    --value;
  }
  return GRU_SUCCESS;
}

void gru_ups_init(struct gru_ups *ups)
{
  ups->src_size = 0;
  ups->dst_size = 0;
  ups->src_crc = 0;
  ups->dst_crc = 0;
  vector_init(&ups->chunks, sizeof(struct ups_chunk));
}

void gru_ups_destroy(struct gru_ups *ups)
{
  for (size_t i = 0; i < ups->chunks.size; ++i) {
    struct ups_chunk *chunk = vector_at(&ups->chunks, i);
    if (chunk->xor_data)
      free(chunk->xor_data);
  }
  vector_destroy(&ups->chunks);
}

static enum gru_error gru_ups_load_blob(struct gru_ups *ups,
                                        struct gru_blob *blob)
{
  vector_clear(&ups->chunks);
  if (blob->size < 6 || memcmp(blob->data, "UPS1", 4) != 0)
    return GRU_ERROR_DATA;
  size_t ups_p = 4;
  enum gru_error e = read_varint(blob, &ups_p, &ups->src_size);
  if (e)
    return e;
  e = read_varint(blob, &ups_p, &ups->dst_size);
  if (e)
    return e;
  while (ups_p < blob->size - 12) {
    struct ups_chunk chunk;
    e = read_varint(blob, &ups_p, &chunk.copy_length);
    if (e)
      return e;
    size_t xor_start = ups_p;
    chunk.xor_length = 0;
    while (ups_p < blob->size) {
      if (*(uint8_t*)blob_at(blob, ups_p++) == 0)
        break;
      ++chunk.xor_length;
    }
    chunk.xor_data = malloc(chunk.xor_length);
    if (chunk.xor_length > 0 && !chunk.xor_data)
      return GRU_ERROR_MEMORY;
    memcpy(chunk.xor_data, blob_at(blob, xor_start), chunk.xor_length);
    if (!vector_push_back(&ups->chunks, 1, &chunk)) {
      free(chunk.xor_data);
      return GRU_ERROR_MEMORY;
    }
  }
  if (ups_p > blob->size - 12)
    return GRU_ERROR_DATA;
  uint32_t *crc_table = blob_at(blob, ups_p);
  ups->src_crc = gru_util_letoh32(crc_table[0]);
  ups->dst_crc = gru_util_letoh32(crc_table[1]);
  uint32_t ups_scrc = gru_util_letoh32(crc_table[2]);
  uint32_t ups_rcrc;
  gru_blob_crc32(blob, 0, blob->size - 4, &ups_rcrc);
  if (ups_scrc != ups_rcrc)
    return GRU_ERROR_DATA;
  return GRU_SUCCESS;
}

enum gru_error gru_ups_load(struct gru_ups *ups, const char *filename)
{
  struct gru_blob blob;
  gru_blob_init(&blob);
  enum gru_error e = gru_blob_load(&blob, filename);
  if (e)
    return e;
  e = gru_ups_load_blob(ups, &blob);
  gru_blob_destroy(&blob);
  return e;
}

enum gru_error gru_ups_save(struct gru_ups *ups, const char *filename)
{
  struct vector data;
  vector_init(&data, sizeof(uint8_t));
  if (!vector_push_back(&data, 4, "UPS1"))
    return GRU_ERROR_MEMORY;
  if (write_varint(&data, ups->src_size))
    return GRU_ERROR_MEMORY;
  if (write_varint(&data, ups->dst_size))
    return GRU_ERROR_MEMORY;
  for (size_t i = 0; i < ups->chunks.size; ++i) {
    struct ups_chunk *chunk = vector_at(&ups->chunks, i);
    if (write_varint(&data, chunk->copy_length))
      return GRU_ERROR_MEMORY;
    if (!vector_push_back(&data, chunk->xor_length, chunk->xor_data))
      return GRU_ERROR_MEMORY;
    uint8_t null = 0;
    if (!vector_push_back(&data, 1, &null))
      return GRU_ERROR_MEMORY;
  }
  uint32_t crc_table[] =
  {
    gru_util_htole32(ups->src_crc),
    gru_util_htole32(ups->dst_crc),
  };
  if (!vector_push_back(&data, sizeof(crc_table), &crc_table))
    return GRU_ERROR_MEMORY;
  uint32_t ups_crc = gru_util_htole32(gru_util_crc32(data.begin, data.size));
  if (!vector_push_back(&data, sizeof(ups_crc), &ups_crc))
    return GRU_ERROR_MEMORY;
  struct gru_blob blob;
  blob.size = data.size;
  blob.data = vector_release(&data);
  enum gru_error e = gru_blob_save(&blob, filename);
  gru_blob_destroy(&blob);
  return e;
}

enum gru_error gru_ups_make(struct gru_ups *ups, struct gru_blob *src_blob,
                            struct gru_blob *dst_blob)
{
  ups->src_size = src_blob->size;
  ups->dst_size = dst_blob->size;
  gru_blob_crc32(src_blob, 0, src_blob->size, &ups->src_crc);
  gru_blob_crc32(dst_blob, 0, dst_blob->size, &ups->dst_crc);
  vector_clear(&ups->chunks);
  uint8_t *src_data = src_blob->data;
  uint8_t *dst_data = dst_blob->data;
  for (size_t i = 0; i < src_blob->size || i < dst_blob->size;  ++i) {
    struct ups_chunk chunk;
    chunk.copy_length = 0;
    uint8_t sd;
    uint8_t dd;
    while (i < src_blob->size || i < dst_blob->size) {
      sd = (i < src_blob->size ? src_data[i] : 0);
      dd = (i < dst_blob->size ? dst_data[i] : 0);
      if (sd != dd)
        break;
      ++chunk.copy_length;
      ++i;
    }
    size_t xor_start = i;
    chunk.xor_length = 0;
    while (i < src_blob->size || i < dst_blob->size) {
      ++chunk.xor_length;
      ++i;
      sd = (i < src_blob->size ? src_data[i] : 0);
      dd = (i < dst_blob->size ? dst_data[i] : 0);
      if (sd == dd)
        break;
    }
    if (chunk.xor_length == 0)
      break;
    chunk.xor_data = malloc(chunk.xor_length);
    if (chunk.xor_length > 0 && !chunk.xor_data)
      return GRU_ERROR_MEMORY;
    uint8_t *xor_data = chunk.xor_data;
    for (size_t j = 0; j < chunk.xor_length; ++j) {
      uint8_t sd = (xor_start + j < src_blob->size ?
                    src_data[xor_start + j] : 0);
      uint8_t dd = (xor_start + j < dst_blob->size ?
                    dst_data[xor_start + j] : 0);
      xor_data[j] = sd ^ dd;
    }
    if (!vector_push_back(&ups->chunks, 1, &chunk)) {
      free(chunk.xor_data);
      return GRU_ERROR_MEMORY;
    }
  }
  return GRU_SUCCESS;
}

enum gru_error gru_ups_apply(struct gru_ups *ups, struct gru_blob *blob,
                             gru_bool_t ignore_checksums)
{
  if (!ignore_checksums) {
    if (blob->size != ups->src_size)
      return GRU_ERROR_DATA;
    uint32_t src_crc;
    gru_blob_crc32(blob, 0, blob->size, &src_crc);
    if (src_crc != ups->src_crc)
      return GRU_ERROR_DATA;
  }
  if (gru_blob_resize(blob, ups->dst_size))
    return GRU_ERROR_MEMORY;
  size_t dst_p = 0;
  for (size_t i = 0; i < ups->chunks.size && dst_p < blob->size; ++i) {
    struct ups_chunk *chunk = vector_at(&ups->chunks, i);
    dst_p += chunk->copy_length;
    uint8_t *blob_xor = blob_at(blob, dst_p);
    uint8_t *chunk_xor = chunk->xor_data;
    for (size_t j = 0; j < chunk->xor_length && dst_p < blob->size; ++j)
      *blob_xor++ ^= *chunk_xor++;
    dst_p += chunk->xor_length + 1;
  }
  if (!ignore_checksums) {
    uint32_t dst_crc;
    gru_blob_crc32(blob, 0, blob->size, &dst_crc);
    if (dst_crc != ups->dst_crc)
      return GRU_ERROR_DATA;
  }
  return GRU_SUCCESS;
}

enum gru_error gru_ups_undo(struct gru_ups *ups, struct gru_blob *blob,
                            gru_bool_t ignore_checksums)
{
  if (!ignore_checksums) {
    if (blob->size != ups->dst_size)
      return GRU_ERROR_DATA;
    uint32_t dst_crc;
    gru_blob_crc32(blob, 0, blob->size, &dst_crc);
    if (dst_crc != ups->dst_crc)
      return GRU_ERROR_DATA;
  }
  if (gru_blob_resize(blob, ups->src_size))
    return GRU_ERROR_MEMORY;
  size_t src_p = 0;
  for (size_t i = 0; i < ups->chunks.size && src_p < blob->size; ++i) {
    struct ups_chunk *chunk = vector_at(&ups->chunks, i);
    src_p += chunk->copy_length;
    uint8_t *blob_xor = blob_at(blob, src_p);
    uint8_t *chunk_xor = chunk->xor_data;
    for (size_t j = 0; j < chunk->xor_length && src_p < blob->size; ++j)
      *blob_xor++ ^= *chunk_xor++;
    src_p += chunk->xor_length + 1;
  }
  if (!ignore_checksums) {
    uint32_t src_crc;
    gru_blob_crc32(blob, 0, blob->size, &src_crc);
    if (src_crc != ups->src_crc)
      return GRU_ERROR_DATA;
  }
  return GRU_SUCCESS;
}

size_t gru_ups_src_size(struct gru_ups *ups)
{
  return ups->src_size;
}

size_t gru_ups_dst_size(struct gru_ups *ups)
{
  return ups->dst_size;
}

uint32_t gru_ups_src_crc(struct gru_ups *ups)
{
  return ups->src_crc;
}

uint32_t gru_ups_dst_crc(struct gru_ups *ups)
{
  return ups->dst_crc;
}
