#ifndef GRC_H
#define GRC_H
#include <jansson.h>

typedef enum
{
  GRC_SUCCESS,
  GRC_ERROR_PARAM,
  GRC_ERROR_MEMORY,
  GRC_ERROR_FILEIN,
  GRC_ERROR_FILEOUT,
  GRC_ERROR_DATA,
} grc_error_t;

typedef grc_error_t (*grc_res_proc_t)(const char *input_file,
                                      const char *output_file,
                                      const char *resource_name,
                                      json_t *j_descriptor);
typedef struct
{
  const char     *name;
  grc_res_proc_t  proc;
} grc_res_type_t;

grc_error_t grc_set_error(grc_error_t error_code, const char *format, ...);

#endif
