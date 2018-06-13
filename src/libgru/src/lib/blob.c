#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "blob.h"
#include "util.h"

void gru_blob_init(struct gru_blob *blob)
{
  blob->data = NULL;
  blob->size = 0;
}

void gru_blob_destroy(struct gru_blob *blob)
{
  if (blob->data)
    free(blob->data);
}

enum gru_error gru_blob_load(struct gru_blob *blob, const char *filename)
{
  FILE *file = fopen(filename, "rb");
  if (!file)
    return GRU_ERROR_FILEIN;
  if (fseek(file, 0, SEEK_END)) {
    fclose(file);
    return GRU_ERROR_FILEIN;
  }
  size_t size = ftell(file);
  if (fseek(file, 0, SEEK_SET)) {
    fclose(file);
    return GRU_ERROR_FILEIN;
  }
  void *data = malloc(size);
  if (size > 0 && !data) {
    fclose(file);
    return GRU_ERROR_MEMORY;
  }
  if (fread(data, 1, size, file) != size) {
    fclose(file);
    free(data);
    return GRU_ERROR_FILEIN;
  }
  fclose(file);
  if (blob->data)
    free(blob->data);
  blob->data = data;
  blob->size = size;
  return GRU_SUCCESS;
}

enum gru_error gru_blob_save(struct gru_blob *blob, const char *filename)
{
  FILE *file = fopen(filename, "wb");
  if (!file)
    return GRU_ERROR_FILEOUT;
  if (fwrite(blob->data, 1, blob->size, file) != blob->size) {
    fclose(file);
    return GRU_ERROR_FILEOUT;
  }
  fclose(file);
  return GRU_SUCCESS;
}

size_t gru_blob_size(struct gru_blob *blob)
{
  return blob->size;
}

enum gru_error gru_blob_ensure_size(struct gru_blob *blob, size_t size)
{
  if (blob->size < size) {
    void *new_data = realloc(blob->data, size);
    if (size > 0 && !new_data)
      return GRU_ERROR_MEMORY;
    blob->data = new_data;
    memset(blob_at(blob, blob->size), 0, size - blob->size);
    blob->size = size;
  }
  return GRU_SUCCESS;
}


enum gru_error gru_blob_resize(struct gru_blob *blob, size_t size)
{
  if (size != blob->size) {
    void *new_data = realloc(blob->data, size);
    if (size > 0 && !new_data)
      return GRU_ERROR_MEMORY;
    blob->data = new_data;
    if (blob->size < size)
      memset(blob_at(blob, blob->size), 0, size - blob->size);
    blob->size = size;
  }
  return GRU_SUCCESS;
}

enum gru_error gru_blob_crc32(struct gru_blob *blob, size_t start, size_t size,
                              uint32_t *crc)
{
  if (start > blob->size || start + size > blob->size)
    return GRU_ERROR_RANGE;
  *crc = gru_util_crc32(blob_at(blob, start), size);
  return GRU_SUCCESS;
}

enum gru_error gru_blob_write(struct gru_blob *blob, size_t address,
                              struct gru_blob *src_blob, size_t src_address,
                              size_t size)
{
  if (src_address > src_blob->size || src_address + size > src_blob->size)
    return GRU_ERROR_RANGE;
  if (gru_blob_ensure_size(blob, address + size))
    return GRU_ERROR_MEMORY;
  memmove(blob_at(blob, address), blob_at(src_blob, src_address), size);
  return GRU_SUCCESS;
}

enum gru_error gru_blob_copy(struct gru_blob *blob, size_t start, size_t size,
                             struct gru_blob *dst_blob)
{
  if (start > blob->size || start + size > blob->size)
    return GRU_ERROR_RANGE;
  dst_blob->data = malloc(size);
  if (size > 0 && !dst_blob->data)
    return GRU_ERROR_MEMORY;
  dst_blob->size = size;
  memcpy(dst_blob->data, blob_at(blob, start), size);
  return GRU_SUCCESS;
}

enum gru_error gru_blob_write8(struct gru_blob *blob,
                               size_t address, uint8_t value)
{
  if (gru_blob_ensure_size(blob, address + sizeof(uint8_t)))
    return GRU_ERROR_MEMORY;
  *(uint8_t*)blob_at(blob, address) = value;
  return GRU_SUCCESS;
}

enum gru_error gru_blob_read8(struct gru_blob *blob,
                              size_t address, uint8_t *value)
{
  if (address + sizeof(uint8_t) > blob->size)
    return GRU_ERROR_RANGE;
  *value = *(uint8_t*)blob_at(blob, address);
  return GRU_SUCCESS;
}

enum gru_error gru_blob_write16be(struct gru_blob *blob,
                                  size_t address, uint16_t value)
{
  if (gru_blob_ensure_size(blob, address + sizeof(uint16_t)))
    return GRU_ERROR_MEMORY;
  *(uint16_t*)blob_at(blob, address) = gru_util_htobe16(value);
  return GRU_SUCCESS;
}

enum gru_error gru_blob_write16le(struct gru_blob *blob,
                                  size_t address, uint16_t value)
{
  if (gru_blob_ensure_size(blob, address + sizeof(uint16_t)))
    return GRU_ERROR_MEMORY;
  *(uint16_t*)blob_at(blob, address) = gru_util_htole16(value);
  return GRU_SUCCESS;
}

