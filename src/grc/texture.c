#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <jansson.h>
#include "../../include/mips.h"
#include "../../include/n64.h"
#include "../../include/grc.h"
#include "texture.h"
#include "resource.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct format_info
{
  const char   *name;
  size_t        bpp;
  void        (*make_proc)(void *dest, void *src, size_t line_width,
                           size_t tile_width, size_t tile_height,
                           size_t width, size_t height);
  uint8_t       im_fmt;
  uint8_t       im_siz;
};

static uint8_t make_i(uint8_t **src)
{
  uint8_t r = (*(*src)++ * 76  + 127) / 255;
  uint8_t g = (*(*src)++ * 150 + 127) / 255;
  uint8_t b = (*(*src)++ * 29  + 127) / 255;
  return r + g + b;
}

static void make_rgba32(void *dest, void *src, size_t line_width,
                        size_t tile_width, size_t tile_height,
                        size_t width, size_t height)
{
  uint8_t *d = dest;
  uint8_t *s = src;
  for (size_t y = 0; y < tile_height; ++y) {
    uint8_t *l = s;
    for (size_t x = 0; x < tile_width; ++x) {
      if (x < width && y < height) {
        *d++ = *l++;
        *d++ = *l++;
        *d++ = *l++;
        *d++ = *l++;
      }
      else {
        *d++ = 0;
        *d++ = 0;
        *d++ = 0;
        *d++ = 0;
      }
    }
    s += line_width * 4;
  }
}

static void make_rgba16(void *dest, void *src, size_t line_width,
                        size_t tile_width, size_t tile_height,
                        size_t width, size_t height)
{
  uint8_t  *d = dest;
  uint8_t  *s = src;
  for (size_t y = 0; y < tile_height; ++y) {
    uint8_t *l = s;
    for (size_t x = 0; x < tile_width; ++x) {
      if (x < width && y < height) {
        uint16_t r = (*l++ * 31 + 127) / 255;
        uint16_t g = (*l++ * 31 + 127) / 255;
        uint16_t b = (*l++ * 31 + 127) / 255;
        uint16_t a = (*l++ * 1  + 254) / 255;
        uint16_t p = (r << 11) | (g << 6) | (b << 1) | (a << 0);
        *d++ = (p & 0xFF00) >> 8;
        *d++ = (p & 0x00FF) >> 0;
      }
      else {
        *d++ = 0;
        *d++ = 0;
      }
    }
    s += line_width * 4;
  }
}

static void make_ia16(void *dest, void *src, size_t line_width,
                      size_t tile_width, size_t tile_height,
                      size_t width, size_t height)
{
  uint8_t  *d = dest;
  uint8_t  *s = src;
  for (size_t y = 0; y < tile_height; ++y) {
    uint8_t *l = s;
    for (size_t x = 0; x < tile_width; ++x) {
      if (x < width && y < height) {
        uint16_t i = make_i(&l);
        uint16_t a = *l++;
        uint16_t p = (i << 8) | (a << 0);
        *d++ = (p & 0xFF00) >> 8;
        *d++ = (p & 0x00FF) >> 0;
      }
      else {
        *d++ = 0;
        *d++ = 0;
      }
    }
    s += line_width * 4;
  }
}

static void make_ia8(void *dest, void *src, size_t line_width,
                     size_t tile_width, size_t tile_height,
                     size_t width, size_t height)
{
  uint8_t  *d = dest;
  uint8_t  *s = src;
  for (size_t y = 0; y < tile_height; ++y) {
    uint8_t *l = s;
    for (size_t x = 0; x < tile_width; ++x) {
      uint8_t p = 0;
      if (x < width && y < height) {
        uint8_t i = (make_i(&l) * 15 + 127) / 255;
        uint8_t a = (*l++       * 15 + 127) / 255;
        p = (i << 4) | (a << 0);
      }
      *d++ = p;
    }
    s += line_width * 4;
  }
}

static void make_ia4(void *dest, void *src, size_t line_width,
                     size_t tile_width, size_t tile_height,
                     size_t width, size_t height)
{
  uint8_t  *d = dest;
  uint8_t  *s = src;
  uint8_t   b = 0;
  for (size_t y = 0; y < tile_height; ++y) {
    uint8_t  *l = s;
    int       n = 0;
    for (size_t x = 0; x <= tile_width; ++x) {
      uint8_t p = 0;
      if (x < width && y < height) {
        uint8_t i = (make_i(&l) * 7  + 127) / 255;
        uint8_t a = (*l++       * 1  + 254) / 255;
        p = (i << 1) | (a << 0);
      }
      if (n++ % 2 == 0)
        b |= p << 4;
      else {
        b |= p;
        *d++ = b;
        b = 0;
      }
    }
    s += line_width * 4;
  }
}

static void make_i8(void *dest, void *src, size_t line_width,
                    size_t tile_width, size_t tile_height,
                    size_t width, size_t height)
{
  uint8_t  *d = dest;
  uint8_t  *s = src;
  for (size_t y = 0; y < tile_height; ++y) {
    uint8_t *l = s;
    for (size_t x = 0; x < tile_width; ++x) {
      if (x < width && y < height) {
        *d++ = make_i(&l);
        ++l;
      }
      else
        *d++ = 0;
    }
    s += line_width * 4;
  }
}

