#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "resource.h"

static enum grc_error print_data(FILE *f, const void *data, uint32_t size)
{
  if (fputs(".byte ", f) == EOF)
    goto io_error;
  const uint8_t *p = data;
  for (uint32_t i = 0; i < size; ++i) {
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

static enum grc_error make_resource_src(FILE *f,
                                        const char *resource_name,
                                        const void *resource_data,
                                        uint32_t resource_size)
{
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
  return grc_set_error(GRC_SUCCESS, NULL);
io_error:
  return grc_set_error(GRC_ERROR_FILEOUT, NULL);
}

enum grc_error make_resource(const char *file_name, const char *resource_name,
                             const void *resource_data, uint32_t resource_size)
{
  enum grc_error e;
  const char *as = getenv("AS");
  if (!as)
    as = "mips64-as";
  const char *f = "%s - -o \"%s\" -march=vr4300 -mabi=32";
  char *s = malloc(snprintf(NULL, 0, f, as, file_name) + 1);
  FILE *p = NULL;
  if (!s) {
    e = grc_set_error(GRC_ERROR_MEMORY, NULL);
    goto exit;
  }
  sprintf(s, f, as, file_name);
  p = popen(s, "w");
  if (!p) {
    e = grc_set_error(GRC_ERROR_FILEOUT, "could not open pipe `%s`", s);
    goto exit;
  }
  e = make_resource_src(p, resource_name, resource_data, resource_size);
  if (e)
    goto exit;
  if (pclose(p)) {
    p = NULL;
    e = grc_set_error(GRC_ERROR_FILEOUT, "`%s` failed", s);
    goto exit;
  }
  p = NULL;
  e = grc_set_error(GRC_SUCCESS, NULL);
exit:
  if (s)
    free(s);
  if (p)
    pclose(p);
  return e;
}
