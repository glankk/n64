#ifndef OPS_H
#define OPS_H
#include <stddef.h>
#include "../lib/libgs.h"

void write_codes(struct gs_code *codes, size_t num_codes,
                 size_t data_size, enum gs_error *gs_error_code,
                 char (*gs_error_description)[GS_ERROR_LENGTH]);
void write_raw(uint32_t address, void *data, size_t data_size,
               size_t chunk_size, enum gs_error *gs_error_code,
               char (*gs_error_description)[GS_ERROR_LENGTH]);

#endif
