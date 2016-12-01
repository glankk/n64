#ifndef TEXTURE_H
#define TEXTURE_H
#include <jansson.h>
#include "grc.h"

enum grc_error make_texture(const char *input_file, const char *output_file,
                            const char *resource_name, json_t *j_descriptor);

#endif
