#include <string.h>
#include <stdint.h>
#include "gru.h"
#include "n64rom.h"
#include "blob.h"
#include "util.h"

struct n64_header
{
  struct
  {
    uint32_t  endian : 8;
    uint32_t  rls    : 4;
    uint32_t  pgs    : 4;
    uint32_t  pwd    : 8;
    uint32_t  lat    : 8;
  }           pi_bsd_dom1;
  uint32_t    clock;
  uint32_t    pc;
  uint32_t    release;
  uint32_t    crc1;
  uint32_t    crc2;
  char        reserved_00_[0x0008];
  char        name[20];
  char        reserved_01_[0x0007];
  char        format;
  char        id[2];
  char        region;
  uint8_t     version;
};

static struct n64_header *get_header(struct gru_n64rom *n64rom)
{
  if (gru_blob_ensure_size(&n64rom->blob, 0x40))
    return NULL;
  return (struct n64_header*)n64rom->blob.data;
}

static uint32_t safe_u32be(struct gru_n64rom *n64rom, size_t address)
{
  uint32_t n = 0;
  for (size_t i = 0; i < 4 && address + i < n64rom->blob.size; ++i)
    n |= *(uint8_t*)blob_at(&n64rom->blob, address + i) << (24 - i * 8);
  return n;
}

void gru_n64rom_init(struct gru_n64rom *n64rom)
{
  gru_blob_init(&n64rom->blob);
}

void gru_n64rom_destroy(struct gru_n64rom *n64rom)
{
  gru_blob_destroy(&n64rom->blob);
}

