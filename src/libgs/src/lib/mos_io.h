#ifndef MOS_IO_H
#define MOS_IO_H
#include <stdint.h>
#include "libgs.h"

void gs_mos_set_mode(struct gs_context *gs, int mos_mode);
uint8_t gs_mos_read_raw(struct gs_context *gs);
uint8_t gs_mos_read(struct gs_context *gs);
void gs_mos_write(struct gs_context *gs, uint8_t data, int flagged);
void gs_mos_write_async(struct gs_context *gs, uint8_t data, int flagged);
void gs_mos_clear(struct gs_context *gs);
void gs_mos_clear_async(struct gs_context *gs);
void gs_mos_write_fast(struct gs_context *gs, uint8_t data);
void gs_mos_write_fast_async(struct gs_context *gs, uint8_t data);
void gs_mos_write_bulk_async(struct gs_context *gs,
                             const void *data, size_t data_size);
void gs_mos_clear_bulk_async(struct gs_context *gs);

#endif
