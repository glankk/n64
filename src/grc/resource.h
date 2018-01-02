#ifndef RESOURCE_H
#define RESOURCE_H
#include <stdint.h>
#include "grc.h"

enum grc_error make_resource(const char *file_name, const char *resource_name,
                             const void *resource_data, uint32_t resource_size);

#endif
