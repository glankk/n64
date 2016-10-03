#ifndef GRU_UPS_H
#define GRU_UPS_H
#include <stddef.h>
#include <stdint.h>
#include <vector/vector.h>
#include "gru.h"
#include "blob.h"

struct gru_ups
{
  size_t src_size;
  size_t dst_size;
  uint32_t src_crc;
  uint32_t dst_crc;
  struct vector chunks;
};

void gru_ups_init(struct gru_ups *ups);
void gru_ups_destroy(struct gru_ups *ups);
enum gru_error gru_ups_load(struct gru_ups *ups, const char *filename);
enum gru_error gru_ups_save(struct gru_ups *ups, const char *filename);
enum gru_error gru_ups_make(struct gru_ups *ups, struct gru_blob *src_blob,
                            struct gru_blob *dst_blob);
enum gru_error gru_ups_apply(struct gru_ups *ups, struct gru_blob *blob,
                             gru_bool_t ignore_checksums);
enum gru_error gru_ups_undo(struct gru_ups *ups, struct gru_blob *blob,
                            gru_bool_t ignore_checksums);
size_t gru_ups_src_size(struct gru_ups *ups);
size_t gru_ups_dst_size(struct gru_ups *ups);
uint32_t gru_ups_src_crc(struct gru_ups *ups);
uint32_t gru_ups_dst_crc(struct gru_ups *ups);

#endif
