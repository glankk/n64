#include "gru.h"

const char *error_strings[] =
{
  "success",
  "memory allocation failed",
  "parameter out of range",
  "invalid parameter",
  "file input error",
  "file output error",
  "data processing failed; data malformed",
};

const char *gru_error_string(enum gru_error e)
{
  return error_strings[e];
}
