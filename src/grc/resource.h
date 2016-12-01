#ifndef RESOURCE_H
#define RESOURCE_H
#include <stddef.h>
#include "grc.h"

enum grc_error make_resource(const char *file_name, const char *resource_name,
                             const void *resource_data, size_t resource_size);

#endif
