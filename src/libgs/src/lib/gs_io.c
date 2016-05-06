#include <stdint.h>
#include "libgs.h"
#include "mos_io.h"
#include "gs_io.h"


uint8_t gs_readwrite_u4(struct gs_context *gs, uint8_t data)
{
  {
    gs_mos_write(gs, data & 0x0F, 1);
    if (gs_failed(gs))
      return 0;
    int spin = 0, v;
    while (((v = gs_mos_read(gs)) & 0x10) == 0x00) {
      if (gs_failed(gs))
        return 0;
      ++spin;
      if (spin > gs->spin_max) {
        gs_set_error(gs, GS_ERROR,
                     "gs_readwrite_u4 spin is stuck on 0x%02x", v);
        return 0;
      }
    }
    if (gs_failed(gs))
      return 0;
  }

  {
    uint8_t data_in = gs_mos_read(gs) & 0x0F;
    if (gs_failed(gs))
      return 0;
    gs_mos_clear(gs);
    if (gs_failed(gs))
      return 0;
    int spin = 0, v;
    while (((v = gs_mos_read(gs)) & 0x10) == 0x10) {
      if (gs_failed(gs))
        return 0;
      ++spin;
      if (spin > gs->spin_max) {
        gs_set_error(gs, GS_ERROR,
                     "gs_readwrite_u4 spin is stuck on 0x%02x", v);
        return 0;
      }
    }
    if (gs_failed(gs))
      return 0;
    gs_clear_error(gs);
    return data_in;
  }
}


void gs_write_u4(struct gs_context *gs, uint8_t data)
{
  gs_mos_write(gs, data & 0x0F, 1);
  if (gs_failed(gs))
    return;
  gs_mos_clear(gs);
}

uint8_t gs_readwrite_u8(struct gs_context *gs, uint8_t data)
{
  uint8_t result = 0;
  for (int i = 1; i >= 0; --i) {
    result |= gs_readwrite_u4(gs, (data >> i * 4) & 0x0F) << i * 4;
    if (gs_failed(gs))
      return 0;
  }
  return result;
}

void gs_write_u8(struct gs_context *gs, uint8_t data)
{
  gs_write_u4(gs, (data >> 4) & 0x0F);
  if (gs_failed(gs))
    return;
  gs_write_u4(gs, (data >> 0) & 0x0F);
}

void gs_write_u8_fast(struct gs_context *gs, uint8_t data)
{
  gs_mos_write_fast_async(gs, data);
}

uint8_t gs_read_u8(struct gs_context *gs)
{
  return gs_readwrite_u8(gs, 0);
}

uint16_t gs_readwrite_u16(struct gs_context *gs, uint16_t data)
{
  uint16_t data_in = 0;
  for (int i = 1; i >= 0; --i) {
    data_in |= (uint16_t)gs_readwrite_u8(gs, (data >> i * 8) & 0xFF) << i * 8;
    if (gs_failed(gs))
      return 0;
  }
  return data_in;
}

uint32_t gs_readwrite_u32(struct gs_context *gs, uint32_t data)
{
  uint32_t data_in = 0;
  for (int i = 3; i >= 0; --i) {
    data_in |= (uint32_t)gs_readwrite_u8(gs, (data >> i * 8) & 0xFF) << i * 8;
    if (gs_failed(gs))
      return 0;
  }
  return data_in;
}

void gs_write_u32(struct gs_context *gs, uint32_t data)
{
  for (int i = 3; i >= 0; --i) {
    gs_write_u8(gs, (data >> i * 8) & 0xFF);
    if (gs_failed(gs))
      return;
  }
}

void gs_readwrite(struct gs_context *gs, void *data_in, const void *data_out,
                  uint32_t data_size)
{
  uint8_t *c_data_in = data_in;
  const uint8_t *c_data_out = data_out;
  while (data_size--) {
    uint8_t data = gs_readwrite_u8(gs, c_data_out ? *c_data_out++ : 0);
    if (gs_failed(gs))
      return;
    if (c_data_in)
      *c_data_in++ = data;
  }
  gs_clear_error(gs);
}

void gs_read(struct gs_context *gs, void *data, uint32_t data_size)
{
  gs_readwrite(gs, data, NULL, data_size);
}
