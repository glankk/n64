#ifndef GRC_H
#define GRC_H
#include <jansson.h>

enum grc_error
{
  GRC_SUCCESS,
  GRC_ERROR_PARAM,
  GRC_ERROR_MEMORY,
  GRC_ERROR_FILEIN,
  GRC_ERROR_FILEOUT,
  GRC_ERROR_DATA,
};

typedef enum grc_error (*grc_res_proc)(const char *input_file,
                                       const char *output_file,
                                       const char *resource_name,
                                       json_t *j_descriptor);
struct grc_res_type
{
  const char     *name;
  grc_res_proc    proc;
};

enum grc_error grc_set_error(enum grc_error error_code,
                             const char *format, ...);

#endif
