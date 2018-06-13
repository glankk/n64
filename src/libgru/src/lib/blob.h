#ifndef GRU_BLOB_H
#define GRU_BLOB_H
#include <stddef.h>
#include <stdint.h>
#include "gru.h"

struct gru_blob
{
  void   *data;
  size_t  size;
};

static inline void *blob_at(struct gru_blob *blob, size_t address)
{
  return (char*)blob->data + address;
}

void gru_blob_init(struct gru_blob *blob);
void gru_blob_destroy(struct gru_blob *blob);
enum gru_error gru_blob_load(struct gru_blob *blob, const char *filename);
enum gru_error gru_blob_save(struct gru_blob *blob, const char *filename);
size_t gru_blob_size(struct gru_blob *blob);
enum gru_error gru_blob_ensure_size(struct gru_blob *blob, size_t size);
enum gru_error gru_blob_resize(struct gru_blob *blob, size_t size);
enum gru_error gru_blob_crc32(struct gru_blob *blob, size_t start, size_t size,
                              uint32_t *crc);
enum gru_error gru_blob_write(struct gru_blob *blob, size_t address,
                              struct gru_blob *src_blob, size_t src_address,
                              size_t size);
enum gru_error gru_blob_copy(struct gru_blob *blob, size_t start, size_t size,
                             struct gru_blob *dst_blob);
enum gru_error gru_blob_write8(struct gru_blob *blob,
                               size_t address, uint8_t value);
enum gru_error gru_blob_read8(struct gru_blob *blob,
                              size_t address, uint8_t *value);
enum gru_error gru_blob_write16be(struct gru_blob *blob,
                                  size_t address, uint16_t value);
enum gru_error gru_blob_write16le(struct gru_blob *blob,
                                  size_t address, uint16_t value);
enum gru_error gru_blob_read16be(struct gru_blob *blob,
                                 size_t address, uint16_t *value);
enum gru_error gru_blob_read16le(struct gru_blob *blob,
                                 size_t address, uint16_t *value);
enum gru_error gru_blob_write32be(struct gru_blob *blob,
                                  size_t address, uint32_t value);
enum gru_error gru_blob_write32le(struct gru_blob *blob,
                                  size_t address, uint32_t value);
enum gru_error gru_blob_read32be(struct gru_blob *blob,
                                 size_t address, uint32_t *value);
enum gru_error gru_blob_read32le(struct gru_blob *blob,
                                 size_t address, uint32_t *value);
enum gru_error gru_blob_writefloatbe(struct gru_blob *blob,
                                     size_t address, float value);
enum gru_error gru_blob_writefloatle(struct gru_blob *blob,
                                     size_t address, float value);
enum gru_error gru_blob_readfloatbe(struct gru_blob *blob,
                                    size_t address, float *value);
enum gru_error gru_blob_readfloatle(struct gru_blob *blob,
                                    size_t address, float *value);
enum gru_error gru_blob_writestring(struct gru_blob *blob,
                                    size_t address, size_t size,
                                    const char *value);
enum gru_error gru_blob_readstring(struct gru_blob *blob,
                                   size_t address, size_t size,
                                   char **value);
void gru_blob_swap(struct gru_blob *blob, size_t word_size);
size_t gru_blob_find(struct gru_blob *blob,
                     struct gru_blob *search,
                     size_t start, size_t length);

#endif