enum gru_error gru_blob_read16be(struct gru_blob *blob,
                                 size_t address, uint16_t *value)
{
  if (address + sizeof(uint16_t) > blob->size)
    return GRU_ERROR_RANGE;
  *value = gru_util_betoh16(*(uint16_t*)blob_at(blob, address));
  return GRU_SUCCESS;
}

enum gru_error gru_blob_read16le(struct gru_blob *blob,
                                 size_t address, uint16_t *value)
{
  if (address + sizeof(uint16_t) > blob->size)
    return GRU_ERROR_RANGE;
  *value = gru_util_letoh16(*(uint16_t*)blob_at(blob, address));
  return GRU_SUCCESS;
}

enum gru_error gru_blob_write32be(struct gru_blob *blob,
                                  size_t address, uint32_t value)
{
  if (gru_blob_ensure_size(blob, address + sizeof(uint32_t)))
    return GRU_ERROR_MEMORY;
  *(uint32_t*)blob_at(blob, address) = gru_util_htobe32(value);
  return GRU_SUCCESS;
}

enum gru_error gru_blob_write32le(struct gru_blob *blob,
                                  size_t address, uint32_t value)
{
  if (gru_blob_ensure_size(blob, address + sizeof(uint32_t)))
    return GRU_ERROR_MEMORY;
  *(uint32_t*)blob_at(blob, address) = gru_util_htole32(value);
  return GRU_SUCCESS;
}

enum gru_error gru_blob_read32be(struct gru_blob *blob,
                                 size_t address, uint32_t *value)
{
  if (address + sizeof(uint32_t) > blob->size)
    return GRU_ERROR_RANGE;
  *value = gru_util_betoh32(*(uint32_t*)blob_at(blob, address));
  return GRU_SUCCESS;
}

enum gru_error gru_blob_read32le(struct gru_blob *blob,
                                 size_t address, uint32_t *value)
{
  if (address + sizeof(uint32_t) > blob->size)
    return GRU_ERROR_RANGE;
  *value = gru_util_letoh32(*(uint32_t*)blob_at(blob, address));
  return GRU_SUCCESS;
}

enum gru_error gru_blob_writefloatbe(struct gru_blob *blob,
                                     size_t address, float value)
{
  if (gru_blob_ensure_size(blob, address + sizeof(uint32_t)))
    return GRU_ERROR_RANGE;
  union
  {
    float    f;
    uint32_t u32;
  } v = {value};
  *(uint32_t*)blob_at(blob, address) = gru_util_htobe32(v.u32);
  return GRU_SUCCESS;
}

enum gru_error gru_blob_writefloatle(struct gru_blob *blob,
                                     size_t address, float value)
{
  if (gru_blob_ensure_size(blob, address + sizeof(uint32_t)))
    return GRU_ERROR_RANGE;
  union
  {
    float    f;
    uint32_t u32;
  } v = {value};
  *(uint32_t*)blob_at(blob, address) = gru_util_htole32(v.u32);
  return GRU_SUCCESS;
}

enum gru_error gru_blob_readfloatbe(struct gru_blob *blob,
                                    size_t address, float *value)
{
  if (address + sizeof(uint32_t) > blob->size)
    return GRU_ERROR_RANGE;
  union
  {
    uint32_t u32;
    float    f;
  } v = {gru_util_betoh32(*(uint32_t*)blob_at(blob, address))};
  *value = v.f;
  return GRU_SUCCESS;
}

enum gru_error gru_blob_readfloatle(struct gru_blob *blob,
                                    size_t address, float *value)
{
  if (address + sizeof(uint32_t) > blob->size)
    return GRU_ERROR_RANGE;
  union
  {
    uint32_t u32;
    float    f;
  } v = {gru_util_letoh32(*(uint32_t*)blob_at(blob, address))};
  *value = v.f;
  return GRU_SUCCESS;
}

enum gru_error gru_blob_writestring(struct gru_blob *blob,
                                    size_t address, size_t size,
                                    const char *value)
{
  if (gru_blob_ensure_size(blob, address + size))
    return GRU_ERROR_MEMORY;
  memcpy(blob_at(blob, address), value, size);
  return GRU_SUCCESS;
}

enum gru_error gru_blob_readstring(struct gru_blob *blob,
                                   size_t address, size_t size,
                                   char **value)
{
  if (address > blob->size || address + size > blob->size)
    return GRU_ERROR_RANGE;
  *value = malloc(size);
  if (size > 0 && !*value)
    return GRU_ERROR_MEMORY;
  memcpy(*value, blob_at(blob, address), size);
  return GRU_SUCCESS;
}

void gru_blob_swap(struct gru_blob *blob, size_t word_size)
{
  char *data = blob->data;
  for (size_t i = 0; i + word_size <= blob->size; i += word_size)
    for (size_t j = 0; j < word_size / 2; ++j) {
      char *a = &data[i + j];
      char *b = &data[i + word_size - j - 1];
      char v = *a;
      *a = *b;
      *b = v;
    }
}

size_t gru_blob_find(struct gru_blob *blob,
                     struct gru_blob *search,
                     size_t start, size_t length)
{
  if (blob->size < start || search->size == 0)
    return -1;
  if (length == 0 || start + length > blob->size)
    length = blob->size - start;
  if (length < search->size)
    return -1;
  char *s = blob->data;
  char *e = s + length - search->size + 1;
  char *ss = search->data;
  char *se = ss + search->size;
  for (char *p = s; p != e; ++p) {
    char *a = p;
    char *b = ss;
    while (*a++ == *b++)
      if (b == se)
        return p - s;
  }
  return -1;
}
