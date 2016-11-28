#ifndef N64_GRC_H
#define N64_GRC_H
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
  uint8_t   im_fmt;
  uint8_t   im_siz;
  uint16_t  image_width;
  uint16_t  image_height;
  uint16_t  tile_width;
  uint16_t  tile_height;
  uint16_t  tiles_x;
  uint16_t  tiles_y;
  char      texture_data[];
} grc_texture_t;

int grc_resource_get(const char *res_name, void **data, size_t *size);

#ifdef __cplusplus
}
#endif

#endif
