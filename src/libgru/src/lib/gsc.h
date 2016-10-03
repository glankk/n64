#ifndef GRU_GSC_H
#define GRU_GSC_H
#include <stddef.h>
#include <stdint.h>
#include <vector/vector.h>
#include "blob.h"
#include "gru.h"

struct gru_gsc
{
  struct vector codes;
};

void gru_gsc_init(struct gru_gsc *gsc);
void gru_gsc_destroy(struct gru_gsc *gsc);
enum gru_error gru_gsc_load(struct gru_gsc *gsc, const char *filename);
enum gru_error gru_gsc_save(struct gru_gsc *gsc, const char *filename);
size_t gru_gsc_size(struct gru_gsc *gsc);
enum gru_error gru_gsc_insert(struct gru_gsc *gsc, size_t position,
                              uint32_t address, uint16_t value);
enum gru_error gru_gsc_replace(struct gru_gsc *gsc, size_t index,
                               uint32_t address, uint16_t value);
enum gru_error gru_gsc_remove(struct gru_gsc *gsc, size_t index);
enum gru_error gru_gsc_get(struct gru_gsc *gsc, size_t index,
                           uint32_t *address, uint16_t *value);
void gru_gsc_shift(struct gru_gsc *gsc, int32_t offset);
enum gru_error gru_gsc_apply_be(struct gru_gsc *gsc, struct gru_blob *blob);
enum gru_error gru_gsc_apply_le(struct gru_gsc *gsc, struct gru_blob *blob);

#endif