enum gru_error gru_n64rom_load_file(struct gru_n64rom *n64rom,
                                    const char *filename)
{
  enum gru_error e = gru_blob_load(&n64rom->blob, filename);
  if (e)
    return e;
  uint32_t endian = safe_u32be(n64rom, 0);
  if ((endian & 0x00FF0000) == 0x00800000)
    gru_blob_swap(&n64rom->blob, 2);
  else if ((endian & 0x000000FF) == 0x00000080)
    gru_blob_swap(&n64rom->blob, 4);
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_load_blob(struct gru_n64rom *n64rom,
                                    struct gru_blob *blob)
{
  struct gru_blob blob_copy;
  enum gru_error e = gru_blob_copy(blob, 0, blob->size, &blob_copy);
  if (e)
    return e;
  uint32_t endian = safe_u32be(n64rom, 0);
  if ((endian & 0x00FF0000) == 0x00800000)
    gru_blob_swap(&n64rom->blob, 2);
  else if ((endian & 0x000000FF) == 0x00000080)
    gru_blob_swap(&n64rom->blob, 4);
  gru_blob_destroy(&n64rom->blob);
  n64rom->blob = blob_copy;
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_save_file(struct gru_n64rom *n64rom,
                                    const char *filename,
                                    enum gru_endian endian)
{
  size_t word_size;
  switch (endian) {
    case GRU_ENDIAN_BIG_4:
      return gru_blob_save(&n64rom->blob, filename);
    case GRU_ENDIAN_LITTLE_4:
      word_size = 4;
      break;
    case GRU_ENDIAN_LITTLE_2:
      word_size = 2;
      break;
    default:
      return GRU_ERROR_PARAM;
  }
  struct gru_blob blob;
  enum gru_error e = gru_blob_copy(&n64rom->blob, 0, n64rom->blob.size,
                                   &blob);
  if (e)
    return e;
  gru_blob_swap(&blob, word_size);
  e = gru_blob_save(&blob, filename);
  gru_blob_destroy(&blob);
  return e;
}

enum gru_error gru_n64rom_save_blob(struct gru_n64rom *n64rom,
                                    struct gru_blob *blob,
                                    enum gru_endian endian)
{
  gru_bool_t swap = GRU_FALSE;
  size_t word_size;
  switch (endian) {
    case GRU_ENDIAN_BIG_4:
      break;
    case GRU_ENDIAN_LITTLE_4:
      swap = GRU_TRUE;
      word_size = 4;
      break;
    case GRU_ENDIAN_LITTLE_2:
      swap = GRU_TRUE;
      word_size = 2;
      break;
    default:
      return GRU_ERROR_PARAM;
  }
  enum gru_error e = gru_blob_copy(&n64rom->blob, 0, n64rom->blob.size,
                                   blob);
  if (e)
    return e;
  if (swap)
    gru_blob_swap(blob, word_size);
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_clock_get(struct gru_n64rom *n64rom,
                                    uint32_t *clock_out)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  *clock_out = gru_util_betoh32(header->clock);
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_clock_set(struct gru_n64rom *n64rom,
                                    uint32_t clock)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  header->clock = gru_util_htobe32(clock);
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_pc_get(struct gru_n64rom *n64rom,
                                 uint32_t *pc_out)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  *pc_out = gru_util_betoh32(header->pc);
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_pc_set(struct gru_n64rom *n64rom,
                                 uint32_t pc)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  header->pc = gru_util_htobe32(pc);
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_release_get(struct gru_n64rom *n64rom,
                                      uint32_t *release_out)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  *release_out = gru_util_betoh32(header->release);
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_release_set(struct gru_n64rom *n64rom,
                                      uint32_t release)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  header->release = gru_util_htobe32(release);
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_crc_get(struct gru_n64rom *n64rom,
                                  uint32_t *crc1_out, uint32_t *crc2_out)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  *crc1_out = gru_util_betoh32(header->crc1);
  *crc2_out = gru_util_betoh32(header->crc2);
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_crc_set(struct gru_n64rom *n64rom,
                                  uint32_t crc1, uint32_t crc2)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  header->crc1 = gru_util_htobe32(crc1);
  header->crc2 = gru_util_htobe32(crc2);
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_name_get(struct gru_n64rom *n64rom,
                                   char name_out[20])
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  memcpy(name_out, header->name, sizeof(header->name));
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_name_set(struct gru_n64rom *n64rom,
                                   const char *name)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  memset(header->name, 0, sizeof(header->name));
  strncpy(header->name, name, sizeof(header->name));
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_format_get(struct gru_n64rom *n64rom,
                                     char *format_out)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  *format_out = header->format;
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_format_set(struct gru_n64rom *n64rom,
                                     char format)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  header->format = format;
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_id_get(struct gru_n64rom *n64rom,
                                 char id_out[2])
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  memcpy(id_out, header->id, 2);
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_id_set(struct gru_n64rom *n64rom,
                                 const char id[2])
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  memcpy(header->id, id, 2);
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_region_get(struct gru_n64rom *n64rom,
                                     char *region_out)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  *region_out = header->region;
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_region_set(struct gru_n64rom *n64rom,
                                     char region)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  header->region = region;
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_version_get(struct gru_n64rom *n64rom,
                                      uint8_t *version_out)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  *version_out = header->version;
  return GRU_SUCCESS;
}

enum gru_error gru_n64rom_version_set(struct gru_n64rom *n64rom,
                                      uint8_t version)
{
  struct n64_header *header = get_header(n64rom);
  if (!header)
    return GRU_ERROR_MEMORY;
  header->version = version;
  return GRU_SUCCESS;
}

int gru_n64rom_cic(struct gru_n64rom *n64rom)
{
  if (n64rom->blob.size >= 0x1000)
    switch (gru_util_crc32(blob_at(&n64rom->blob, 0x40), 0x1000 - 0x40)) {
      case 0x6170A4A1: return 6101;
      case 0x90BB6CB5: return 6102;
      case 0x0B050EE0: return 6103;
      case 0x98BC2C86: return 6105;
      case 0xACC8580A: return 6106;
    }
  return 0;
}

void gru_n64rom_crc_compute(struct gru_n64rom *n64rom,
                            uint32_t *crc1, uint32_t *crc2)
{
  int cic = gru_n64rom_cic(n64rom);
  uint32_t seed;
  switch (cic) {
    default:   seed = 0xF8CA4DDC; break;
    case 6103: seed = 0xA3886759; break;
    case 6105: seed = 0xDF26F436; break;
    case 6106: seed = 0x1FEA617A; break;
  }
  uint32_t reg[6] = {seed, seed, seed, seed, seed, seed};
  for (size_t i = 0x1000; i < 0x101000; i += 4) {
    uint32_t t_reg[4];
    t_reg[0] = safe_u32be(n64rom, i);
    t_reg[1] = t_reg[0] & 0x1F;
    t_reg[2] = (t_reg[0] << t_reg[1]) | (t_reg[0] >> (32 - t_reg[1]));
    t_reg[3] = reg[5];
    reg[5] += t_reg[0];
    reg[4] += t_reg[2];
    if (reg[5] < t_reg[3])
      ++reg[3];
    reg[2] ^= t_reg[0];
    reg[1] ^= (reg[1] > t_reg[0] ? t_reg[2] : reg[5] ^ t_reg[0]);
    reg[0] += (cic == 6105 ?
               safe_u32be(n64rom, 0x750 + (i & 0xFF)) : reg[4]) ^ t_reg[0];
  }
  if (cic == 6103) {
    *crc1 = (reg[5] ^ reg[3]) + reg[2];
    *crc2 = (reg[4] ^ reg[1]) + reg[0];
  }
  else if (cic == 6106) {
    *crc1 = (reg[5] * reg[3]) + reg[2];
    *crc2 = (reg[4] * reg[1]) + reg[0];
  }
  else {
    *crc1 = (reg[5] ^ reg[3]) ^ reg[2];
    *crc2 = (reg[4] ^ reg[1]) ^ reg[0];
  }
}

gru_bool_t gru_n64rom_crc_check(struct gru_n64rom *n64rom)
{
  uint32_t crc1;
  uint32_t crc2;
  gru_n64rom_crc_compute(n64rom, &crc1, &crc2);
  return crc1 == safe_u32be(n64rom, 0x10) && crc2 == safe_u32be(n64rom, 0x14);
}

enum gru_error gru_n64rom_crc_update(struct gru_n64rom *n64rom)
{
  uint32_t crc1;
  uint32_t crc2;
  gru_n64rom_crc_compute(n64rom, &crc1, &crc2);
  return gru_n64rom_crc_set(n64rom, crc1, crc2);
}
