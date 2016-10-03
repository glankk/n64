#ifndef GRU_GRU_H
#define GRU_GRU_H

typedef int gru_bool_t;
#define GRU_TRUE  1
#define GRU_FALSE 0

enum gru_error
{
  GRU_SUCCESS,
  GRU_ERROR_MEMORY,
  GRU_ERROR_RANGE,
  GRU_ERROR_PARAM,
  GRU_ERROR_FILEIN,
  GRU_ERROR_FILEOUT,
  GRU_ERROR_DATA,
};

enum gru_endian
{
  GRU_ENDIAN_BIG_4,
  GRU_ENDIAN_LITTLE_4,
  GRU_ENDIAN_LITTLE_2,
};

const char *gru_error_string(enum gru_error e);

#endif
