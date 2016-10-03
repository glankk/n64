#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <vector/vector.h>
#include "gru.h"
#include "gsc.h"
#include "blob.h"
#include "util.h"

struct gsc_code
{
  uint32_t address;
  uint16_t value;
};

static char *getline(FILE *file)
{
  const size_t buffer_size = 256;
  char *line = NULL;
  for (size_t i = 0; ; ++i) {
    int c = fgetc(file);
    if (ferror(file)) {
      if (line)
        free(line);
      return NULL;
    }
    if (i % buffer_size == 0) {
      char *new_line = realloc(line, i + buffer_size);
      if (!new_line) {
        if (line)
          free(line);
        return NULL;
      }
      line = new_line;
    }
    if (c == EOF || c == '\n') {
      line[i] = 0;
      return line;
    }
    else
      line[i] = c;
  }
}

static gru_bool_t isxstrl(const char *string, size_t length)
{
  const char *s;
  for (s = string; *s != 0; ++s)
    if (!isxdigit(*s))
      return GRU_FALSE;
  return s - string == length;
}

void gru_gsc_init(struct gru_gsc *gsc)
{
  vector_init(&gsc->codes, sizeof(struct gsc_code));
}

void gru_gsc_destroy(struct gru_gsc *gsc)
{
  vector_destroy(&gsc->codes);
}

enum gru_error gru_gsc_load(struct gru_gsc *gsc, const char *filename)
{
  vector_clear(&gsc->codes);
  FILE *file = fopen(filename, "r");
  if (!file)
    return GRU_ERROR_FILEIN;
  while (!feof(file)) {
    char *line = getline(file);
    if (!line) {
      fclose(file);
      return GRU_ERROR_FILEIN;
    }
    char address[10] = "";
    char value[6] = "";
    sscanf(line, " %9s %5s", address, value);
    free(line);
    if (!isxstrl(address, 8) || !isxstrl(value, 4))
      continue;
    struct gsc_code code;
    sscanf(address, "%" SCNx32, &code.address);
    sscanf(value, "%" SCNx16, &code.value);
    if (!vector_push_back(&gsc->codes, 1, &code)) {
      fclose(file);
      return GRU_ERROR_MEMORY;
    }
  }
  fclose(file);
  return GRU_SUCCESS;
}

enum gru_error gru_gsc_save(struct gru_gsc *gsc, const char *filename)
{
  FILE *file = fopen(filename, "w");
  if (!file)
    return GRU_ERROR_FILEOUT;
  for (size_t i = 0; i < gsc->codes.size; ++i) {
    struct gsc_code *code = vector_at(&gsc->codes, i);
    if (fprintf(file, "%08" PRIX32 " %04" PRIX16 "\n",
                code->address, code->value) < 0)
    {
      fclose(file);
      return GRU_ERROR_FILEOUT;
    }
  }
  fclose(file);
  return GRU_SUCCESS;
}

size_t gru_gsc_size(struct gru_gsc *gsc)
{
  return gsc->codes.size;
}

enum gru_error gru_gsc_insert(struct gru_gsc *gsc, size_t position,
                              uint32_t address, uint16_t value)
{
  if (position > gsc->codes.size)
    return GRU_ERROR_RANGE;
  struct gsc_code code = {address, value};
  if (!vector_insert(&gsc->codes, position, 1, &code))
    return GRU_ERROR_MEMORY;
  return GRU_SUCCESS;
}

enum gru_error gru_gsc_replace(struct gru_gsc *gsc, size_t index,
                               uint32_t address, uint16_t value)
{
  if (index >= gsc->codes.size)
    return GRU_ERROR_RANGE;
  struct gsc_code new_code = {address, value};
  struct gsc_code *code = vector_at(&gsc->codes, index);
  *code = new_code;
  return GRU_SUCCESS;
}

enum gru_error gru_gsc_remove(struct gru_gsc *gsc, size_t index)
{
  if (index >= gsc->codes.size)
    return GRU_ERROR_RANGE;
  vector_erase(&gsc->codes, index, 1);
  return GRU_SUCCESS;
}

enum gru_error gru_gsc_get(struct gru_gsc *gsc, size_t index,
                           uint32_t *address, uint16_t *value)
{
  if (index >= gsc->codes.size)
    return GRU_ERROR_RANGE;
  struct gsc_code *code = vector_at(&gsc->codes, index);
  *address = code->address;
  *value = code->value;
  return GRU_SUCCESS;
}

void gru_gsc_shift(struct gru_gsc *gsc, int32_t offset)
{
  for (size_t i = 0; i < gsc->codes.size; ++i) {
    struct gsc_code *code = vector_at(&gsc->codes, i);
    uint32_t head = code->address & 0xFF000000;
    uint32_t address = code->address & 0x00FFFFFF;
    address = (address + offset) & 0x00FFFFFF;
    code->address = head | address;
  }
}

static enum gru_error gru_gsc_apply(struct gru_gsc *gsc, struct gru_blob *blob,
                                    uint16_t (*byte_order_proc)(uint16_t))
{
  for (size_t i = 0; i < gsc->codes.size; ++i) {
    struct gsc_code *code = vector_at(&gsc->codes, i);
    uint32_t head = code->address & 0xFF000000;
    uint32_t address = code->address & 0x00FFFFFF;
    size_t value_size;
    if (head == 0x80000000)
      value_size = 1;
    else if (head == 0x81000000)
      value_size = 2;
    else
      continue;
    if (gru_blob_ensure_size(blob, address + value_size))
      return GRU_ERROR_MEMORY;
    void *code_ptr = blob_at(blob, address);
    if (head == 0x80000000)
      *(uint8_t*)code_ptr = code->value & 0x00FF;
    else if (head == 0x81000000)
      *(uint16_t*)code_ptr = byte_order_proc(code->value);
  }
  return GRU_SUCCESS;
}

enum gru_error gru_gsc_apply_be(struct gru_gsc *gsc, struct gru_blob *blob)
{
  return gru_gsc_apply(gsc, blob, gru_util_htobe16);
}

enum gru_error gru_gsc_apply_le(struct gru_gsc *gsc, struct gru_blob *blob)
{
  return gru_gsc_apply(gsc, blob, gru_util_htole16);
}