static void make_i4(void *dest, void *src, size_t line_width,
                    size_t tile_width, size_t tile_height,
                    size_t width, size_t height)
{
  uint8_t  *d = dest;
  uint8_t  *s = src;
  uint8_t   b = 0;
  for (size_t y = 0; y < tile_height; ++y) {
    uint8_t  *l = s;
    int       n = 0;
    for (size_t x = 0; x <= tile_width; ++x) {
      uint8_t p = 0;
      if (x < width && y < height) {
        p = (make_i(&l) * 15 + 127) / 255;
        ++l;
      }
      if (n++ % 2 == 0)
        b |= p << 4;
      else {
        b |= p;
        *d++ = b;
        b = 0;
      }
    }
    s += line_width * 4;
  }
}

static struct format_info format_info[] =
{
  {"rgba32", 32, make_rgba32, G_IM_FMT_RGBA, G_IM_SIZ_32b},
  {"rgba16", 16, make_rgba16, G_IM_FMT_RGBA, G_IM_SIZ_16b},
  {"ia16",   16, make_ia16,   G_IM_FMT_IA,   G_IM_SIZ_16b},
  {"ia8",    8,  make_ia8,    G_IM_FMT_IA,   G_IM_SIZ_8b},
  {"ia4",    4,  make_ia4,    G_IM_FMT_IA,   G_IM_SIZ_4b},
  {"i8",     8,  make_i8,     G_IM_FMT_I,    G_IM_SIZ_8b},
  {"i4",     4,  make_i4,     G_IM_FMT_I,    G_IM_SIZ_4b},
};

enum grc_error make_texture(const char *input_file, const char *output_file,
                            const char *resource_name, json_t *j_descriptor)
{
  /* parse descriptor */
  struct format_info *fi = &format_info[0];
  int tile_width = 0;
  int tile_height = 0;
  if (j_descriptor) {
    json_t *j_format = json_object_get(j_descriptor, "format");
    if (j_format) {
      if (!json_is_string(j_format))
        return grc_set_error(GRC_ERROR_DATA, "invalid image format parameter");
      _Bool format_found = 0;
      const char *format = json_string_value(j_format);
      for (int i = 0; i < sizeof(format_info) / sizeof(*format_info); ++i)
        if (strcmp(format_info[i].name, format) == 0) {
          format_found = 1;
          fi = &format_info[i];
          break;
        }
      if (!format_found)
        return grc_set_error(GRC_ERROR_PARAM,
                             "unrecognized texture format `%s`", format);
    }
    json_t *j_tile_width = json_object_get(j_descriptor, "tile_width");
    if (j_tile_width) {
      if (!json_is_integer(j_tile_width))
        return grc_set_error(GRC_ERROR_DATA, "invalid tile width parameter");
      tile_width = json_integer_value(j_tile_width);
    }
    json_t *j_tile_height = json_object_get(j_descriptor, "tile_height");
    if (j_tile_height) {
      if (!json_is_integer(j_tile_height))
        return grc_set_error(GRC_ERROR_DATA, "invalid tile height parameter");
      tile_height = json_integer_value(j_tile_height);
    }
  }
  /* load image */
  int image_width;
  int image_height;
  void *image_data = stbi_load(input_file, &image_width, &image_height,
                               NULL, 4);
  if (!image_data)
    return grc_set_error(GRC_ERROR_FILEIN, "failed to load image `%s`",
                         input_file);
  /* prepare texture */
  if (tile_width <= 0) {
    tile_width = image_width;
    if (fi->bpp == 4)
      tile_width += tile_width % 2;
  }
  if (tile_height <= 0)
    tile_height = image_height;
  size_t tile_size = ((tile_width * tile_height * fi->bpp + 7) / 8 + 63) /
                     64 * 64;
  size_t tiles_x = (image_width + tile_width - 1) / tile_width;
  size_t tiles_y = (image_height + tile_height - 1) / tile_height;
  size_t resource_size = sizeof(struct grc_texture) +
                         tile_size * tiles_x * tiles_y;
  struct grc_texture *resource_data = malloc(resource_size);
  if (!resource_data) {
    stbi_image_free(image_data);
    return grc_set_error(GRC_ERROR_MEMORY, NULL);
  }
  resource_data->im_fmt = fi->im_fmt;
  resource_data->im_siz = fi->im_siz;
  resource_data->image_width = mips_htom16(image_width);
  resource_data->image_height = mips_htom16(image_height);
  resource_data->tile_width = mips_htom16(tile_width);
  resource_data->tile_height = mips_htom16(tile_height);
  resource_data->tiles_x = mips_htom16(tiles_x);
  resource_data->tiles_y = mips_htom16(tiles_y);
  /* make texture */
  {
    char *texture_p = &resource_data->texture_data[0];
    for (int y = 0; y < image_height; y += tile_height) {
      char *image_p = (char*)image_data + y * image_width * 4;
      int h = y + tile_height <= image_height ? tile_height : image_height - y;
      for (int x = 0; x < image_width; x += tile_width) {
        int w = x + tile_width <= image_width ? tile_width : image_width - x;
        memset(texture_p, 0, tile_size);
        fi->make_proc(texture_p, image_p, image_width,
                      tile_width, tile_height, w, h);
        texture_p += tile_size;
        image_p += w * 4;
      }
    }
  }
  stbi_image_free(image_data);
  /* make resource */
  enum grc_error e = make_resource(output_file, resource_name, resource_data,
                                   resource_size);
  free(resource_data);
  if (e)
    return e;
  return grc_set_error(GRC_SUCCESS, NULL);
}
