#ifndef GS_IO_H
#define GS_IO_H
#include <stdint.h>
#include "libgs.h"

uint8_t gs_readwrite_u4(struct gs_context *gs, uint8_t data);
void gs_write_u4(struct gs_context *gs, uint8_t data);
uint8_t gs_readwrite_u8(struct gs_context *gs, uint8_t data);
void gs_write_u8(struct gs_context *gs, uint8_t data);
void gs_write_u8_fast(struct gs_context *gs, uint8_t data);
uint8_t gs_read_u8(struct gs_context *gs);
uint16_t gs_readwrite_u16(struct gs_context *gs, uint16_t data);
uint32_t gs_readwrite_u32(struct gs_context *gs, uint32_t data);
void gs_write_u32(struct gs_context *gs, uint32_t data);
void gs_readwrite(struct gs_context *gs, void *data_in, const void *data_out,
                  uint32_t data_size);
void gs_read(struct gs_context *gs, void *data, uint32_t data_size);

#endif
