#ifndef GRU_N64ROM_H
#define GRU_N64ROM_H
#include <stdint.h>
#include "gru.h"
#include "blob.h"

struct gru_n64rom
{
  struct gru_blob blob;
};

void gru_n64rom_init(struct gru_n64rom *n64rom);
void gru_n64rom_destroy(struct gru_n64rom *n64rom);
enum gru_error gru_n64rom_load_file(struct gru_n64rom *n64rom,
                                    const char *filename);
enum gru_error gru_n64rom_load_blob(struct gru_n64rom *n64rom,
                                    struct gru_blob *blob);
enum gru_error gru_n64rom_save_file(struct gru_n64rom *n64rom,
                                    const char *filename,
                                    enum gru_endian endian);
enum gru_error gru_n64rom_save_blob(struct gru_n64rom *n64rom,
                                    struct gru_blob *blob,
                                    enum gru_endian endian);
enum gru_error gru_n64rom_clock_get(struct gru_n64rom *n64rom,
                                    uint32_t *clock_out);
enum gru_error gru_n64rom_clock_set(struct gru_n64rom *n64rom,
                                    uint32_t clock);
enum gru_error gru_n64rom_pc_get(struct gru_n64rom *n64rom,
                                 uint32_t *pc_out);
enum gru_error gru_n64rom_pc_set(struct gru_n64rom *n64rom,
                                 uint32_t pc);
enum gru_error gru_n64rom_release_get(struct gru_n64rom *n64rom,
                                      uint32_t *release_out);
enum gru_error gru_n64rom_release_set(struct gru_n64rom *n64rom,
                                      uint32_t release);
enum gru_error gru_n64rom_crc_get(struct gru_n64rom *n64rom,
                                  uint32_t *crc1_out, uint32_t *crc2_out);
enum gru_error gru_n64rom_crc_set(struct gru_n64rom *n64rom,
                                  uint32_t crc1, uint32_t crc2);
enum gru_error gru_n64rom_name_get(struct gru_n64rom *n64rom,
                                   char name_out[20]);
enum gru_error gru_n64rom_name_set(struct gru_n64rom *n64rom,
                                   const char *name);
enum gru_error gru_n64rom_format_get(struct gru_n64rom *n64rom,
                                     char *format_out);
enum gru_error gru_n64rom_format_set(struct gru_n64rom *n64rom,
                                     char format);
enum gru_error gru_n64rom_id_get(struct gru_n64rom *n64rom,
                                 char id_out[2]);
enum gru_error gru_n64rom_id_set(struct gru_n64rom *n64rom,
                                 const char id[2]);
enum gru_error gru_n64rom_region_get(struct gru_n64rom *n64rom,
                                     char *region_out);
enum gru_error gru_n64rom_region_set(struct gru_n64rom *n64rom,
                                     char region);
enum gru_error gru_n64rom_version_get(struct gru_n64rom *n64rom,
                                      uint8_t *version_out);
enum gru_error gru_n64rom_version_set(struct gru_n64rom *n64rom,
                                      uint8_t version);
int gru_n64rom_cic(struct gru_n64rom *n64rom);
void gru_n64rom_crc_compute(struct gru_n64rom *n64rom,
                            uint32_t *crc1, uint32_t *crc2);
gru_bool_t gru_n64rom_crc_check(struct gru_n64rom *n64rom);
enum gru_error gru_n64rom_crc_update(struct gru_n64rom *n64rom);

#endif
