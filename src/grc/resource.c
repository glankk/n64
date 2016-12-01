#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "resource.h"

static enum grc_error print_data(FILE *f, const void *data, size_t size)
{
  if (fputs(".byte ", f) == EOF)
    goto io_error;
  const uint8_t *p = data;
  for (size_t i = 0; i < size; ++i) {
    if (i > 0) {
      if (fputs(", ", f) == EOF)
        goto io_error;
    }
    if (fprintf(f, "%" PRIu8, *p++) < 0)
      goto io_error;
  }
  if (fputs("\n", f) == EOF)
    goto io_error;
  return grc_set_error(GRC_SUCCESS, NULL);
io_error:
  return grc_set_error(GRC_ERROR_FILEOUT, NULL);
}

static enum grc_error make_resource_src(const char *file_name,
                                        const char *resource_name,
                                        const void *resource_data,
                                        size_t resource_size)
{
  FILE *f = fopen(file_name, "w");
  if (!f)
    goto io_error;
  if (fprintf(f, ".section .data.resource_table\n"
                 ".long resource_name, resource_data, %" PRIu32 "\n",
              resource_size) < 0)
    goto io_error;
  if (fputs(".section .data\n"
            "resource_name: ", f) == EOF)
    goto io_error;
  enum grc_error e = print_data(f, resource_name, strlen(resource_name) + 1);
  if (e)
    return e;
  if (fputs(".section .data.resource_data\n"
            "resource_data: ", f) == EOF)
    goto io_error;
  e = print_data(f, resource_data, resource_size);
  if (e)
    return e;
  if (fclose(f) == EOF)
    goto io_error;
  return grc_set_error(GRC_SUCCESS, NULL);
io_error:
  return grc_set_error(GRC_ERROR_FILEOUT, NULL);
}

enum grc_error make_resource(const char *file_name, const char *resource_name,
                             const void *resource_data, size_t resource_size)
{
  const char *as = getenv("AS");
  if (!as)
    as = "mips64-as";
  const char *f = "%s \"%s\" -o \"%s\" -march=vr4300 -mtune=vr4300 -mabi=32";
  char resource_src_name[L_tmpnam + 2] = "./";
  tmpnam(&resource_src_name[2]);
  char *c = malloc(snprintf(NULL, 0, f, as, resource_src_name, file_name) + 1);
  if (!c)
    return grc_set_error(GRC_ERROR_MEMORY, NULL);
  sprintf(c, f, as, resource_src_name, file_name);
  enum grc_error e = make_resource_src(resource_src_name, resource_name,
                                       resource_data, resource_size);
  if (e)
    goto exit;
  if (system(c)) {
    e = grc_set_error(GRC_ERROR_FILEOUT, "execution of `%s` failed", c);
    goto exit;
  }
  e = grc_set_error(GRC_SUCCESS, NULL);
exit:
  free(c);
  remove(resource_src_name);
  return e;
}
